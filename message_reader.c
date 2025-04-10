#include "message_slot.h"
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>


int main(int argc, char *argv[])
{
    int fd, Read_Length;
    char *msg[BUFFER_SIZE];

    if (argc != 3)
    {
        perror("Invalid number of arguments\n");
        exit(1);
    }
    fd = open(argv[1], O_RDONLY);
    if (fd < 0)
    {
        perror("can't open file \n");
        exit(1);
    }
    // atoi changes string to int
    if (ioctl(fd, MSG_SLOT_CHANNEL, atoi(argv[2])) == -1)
    {
        perror("channel couldn't be set \n");
        exit(1);
    }
    Read_Length = read(fd,msg,BUFFER_SIZE);
    if(Read_Length == -1){
        perror("couldn't read message \n");
        exit(1);
    }
    close(fd);
    if(write(1, msg, Read_Length)!= Read_Length){ // check if write failed
        perror("Failed to print message \n");
        exit(1);
    }
    return 0;
}