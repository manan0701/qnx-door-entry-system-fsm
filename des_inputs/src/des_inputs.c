#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/netmgr.h>
#include <sys/neutrino.h>
#include <unistd.h>

#include "../../des_controller/src/des.h"

typedef enum {
	RECEIVED_INPUT, UNKNOWN_INPUT, BUILDING_OCCUPIED
} Input_Status;

Input_Status get_event_input();
void connect_to_controller();
void detach_connection();
void determine_state_after_execution();
void get_person_id();
void get_person_weight();
void print_prompt();
void send_message();
void validate_args(int argc, char *argv[]);

pid_t controller_coid = -1;
pid_t controller_pid = -1;

Person person;
Controller_Response controller_response;

int main(int argc, char *argv[]) {

	validate_args(argc, argv);
	controller_pid = atoi(argv[1]);

	connect_to_controller();
	person.id = -1;

	while (1) {
		Input_Status inputStatus = get_event_input();

		if (inputStatus == UNKNOWN_INPUT) {
			puts("Invalid input event type entered. Please try again");
			continue;
		}

		if (inputStatus == BUILDING_OCCUPIED) {
			puts("Building is occupied. Cannot permit entrance");
			continue;
		}

		send_message();

		if (person.input == EXIT) {
			break;
		}

		if (controller_response == INPUT_ACCEPTED) {
			determine_state_after_execution();
		}
	}

	puts("Exiting inputs");
	detach_connection();
	return EXIT_SUCCESS;
}

void validate_args(int argc, char *argv[]) {
	if (argc != 2) {
		puts("Example usage: \"./des_inputs [controller_pid]\", "
				"where controller_pid is the process id of the controller");
		exit(EXIT_FAILURE);
	}
}

void print_prompt() {
	puts(
			"Enter the event type (ls = left scan, rs = right scan, ws = weight scale, lo = left open, "
					"ro = right open, lc = left closed, rc = right closed, gru = guard right unlock, "
					"grl = guard right lock, gll = guard left lock, glu = guard left unlock, exit = exit programs)");
}

Input_Status get_event_input() {

	print_prompt();

	char user_input[20];
	scanf("%s", &user_input);
	fseek(stdin, 0, SEEK_END);

	if (!strcasecmp(user_input, "ls") || !strcasecmp(user_input, "rs")) {
		person.input = SCAN_ID;
		get_person_id();

		if (person.direction == INBOUND || person.direction == OUTBOUND) {
			return BUILDING_OCCUPIED;
		} else {
			person.direction = INBOUND;
			person.entrance = !strcasecmp(user_input, "ls") ? LEFT : RIGHT;
		}
	} else if (!strcasecmp(user_input, "ws")) {
		person.input = WEIGHT_SCALE;
		get_person_weight();
	} else if (!strcasecmp(user_input, "lo") || !strcasecmp(user_input, "ro")) {
		person.input = OPEN_DOOR;
	} else if (!strcasecmp(user_input, "lc") || !strcasecmp(user_input, "rc")) {
		person.input = CLOSE_DOOR;
	} else if (!strcasecmp(user_input, "gru")
			|| !strcasecmp(user_input, "glu")) {
		person.input = UNLOCK_DOOR;
	} else if (!strcasecmp(user_input, "gll")
			|| !strcasecmp(user_input, "grl")) {
		person.input = LOCK_DOOR;
	} else if (!strcasecmp(user_input, "exit")) {
		person.input = EXIT;
	} else {
		return UNKNOWN_INPUT;
	}
	return RECEIVED_INPUT;
}

void determine_state_after_execution() {

	if (person.input != LOCK_DOOR) {
		return;
	}

	if (person.direction == INBOUND) {
		person.direction = OUTBOUND;
	} else if (person.direction == OUTBOUND) {
		person.id = -1;
		person.weight = -1;
		person.direction = NONE;
	}
}

void get_person_id() {
	int input = -1;

	while (1) {
		puts("Enter the Person's ID:");

		if (!scanf("%d", &input) || input <= 0) {
			puts("Invalid ID entered. Please enter a value greater than 0");
			fseek(stdin, 0, SEEK_END);
		} else {
			break;
		}
	}
	person.id = input;
}

void get_person_weight() {
	int input = -1;

	while (1) {
		puts("Enter the Person's weight:");

		if (!scanf("%d", &input) || input <= 0) {
			puts("Invalid weight entered. Please enter a value greater than 0");
			fseek(stdin, 0, SEEK_END);
		} else {
			break;
		}
	}
	person.weight = input;
}

void connect_to_controller() {
	controller_coid = ConnectAttach(ND_LOCAL_NODE, controller_pid, 1,
	_NTO_SIDE_CHANNEL, 0);

	if (controller_coid == -1) {
		fprintf(stdout, "Failed to connect to the controller channel\n");
		perror("des_inputs[ConnectAttach]");
		exit(EXIT_FAILURE);
	}
}

void send_message() {
	if (MsgSend(controller_coid, &person, sizeof(Person), &controller_response,
			sizeof(Controller_Response)) == -1L) {
		fprintf(stdout, "Failed to send message to the controller\n");
		perror("des_inputs[MsgSend]");
		exit(EXIT_FAILURE);
	}
}

void detach_connection() {
	ConnectDetach(controller_coid);
}
