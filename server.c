#include "server.h"
#include <syslog.h>
#define MAX_CLIENTS 1024

//Declaring variables
pthread_mutex_t mutex;
pthread_t clients[MAX_CLIENTS];
int enteries, entry, MY_PORT, fd, thread_count=0;
char FLAG[2], filename[50], c[100];
struct whiteboardStruct ** whiteboard;

//Declaring functions
int run_daemon();
void create_Message(char * message, int size, int entry);
void create_Error_Message(char * message, size_t size, int entryNumber, int errorType);
void * serve_Client_Request(void * arg);
void * listen_To_New_Clients(void * arg);
void sigterm_handler();
void load_Entries_From_File(FILE * fp);
void initialize_Clean_Entries(int entries);

int main(int argc, char * argv[]) {

	if(argc < 4) {
			printf("Not enough arguments provided!!\n");
			exit(0);
	}
	run_daemon();

	//intialize SIGTERM handler
	struct sigaction sa;
	sa.sa_handler = sigterm_handler;
	sigaction(SIGTERM, &sa, 0);

	MY_PORT = strtol(argv[1], NULL, 10);
	strcpy(FLAG, argv[2]);
	int i;

	if (strcmp(FLAG, "-n") == 0) { 			//initialize whiteboard enteries
		enteries = strtol(argv[3], NULL, 10);
		printf("enteries: %d\n", enteries);
		initialize_Clean_Entries(enteries);
	}

	else {
		strcpy(filename, argv[3]);

		FILE *file;
		if ((file = fopen(filename, "r")) == NULL) {
			printf("Couldn't open file!!\n");
			exit(0);
		}

		whiteboard = malloc(1 * (sizeof(char *)+ (2*sizeof(char)+(2*sizeof(int)))));

		if (file) {
			load_Entries_From_File(file);
			enteries = entry;
			fclose(file);
	 	}
	}

	int	sock, snew, fromlength, number, outnum;

	struct	sockaddr_in	master, from;
	i = 0;
	sock = socket (AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		perror ("Server: cannot open master socket");
		exit (1);
	}

	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &(int) { 1 }, sizeof(int)) < 0)
		error("setsocketopt(SO_REUSEADDR) failed");

	master.sin_family = AF_INET;
	master.sin_addr.s_addr = INADDR_ANY;
	master.sin_port = htons (MY_PORT);

	if (bind (sock, (struct sockaddr*) &master, sizeof (master))) {
		perror ("Server: cannot bind master socket");
		exit (1);
	}

	listen (sock, 5);
	fromlength = sizeof (from);
	pthread_mutex_init(&mutex,NULL);
	int err;
	//snew is the socket descriptor for a unique client
	while(1) {
	int clientfD = accept (sock, (struct sockaddr*) & from, & fromlength);
		if (clientfD < 0) {
			perror ("Server: accept failed");
			exit (1);
		}
		err = pthread_create(&clients[thread_count], NULL, listen_To_New_Clients, (void *) &clientfD);
		if (err != 0)
            printf("\ncan't create thread :[%s]", strerror(err));
		thread_count++;
	}

	outnum = htonl (number);

	free(whiteboard);
}

void * listen_To_New_Clients(void * arg) {
	int * sock_pointer = (int *) arg;
	int sock_id = *sock_pointer;
	pthread_mutex_lock(&mutex);
		memset(c,0,100);
		char buffer[5];
		snprintf(buffer, 5, "%d\n", enteries); 			//convert number of enteries from int to string
		strncpy(c,"CMPUT379 Whiteboard Server v0\n",31);
		strncat(c,buffer,sizeof(buffer));
		printf("Sending: %s\n", buffer);
		send(sock_id,c,100,0);
	pthread_mutex_unlock(&mutex);
	serve_Client_Request(arg);
	return (void *)0;
}

void * serve_Client_Request(void * arg) {
 	int i,j;
	int * sock_pointer = (int *) arg;
	int sock_id = *sock_pointer;

 	while(1) {
		char m[1000000];
		memset(m,0, 1000000);
		memset(c,0, 100);

		int bytes = recv(sock_id,m,1000000,0);

		pthread_mutex_trylock(&mutex);
		if(bytes <= 0) {
			pthread_mutex_unlock(&mutex);
			pthread_exit(arg);
		}

		if (strcmp(m,"exit") == 0) {
				pthread_mutex_unlock(&mutex);
				pthread_exit(arg);
		}

		else {

			char numberStr[bytes]; 						//number of charaters in the entry
			memset(numberStr, 0, bytes);
			for(i = 0;i < bytes-1; i++) {
				if(m[i+1] == 'p' || m[i+1] == 'c')
					break;
				numberStr[i] = m[i+1];
			}

			int entry = atoi(numberStr)-1; 				//converting numberStr to an integer

			char errorMsg[50];
			memset(errorMsg, 0,50);

			if(entry >= enteries || entry < 0 ) {
				create_Error_Message(errorMsg, sizeof(errorMsg),entry+1,0);
				send(sock_id, errorMsg, sizeof(errorMsg), 0);
				continue;
			}

			else {
				if(m[0] == '?') {	   							//if the request is to retrieve the entry
					char message[whiteboard[entry]->size+ 10];
					memset(message,0, strlen(message));
					create_Message(message, sizeof(message), entry);
					send(sock_id, message, strlen(message), 0);
				}

				else if(m[0] == '@') { 	//if the request is to update the entry
					whiteboard[entry]->entryType = m[++i];
					i++; j=0;
					char length[10]; 						//number of charaters in the entry
					memset(length, 0, 10);
					while(m[i]!= '\n') {
						length[j++] = m[i];
						i++;
					}
					int lengthInt = atoi(length); 				//converting numberStr to an integer
					j=0;
					char * msg = whiteboard[entry]->entryMessage;
					msg = (char *) realloc(msg, lengthInt+2);
					for(j = 0; j < lengthInt+2; j++){
						msg[j] = m[i++];
					}
					msg[j] = '\0';

					whiteboard[entry]->size = lengthInt;
					whiteboard[entry]->entryMessage = malloc(strlen(msg)); // continue from here
					whiteboard[entry]->entryMessage = &msg[0];

					for( i = 0; i < enteries; i++) {
					 	char message[whiteboard[i]->size+ 50];
						memset(message,0, strlen(message));
						create_Message(message, sizeof(message), i);
						printf("%s", message);
					}

					create_Error_Message(errorMsg, sizeof(errorMsg),entry+1,1);
					send(sock_id, errorMsg, sizeof(errorMsg), 0);
				}

				else {
					create_Error_Message(errorMsg, sizeof(errorMsg),entry+1,0);
					send(sock_id, errorMsg, sizeof(errorMsg), 1);
				}
			}
		}
		pthread_mutex_unlock(&mutex);
 }
 	return (void *) 0;
}

