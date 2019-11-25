#include "dronecontroller.h"
#include <QApplication>
#include <QDebug>


// entry point
// by Qt Creator ;)
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    DroneController w;
    w.show();

    return a.exec();
}
