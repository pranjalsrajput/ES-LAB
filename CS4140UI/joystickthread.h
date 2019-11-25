#ifndef JOYSTICKTHREAD_H
#define JOYSTICKTHREAD_H

#include<QThread>

class JoyStickThread : public QThread
{
public:
    explicit JoyStickThread();

    void run();
};

#endif // JOYSTICKTHREAD_H
