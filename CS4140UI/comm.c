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

#include "comm.h"
#include "cinterface.h"
#include <stdio.h>

char buffer[128];
int bufferLen = 0;

/**
 * gets the index of a character in the buffer
 * by Beni Kovács
 * @param what the character to look for
 * @return index if found, -1 if not found
 */
int indexOf(char what){
    for(int i=0; i<bufferLen; i++){
        if(buffer[i] == what){
            return i;
        }
    }
    return -1;
}

/***
 * removes n characters from the beginning of the buffer, by shifting
 * by Beni Kovács
 * @param number number of chars to be removed
 */
void removeFromBufferStart(int number){
    for(int i=0; i < bufferLen - number - 1 ; i++){
        buffer[i] = buffer[i + number];
    }
    bufferLen -=number;
}

/**
 * processes incoming data
 * by Beni Kovács
 * @param inBuffer the newly received characters
 * @param inLen length of newly received characters
 */
void processIncoming(char *inBuffer, int inLen) {
    for (int i = 0; i < inLen; i++) {
        if (bufferLen < 128) {
            buffer[bufferLen] = inBuffer[i];
            bufferLen++;
        }
    }
    int indStart = indexOf('[');
    int indEnd;
    int side = SIDE_OTHER;
    if (bufferLen > indStart + 1) {
        indEnd = indStart + 1 + getMessageLength(inBuffer[indStart + 1], side) + 1;
    } else {
        indEnd = -1;
    }
    //maybe loop until reading multiple messages
    while (indStart > -1 && indEnd > -1 && indEnd <= bufferLen) {
        if (indStart <= indEnd) { // a maybe-correct message start
            int messageLength = getMessageLength(buffer[indStart + 1], side);
            if (bufferLen > indStart + messageLength + 2) { // all bytes arrived
                //validate checksum
                int checksumOk = 0;
                int32_t checksum = 0;
                for (uint8_t i = 0; i < 2 + messageLength; i++) {
                    checksum += buffer[indStart + i];
                }
                int8_t cs = (int8_t) buffer[indStart + messageLength + 2];
                if (cs < 0) {
                    cs += 128;
                }
                int8_t cchecksum = (int8_t)(checksum % 128);
                if (cchecksum < 0) {
                    cchecksum += 128;
                }
                if (cchecksum == buffer[indStart + messageLength + 2]) {
                    checksumOk = 1;
                } else {
                    checksumOk = 0;
                }
                if (!checksumOk) {
                    // drop if checksum incorrect
                    removeFromBufferStart(indStart + 1);
                } else {
                    // use message then remove from buffer start
                    messageReceived(buffer + indStart + 1, messageLength + 1);
                    removeFromBufferStart(indStart + 2 + messageLength + 1);
                }
            } else { //wait for more bytes to arrive
                break;
            }
        } else { // sure that end of message can't be before start, error
            removeFromBufferStart(indEnd + 1);
        }
        indStart = indexOf('[');
        if (bufferLen > indStart + 1) {
            indEnd = indStart + 1 + getMessageLength(inBuffer[indStart + 1], side) + 1;
        } else {
            indEnd = -1;
        }
    }
}

/**
 * returns the length of a communication message
 * by Beni Kovács
 * @param action the action
 * @param side if it is the PC or the microcontroller (size may differ)
 * @return the size of the specific message
 */
int getMessageLength(char action, int side){
    switch (action) {
    case 's':
        return side == SIDE_PC ? COMM_LENGTH_STATE_REQUEST : COMM_LENGTH_STATE_RESPONSE;
    case 'a':
        return side == SIDE_PC ? COMM_LENGTH_SENSOR_REQUEST : COMM_LENGTH_SENSOR_RESPONSE;
    case 'm':
        return COMM_LENGTH_CONTROL;
    case 'c':
        return COMM_LENGTH_MODE;
    case 'h':
        return COMM_LENGTH_HEIGHT;
    case 'w':
        return COMM_LENGTH_CHANNELSWITCH;
    case 'l':
        return COMM_LENGTH_LOGGING_CONTROL;
    case 'b':
        return side == SIDE_PC ? COMM_LENGTH_LOGGING_SIZE_REQUEST : COMM_LENGTH_LOGGING_SIZE_RESPONSE;
    case 'd':
        return side == SIDE_PC ? COMM_LENGTH_LOGGING_DUMP_CHUNK_REQUEST : COMM_LENGTH_LOGGING_DUMP_CHUNK_RESPONSE;
    case 't':
        return COMM_LENGTH_TRIM;
    case 'r':
        return 4;
    default:
        return 0;
    }
}

/**
 * constructs a message to be sent
 * this function is not sending the message directly, because that's not platform independent at all
 * by Beni Kovács
 * @param action the action to send
 * @param params parameters (data of action)
 * @param output the encoded message (enough bytes allocated)
 * @param outLen pointer to where the length of the output should be put
 */
void constructMessage(char action, char* params, char* output, int* outLen){
    output[0] = '[';
    output[1] = action;
    int paramsLen = getMessageLength(action, SIDE_THIS);
    for(int i=0; i<paramsLen; i++){
        output[2+i] = params[i];
    }
    int32_t checksum = 0;
    for(uint8_t i=0; i<2+paramsLen; i++){
        checksum += output[i];
    }
    output[2+paramsLen] = (char)(checksum % 128);
    *outLen = 2 + paramsLen + 1;
}
