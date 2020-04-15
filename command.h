#ifndef COMMAND_H
#define COMMAND_H
struct command {
    uint8_t command;
    int8_t data;
};

#define COMMAND_TOGGLE_LED 	1
#define COMMAND_SEND_TEMP 	2

#endif
