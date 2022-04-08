# Door Entry System (DES) for QNX Neutrino RTOS

The repository contains programs that simulate a building entry controller system.

The system comprises of:

1. Two doors with card scanners on each door
2. A scale that weighs a person after they have entered in the building
3. A human guard that uses a switch to open each of the doors

# Details

1. People enter the control space from the left to enter the building and enter the control space from the right to exit the building.

2. Each person requesting to enter or exit the building scans their unique personal id card containing an integer `person_id` number. The scanner sends a message to the controller with the input information (which door opening is being requested, and which `person_id` is being used. Example: `left 12345`).

3. Only one person at a time should be able to be inside the lock.

4. Assume that the door is self-closing (but not self-locking), and that an event will be sent to the controller when the status of the door changes.

# Door Entry System Display - `des_display`

- The program displays the status of the system - which door is open/closed, if there is a user waiting to enter from the left or right, etc.
- It should run in the background and print out status information to the console each time a status update is sent to it using a message from the `des_controller` program.
- The program print out its process id when it first starts up.

# Door Entry System Controller - `des_controller`

- The program runs in the background, operate the state machine for the controller, and directly maintains the persistent data and status for the controller.
- It contains separate functions for the state handler for each state. Each state handler perform the actions required for that state, send a message to the `des_display` program to update the display (as required), and then check the exit conditions for that state.
- When an exit condition is met, the state handler return the function pointer for the next state handler. The `des_controller` program print out its process id when it first starts up.

# Door Entry System Inputs - `des_inputs`

- The program prompts the user for inputs to the controller. This program is simulating all of the input events from the devices; example, card readers, door latches, scale.

Input prompts:

a. The first prompt is:

> _Enter the event type (ls = left scan, rs = right scan, ws = weight scale, lo = left open, ro = right open, lc = left closed, rc = right closed , gru = guard right unlock, grl = guard right lock, gll = guard left lock, glu = guard left unlock, exit = exit programs)_

b. If the event is `lo`, `ro`, `lc`, `rc`, `glu`, `gll`, `gru`, or `grl`, no further prompt is printed.

c. If the event is `ls` or `rs`, prompt for the `person_id`

> _Enter the person_id_

d. If the event is `ws`, prompt for the weight.

> _Enter the weight_

e. Once the input is finished, send a message to the `des_controller` program to provide the input "event", and the prompt is printed again.

# Error handling

- When `des_controller` receives an illegal DES input event from `des_inputs`, then the controller does not transition to an accepting state nor an error state. Rather, the state machine of `des_controller` is to remain in its current state.
- A transition to the next accepting state is only done on receiving the expected valid input event from `des_inputs` based on the DES grammar.

# Development Environment Setup

- The programs were developed for the QNX Neutrino Real-Time Operating System.
- For my development set-up, I have QNX Software Development Platform (SDP) which includes the ISO image for QNX Neutrino and the QNX Momentics Tool Suite. The software provides a complete and comprehensive development environment for QNX Neutrino-based devices and system.
- I ran and tested the QNX Neutrino in a virtual machine on VMware Workstation 16.x Pro.
- Please refer [QNX Development Setup](./Setup Your QNX Development Host.pdf) for detailed instructions written by the Real-Time Programming professor Gerald Hurdle at Algonquin College.

# Example run

> \# `./des_display &` <br>
> The display is running as PID: 12345

> \# `./des_controller 12345 &` <br>
> The controller is running as PID: 67890. <br>
> Waiting for Person...

> \# `./des_inputs 67890` <br>
> Enter the event type (ls = left scan, rs = right scan, ws = weight scale, lo = left open, ro = right open, lc = left closed, rc = right closed , gru = guard right unlock, grl = guard right lock, gll = guard left lock, glu = guard left unlock, exit = exit programs)

# Input Scenarios

### Left scan inputs

ls

12345

glu

lo

ws

123

lc

gll

gru

ro

rc

grl

exit

### Left and Right scan inputs

ls

12345

glu

lo

ws

123

lc

gll

gru

ro

rc

grl

rs

12345

gru

ro

ws

321

rc

grl

glu

lo

lc

gll

exit

### Error scan inputs

> Note: `ls 12345` is a valid input event and all other inputs are invalid.

ls

12345

lo

ro

lo

ro

letmeout

exit

# Credits

- The specification of the project was provided by professor Gerald Hurdle of the Real Time Programming course at Algonquin College. I have heavily copied over the details of document here for clarity.

- I implemented a unique solution to tackle it using a Finite State Machine and hence, I am sharing it.
