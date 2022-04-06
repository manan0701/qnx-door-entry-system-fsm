#include <errno.h>
#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/neutrino.h>

#include "../../des_controller/src/des.h"

void create_channel();
void destroy_channel();
void display_message();
void receive_message();
void reply();

pid_t chid = -1;
pid_t rcvid = -1;

Display display;

int main(int argc, char *argv[]) {

	create_channel();
	printf("The display is running as PID: %d\n", getpid());

	while (1) {
		receive_message();
		display_message();
		reply();

		if (display.output == EXITED) {
			break;
		}
	}

	destroy_channel();
	return EXIT_SUCCESS;
}

void create_channel() {
	if ((chid = ChannelCreate(0)) == -1) {
		perror("des_display[ChannelCreate]");
		exit(EXIT_FAILURE);
	}
}

void receive_message() {
	if ((rcvid = MsgReceive(chid, &display, sizeof(Display), NULL)) == -1) {
		perror("des_display[MsgReceive]");
		exit(EXIT_FAILURE);
	}
}

void display_message() {
	if (display.output == ID_SCANNED) {
		printf("%s, ID = %d\n", outMessage[ID_SCANNED], display.person.id);
		return;
	}
	if (display.output == WEIGHED) {
		printf("%s, Weight = %d\n", outMessage[WEIGHED], display.person.weight);
		return;
	}

	printf("%s\n", outMessage[display.output]);
}

void reply() {
	if (MsgReply(rcvid, EOK, NULL, 0) == -1) {
		perror("des_display[MsgReply]");
		exit(EXIT_FAILURE);
	}
}

void destroy_channel() {
	ChannelDestroy(chid);
}
