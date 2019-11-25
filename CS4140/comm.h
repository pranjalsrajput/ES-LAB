/*
 * PLATFORM INDEPENDENT CODE!!!!
 *
 * TODO: never modify comm.h and comm.c
 * add you code to other files, as this is in 100% sync with the Qt code
 * so I may overwrite your changes
 * -
 * thx.
 * Beni
 */

#include <inttypes.h>

#ifndef COMM_H
#define COMM_H

#define COMM_LENGTH_STATE_REQUEST 0
#define COMM_LENGTH_STATE_RESPONSE 2
#define COMM_LENGTH_SENSOR_REQUEST 0
#define COMM_LENGTH_SENSOR_RESPONSE 12
#define COMM_LENGTH_CONTROL 7
#define COMM_LENGTH_MODE 1
#define COMM_LENGTH_HEIGHT 1
#define COMM_LENGTH_TRIM 0
#define COMM_LENGTH_CHANNELSWITCH 1
#define COMM_LENGTH_LOGGING_CONTROL 1
#define COMM_LENGTH_LOGGING_SIZE_REQUEST 0
#define COMM_LENGTH_LOGGING_SIZE_RESPONSE 4
#define COMM_LENGTH_LOGGING_DUMP_CHUNK_REQUEST 0
#define COMM_LENGTH_LOGGING_DUMP_CHUNK_RESPONSE LOGGING_LINE_SIZE

#define SIDE_PC 1
#define SIDE_MC 2

#define SIDE_THIS SIDE_MC
#define SIDE_OTHER SIDE_PC

int indexOf(char what);

void processIncoming(char *inBuffer, int inLen);

void removeFromBufferStart(int number);

int getMessageLength(char action, int side);

//TODO: use this function to create messages (protocol may change later, so be careful and never construct messages manually

void constructMessage(char action, char *params, char *output, int *outLen);

#endif // COMM_H
