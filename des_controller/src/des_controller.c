#include <errno.h>
#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/netmgr.h>
#include <sys/neutrino.h>

#include "des.h"

void connect_to_display();
void construct_output();
void create_channel();
void detach_connections();
void receive_message();
void reply_to_inputs();
void send_output();
void validate_args(int argc, char *argv[]);

void* start();
void* identity_verified();
void* door_unlock();
void* door_open();
void* door_close();
void* door_lock();
void* stop();

typedef void *(*StateFunc)();

pid_t inputs_chid = -1;
pid_t inputs_rcvid = -1;
pid_t display_pid = -1;
pid_t display_coid = -1;

Person person;
Display display;
Controller_Response response;

int main(int argc, char *argv[]) {

	validate_args(argc, argv);
	display_pid = atoi(argv[1]);

	create_channel();
	connect_to_display();

	printf("The controller is running as PID: %d\n", getpid());

	StateFunc stateFunc = start;
	StateFunc transitionedStateFunc = start;

	while (1) {
		response = INPUT_DENIED;

		if (person.direction == NONE) {
			puts("Waiting for Person...");
		}

		receive_message();

		transitionedStateFunc = (StateFunc) (*stateFunc)();

		if (transitionedStateFunc != stateFunc
				|| (person.input == WEIGHT_SCALE && stateFunc == door_open)) {
			construct_output();
			send_output();
			stateFunc = transitionedStateFunc;
			response = INPUT_ACCEPTED;

			if (transitionedStateFunc == start && person.input == LOCK_DOOR
					&& person.direction == OUTBOUND) {
				puts("Waiting for Person...");
			}
		}
		reply_to_inputs();

		if (stateFunc == stop) {
			break;
		}
	}

	puts("Exiting controller");
	detach_connections();
	return EXIT_SUCCESS;
}

void* start() {
	if (person.input == EXIT) {
		return stop;
	}

	if (person.id != -1 && person.direction == OUTBOUND
			&& person.input == UNLOCK_DOOR) {
		return door_unlock;
	}

	if (person.input == SCAN_ID) {
		return identity_verified;
	}
	return start;
}

void* identity_verified() {
	if (person.input == EXIT) {
		return stop;
	}

	if (person.input == UNLOCK_DOOR) {
		return door_unlock;
	}
	return identity_verified;
}

void* door_unlock() {
	if (person.input == EXIT) {
		return stop;
	}

	if (person.input == OPEN_DOOR) {
		return door_open;
	}
	return door_unlock;
}

void* door_open() {
	if (person.input == EXIT) {
		return stop;
	}

	if (person.input == CLOSE_DOOR) {
		return door_close;
	}

	return door_open;
}

void* door_close() {
	if (person.input == EXIT) {
		return stop;
	}

	if (person.input == LOCK_DOOR) {
		return start;
	}
	return door_close;
}

void* door_lock() {
	if (person.input == EXIT) {
		return stop;
	}
	return start;
}

void* stop() {
	return NULL;
}

void validate_args(int argc, char *argv[]) {
	if (argc != 2) {
		puts(
				"Example usage: \"./des_controller [display_pid]\", where display_pid is the process id of the display");
		exit(EXIT_FAILURE);
	}
}

void connect_to_display() {
	display_coid = ConnectAttach(ND_LOCAL_NODE, display_pid, 1,
	_NTO_SIDE_CHANNEL, 0);

	if (display_coid == -1) {
		fprintf(stdout, "Failed to connect to the display channel\n");
		perror("des_controller[ConnectAttach]");
		exit(EXIT_FAILURE);
	}
}

void create_channel() {
	inputs_chid = ChannelCreate(0);

	if (inputs_chid == -1) {
		perror("des_controller[ChannelCreate]");
		exit(EXIT_FAILURE);
	}
}

void receive_message() {
	inputs_rcvid = MsgReceive(inputs_chid, &person, sizeof(Person), NULL);

	if (inputs_rcvid == -1) {
		perror("des_controller[MsgReceive]");
		exit(EXIT_FAILURE);
	}
}

void reply_to_inputs() {
	if (MsgReply(inputs_rcvid, EOK, &response, sizeof(Controller_Response))
			== -1) {
		perror("des_controller[MsgReply]");
		exit(EXIT_FAILURE);
	}
}

void construct_output() {
	display.person = person;
	Entrance door = person.entrance;

	if (person.direction == OUTBOUND) {
		door = person.entrance == LEFT ? RIGHT : LEFT;
	}

	switch (person.input) {
	case SCAN_ID:
		display.output = ID_SCANNED;
		break;
	case WEIGHT_SCALE:
		display.output = WEIGHED;
		break;
	case LOCK_DOOR:
		display.output = door == LEFT ? GUARD_LEFT_LOCK : GUARD_RIGHT_LOCK;
		break;
	case UNLOCK_DOOR:
		display.output = door == LEFT ? GUARD_LEFT_UNLOCK : GUARD_RIGHT_UNLOCK;
		break;
	case OPEN_DOOR:
		display.output = door == LEFT ? LEFT_OPEN : RIGHT_OPEN;
		break;
	case CLOSE_DOOR:
		display.output = door == LEFT ? LEFT_CLOSE : RIGHT_CLOSE;
		break;
	case EXIT:
		display.output = EXITED;
	}
}

void send_output() {
	if (MsgSend(display_coid, &display, sizeof(Display), NULL, 0) == -1L) {
		fprintf(stdout, "Failed to send message to the display\n");
		perror("des_controller[MsgSend]");
		exit(EXIT_FAILURE);
	}
}

void detach_connections() {
	ChannelDestroy(inputs_chid);
	ConnectDetach(display_coid);
}