void sigterm_handler(){
	int i;
	FILE *fw = NULL;
	fw = fopen("whiteboard.all","w" );
	for(i = 0; i < enteries; i++) {
		char message[whiteboard[i]->size+ 50];
		memset(message,0, strlen(message));
		create_Message(message, sizeof(message), i);
		fprintf(fw,"%s",message);
	}
	fclose(fw);
	free(whiteboard);
  _exit(0);
}

void create_Error_Message(char * message, size_t size, int entryNumber, int errorType) {
	sprintf(message, "%c", '!');
	char buffer[5]; char error[30]; memset(error,0, 30);
	sprintf(buffer, "%d", entryNumber);
	strcat(message, buffer);
	if(errorType == 0)
		strcat(error, "e14\nNo such entry!\n");
	else if (errorType == 1)
		strcat(error, "e0\n\n");
	else
		strcat(error, "\nInvalid Query!!\n");
	strcat(message, error);
}

void create_Message(char * message, int size, int entry) {
	sprintf(message, "%c%d%c%d%s", whiteboard[entry]->sign, whiteboard[entry]->entryNumber,
				whiteboard[entry]->entryType, whiteboard[entry]->size,
				whiteboard[entry]->entryMessage);
}


void initialize_Clean_Entries(int enteries) {
	int i;
	whiteboard = malloc(enteries * (sizeof(char *)+ (2*sizeof(char)+(2*sizeof(int)))));
	for(i = 0; i < enteries; i++) {
		whiteboard[i] = malloc(sizeof(struct whiteboardStruct));
		whiteboard[i]->sign = '!';
		whiteboard[i]->entryNumber = i+1;
		whiteboard[i]->entryType = 'p';
		whiteboard[i]->size = 0;
		whiteboard[i]->entryMessage = calloc(2, sizeof(char));
		strcpy(whiteboard[i]->entryMessage, "\n\n");
	}
}

void load_Entries_From_File(FILE * file) {

	entry = 0;
	int k=0, msgSizeInt, i;
	char line1[30]; char msgSizeStr[10];
	char c, type;

	while (fgets(line1, 30, file) != NULL) {
		char numberStr[30];
		memset(msgSizeStr, 0, 10);
		i=1;
		while(line1[i] != 'c' && line1[i] != 'p') {
			i++;
		}

		entry++;

		whiteboard = realloc(whiteboard, entry * (sizeof(char *)+ (2*sizeof(char)+(2*sizeof(int)))));

		type = line1[i++];
		k=0;
		while(i < (int)strlen(line1)-1) {
			msgSizeStr[k++] = line1[i++];
		}
		msgSizeInt = atoi(msgSizeStr);
		char message[msgSizeInt+2];
		memset(message, 0, msgSizeInt+3);

		for(i = 0;i <= msgSizeInt; i++) {
				message[i] = getc(file);
		}
		message[i] = 0;

		if(message[i-1] != '\n') {
			printf("Bad statefile format\n");
			free(whiteboard);
			exit(0);
		}

		whiteboard[entry-1] = malloc(sizeof(struct whiteboardStruct));
		whiteboard[entry-1]->sign = '!';
		whiteboard[entry-1]->entryNumber = entry;
		whiteboard[entry-1]->entryType = type;
		whiteboard[entry-1]->size = msgSizeInt;
		whiteboard[entry-1]->entryMessage = malloc(strlen(message) + 3);

		char msg[(strlen(message) + 2)];
		memset(msg,0, strlen(message) + 2);
		sprintf(whiteboard[entry-1]->entryMessage,"\n%s",message);
 }
	return;
}


int run_daemon() {
	pid_t pid = 0;
	pid_t sid = 0;
	FILE *fp= NULL;

	pid = fork();

	if (pid < 0) {
			printf("fork failed!\n");
			exit(1);
	}

	if (pid > 0) {
		 printf("pid of child process %d \n", pid);
		 exit(0);
	}

	umask(0);

	fp = fopen ("logfile.log", "w+");
	if(!fp){
		printf("cannot open log file");
	}

	sid = setsid();
	if(sid < 0) {
		fprintf(fp, "cannot create new process group");
			exit(1);
	}

// close standard fds
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	fclose(fp);
	return pid;
}
