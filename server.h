#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <signal.h>
#include <setjmp.h>

struct whiteboardStruct {
	char sign;
	int entryNumber;
	char entryType; //whether the entry is encrypted or plain text
	int size; //number of bytes in the entry sent
	char * entryMessage;
};
