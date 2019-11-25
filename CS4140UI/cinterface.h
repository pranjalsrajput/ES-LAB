#ifndef CINTERFACE_H
#define CINTERFACE_H

#ifdef __cplusplus

/**
 * interface for C code for C++ part
 */

extern "C" void messageReceived(char* ptr, int length);

extern "C" void joyStickEventReceived(int type, int time, int number, int value);

#endif

#endif // CINTERFACE_H
