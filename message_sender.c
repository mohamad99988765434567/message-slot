#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include "message_slot.h"

int main(int argc, char *argv[]) 
{
    int fd;
    if(argc!=4){
        perror("wrong number of arguments  \n");
        exit(1);
    }
    fd = open(argv[1], O_WRONLY);
    if(fd < 0){
        perror("failed to open the file \n ");
        exit(1);
    }
    if (ioctl(fd, MSG_SLOT_CHANNEL, atoi(argv[2])) == -1)
    {
        perror("channel couldnt be set \n");
        exit(1);
    }
    int len = strlen(argv[3]);
    if (write(fd, argv[3], len) != len){
        perror("message write failed");
        exit(1);
    }
    close(fd);
    return 0;
}