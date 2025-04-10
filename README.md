# linux-kernel-message-slot

A Linux kernel module implementing a character device for message-based inter-process communication, with multi-channel support.

# Linux Kernel Module – Message Slot Device

This project implements a Linux kernel module that provides a character device used for inter-process communication (IPC) via virtual message channels. It includes two user-space programs that interact with the device to send and receive messages.

Developed as part of the Operating Systems course at Tel Aviv University.

A message slot is a character device that supports multiple channels, where each channel holds a message that can be read repeatedly until overwritten. Processes access specific channels via `ioctl`, then use standard `read` and `write` system calls for communication.

Unlike pipes or sockets, this mechanism preserves messages and supports multiple reads of the same message by different processes, until the message is explicitly replaced.

## Components

- `message_slot.c` – The kernel module implementing the message slot device logic.
- `message_slot.h` – Contains macro definitions and IOCTL command setup.
- `message_sender.c` – A user-space program that writes a message to a selected channel.
- `message_reader.c` – A user-space program that reads the most recent message from a selected channel.
- `Makefile` – Builds the kernel module.

## Features

- Each device (based on its minor number) can support up to 2^20 message channels.
- Each channel holds exactly one message of up to 128 bytes.
- Messages persist across reads until they are explicitly overwritten.
- Kernel memory is dynamically allocated and freed properly on module removal.
- All file operations include proper error checks and return appropriate `errno` codes.
- Uses safe kernel mechanisms like `access_ok`, `copy_to_user`, and `copy_from_user`.

## How to Use

make  
sudo insmod message_slot.ko  
sudo mknod /dev/slot0 c 235 0  
sudo chmod 666 /dev/slot0  
./message_sender /dev/slot0 3 "hello world"  
./message_reader /dev/slot0 3  

## Compilation

gcc -O3 -Wall -std=c11 message_sender.c -o message_sender  
gcc -O3 -Wall -std=c11 message_reader.c -o message_reader  
make clean  

## Error Handling

EINVAL – If no channel is set, invalid arguments, or wrong IOCTL command.  
EMSGSIZE – If the message exceeds 128 bytes or is empty.  
EWOULDBLOCK – If the selected channel has no message to read.  
ENOSPC – If the user buffer is too small for the message.  
EFAULT – If the user buffer address is invalid.  

## Cleanup

sudo rmmod message_slot  

## Notes

The kernel module assumes there will be at most 256 device files.  
Memory usage is proportional to the number of channels used (up to O(C * M)).  
The user programs are only one possible use case. Any process can interact with the driver using the defined API.
