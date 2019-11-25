#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "joystick.h"

#define NAME_LENGTH 128

#include "joystick.h"
#include "cinterface.h"
#include "joyconstants.h"

#define JS_DEV	"/dev/input/js0"

// adapted by Beni Kovács
// acknowledgement: this is directly from the sample code on the website, only fitted to interface Qt correctly
// uses event based handling
// in case of no joystick, the app can still be used with keyboard
void initJoyStickBackend(){

    int fd;
    unsigned char axes = 2;
    unsigned char buttons = 2;
    int version = 0x000800;
    char name[NAME_LENGTH] = "Unknown";

    if ((fd = open(JS_DEV, O_RDONLY)) < 0) {
        perror("jstest");
        return;
    }

    ioctl(fd, JSIOCGVERSION, &version);
    ioctl(fd, JSIOCGAXES, &axes);
    ioctl(fd, JSIOCGBUTTONS, &buttons);
    ioctl(fd, JSIOCGNAME(NAME_LENGTH), name);

    struct js_event js;

    while (1) {
        if (read(fd, &js, sizeof(struct js_event)) != sizeof(struct js_event)) {
            perror("\njstest: error reading");
            return;
            //exit (1);
        }

        joyStickEventReceived(js.type, js.time, js.number, js.value);
    }



}
