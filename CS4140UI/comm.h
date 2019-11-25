#ifndef COMM_H
#define COMM_H

#define LOGGING_LINE_SIZE 40

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

#define SIDE_THIS SIDE_PC
#define SIDE_OTHER SIDE_MC

#include <inttypes.h>

int indexOf(char what);
void processIncoming(char* inBuffer, int inLen);
void removeFromBufferStart(int number);
int getMessageLength(char action, int side);
void constructMessage(char action, char* params, char* output, int* outLen);

#endif // COMM_H
