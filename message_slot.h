#ifndef CHARDEV_H
#define CHARDEV_H

#include <linux/ioctl.h>
#define MAJOR_NUM 235


#define MSG_SLOT_CHANNEL _IOW(MAJOR_NUM, 0, unsigned long) // from recreation
#define BUFFER_SIZE 128
#define DEVICE_FILE_NAME "message_slot"

#endif