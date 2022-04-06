#ifndef DOOR_ENTRY_SYSTEM_H_
#define DOOR_ENTRY_SYSTEM_H_

#define NUM_STATES 7

typedef enum {
	START,
	IDENTITY_VERIFIED,
	DOOR_UNLOCK,
	DOOR_OPEN,
	DOOR_CLOSE,
	DOOR_LOCK,
	STOP
} State;

#define NUM_INPUTS 7

typedef enum {
	SCAN_ID, UNLOCK_DOOR, OPEN_DOOR, WEIGHT_SCALE, CLOSE_DOOR, LOCK_DOOR, EXIT
} Input;

const char *inMessage[NUM_INPUTS] = { "ID scan", "Guard unlock door",
		"Open door", "Weight scale", "Close door", "Guard lock door", "Exit" };

#define NUM_OUTPUTS 11

typedef enum {
	ID_SCANNED,
	WEIGHED,
	LEFT_OPEN,
	RIGHT_OPEN,
	LEFT_CLOSE,
	RIGHT_CLOSE,
	GUARD_RIGHT_UNLOCK,
	GUARD_RIGHT_LOCK,
	GUARD_LEFT_LOCK,
	GUARD_LEFT_UNLOCK,
	EXITED
} Output;

const char *outMessage[NUM_OUTPUTS] = { "Person scanned ID", "Person weighed",
		"Person opened left door", "Person opened right door",
		"Left door closed (automatically)", "Right door closed (automatically)",
		"Right door unlocked by Guard", "Right door locked by Guard",
		"Left door locked by Guard", "Left door unlocked by Guard",
		"Exit Display" };

typedef enum {
	NONE, INBOUND, OUTBOUND
} Direction;

typedef enum {
	LEFT, RIGHT
} Entrance;

typedef enum {
	INPUT_DENIED, INPUT_ACCEPTED,
} Controller_Response;

typedef struct {
	int id;
	int weight;
	Input input;
	Direction direction;
	Entrance entrance;
} Person;

typedef struct {
	Person person;
	Output output;
} Display;

#endif /* DOOR_ENTRY_SYSTEM_H_ */
