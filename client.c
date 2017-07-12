#include "base64.h"
#include <math.h>

/* ---------------------------------------------------------------------
This is a sample client program for the number server. The client and
the server need not run on the same machine.
--------------------------------------------------------------------- */
int len, sockfd,  outlen, delen, MY_PORT, total_keys=0;  //query length
unsigned char outbuf[10000000], debuf[10000000];
long enteryNumber;
char key[32];
unsigned char iv[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
char HOST[30];
static char ** keys = NULL;

int encrypt(char * array) {

	if(!keys)
		return 0;
	strcpy(key,keys[0]);

	int tmplen, i;
	memset(outbuf, 0,10000);

	EVP_CIPHER_CTX ctx;
	EVP_CIPHER_CTX_init(&ctx);
	EVP_EncryptInit_ex(&ctx, EVP_aes_256_cbc(), NULL, key, iv);

	if(!EVP_EncryptUpdate(&ctx, outbuf, &outlen, array, strlen(array)))
		return 0; /* Error */

	if(!EVP_EncryptFinal_ex(&ctx, outbuf + outlen, &tmplen))
		return 0;   /* Error */

	outlen += tmplen;
	EVP_CIPHER_CTX_cleanup(&ctx);
	return 1;
}

int decrypt(char * array, size_t size, char * Curr_key) {

	int tmplen, i;
	EVP_CIPHER_CTX ctx;

	memset(debuf,0, 1000000);
	EVP_CIPHER_CTX_init(&ctx);
	EVP_DecryptInit_ex(&ctx, EVP_aes_256_cbc(), NULL, Curr_key, iv);

	outlen = (size*3)/4+1;

	if(!EVP_DecryptUpdate(&ctx, debuf, &delen, array, outlen))
		return 0; /* Error */

	if(!EVP_DecryptFinal(&ctx, debuf + outlen, &tmplen))
		return 0;   /* Error */

	delen += tmplen;
	EVP_CIPHER_CTX_cleanup(&ctx);
	return 1;
}

void connectToPort() {

	int	s, number, i,j ;
	struct	sockaddr_in	server;
	struct hostent *host;

	host = gethostbyname(HOST);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	memset(&server,0, sizeof(server));
	bcopy(host->h_addr, & (server.sin_addr), host->h_length);

	server.sin_family = host->h_addrtype;

	server.sin_port = htons (MY_PORT);  // Any local port will do

	if (connect (sockfd, (struct sockaddr*) & server, sizeof (server))) {
		perror ("Client: cannot connect to server");
		exit (1);
	}
	return;
}

void sigint_handler(){
	char query[] = "exit";
	send(sockfd, query, strlen(query), 0);
	close (sockfd);
	exit(0);
}

void setupKeys(char * file) {
	int i;
	FILE *fp = NULL;
	if ((fp = fopen(file, "r")) == NULL) {
		printf("Couldn't open %s!!\n", file);
		close (sockfd);
		exit(0);
	}

	char key[1000];
	keys = malloc(1 * (sizeof (char *)));
	if (fp) {
		while (fgets(key, 1000, fp) != NULL) {
			total_keys++;
			keys = realloc(keys, total_keys * (sizeof(char *)));
			char * decodedKey = base64decode(key, strlen(key));
			keys[total_keys-1] = decodedKey;
		}
	}
	fclose(fp);
	return ;
}

	int main(int argc, char *argv[]) {

		if(argc < 3) {
			printf("Not enough arguments provided!!\n");
			exit(0);
		}

		strcpy(HOST,argv[1]);
		MY_PORT = strtol(argv[2], NULL, 10);

		connectToPort();

		struct sigaction ctrlc;
	  ctrlc.sa_handler = sigint_handler;
	  sigemptyset(&ctrlc.sa_mask);
	  ctrlc.sa_flags=0;
	  sigaction(SIGINT, &ctrlc, 0);

		if (argc == 4)
			setupKeys(argv[3]);

		//receiving message upon connection to the server
		memset(outbuf, 0,1024);
		recv(sockfd, outbuf, 1000000, 0);
		printf("%s\n", outbuf);
		memset(outbuf,0, 1000000);



		while(1) {
			char queryType[5];
			char entryType;
			int entryNumber, messageSize,i;
			char arr[10];
			memset(queryType,0, 5);
			printf("Request (?), update (@) or exit : ");
			scanf(" %s", queryType);

			if (strcmp(queryType,"exit") == 0){
					sigint_handler();
			}

			else if(strcmp(queryType,"?") == 0) {
				printf("Entry number > 0: ");
				int success = scanf(" %d", &entryNumber);
				if (!success) {
					printf("Invalid Input! Try Again type!\n");
					continue;
				}
				char query[10];
				sprintf(query, "%s",queryType);
				sprintf(arr, "%d", entryNumber);
				strcat(query, arr);
				sprintf(arr, "%s", "\n");
				strcat(query, arr);

				send(sockfd, query, strlen(query), 0);
				memset(outbuf,0, 10000);
				int outputSize = recv(sockfd, outbuf, 1000000, 0);

				if(outputSize == 0) {
					printf("Server didn't respond to the query!!\n");
				}

				char output[outputSize];
				char message[outputSize];
				char nullChar[1]; char len[10]; char length[10]; 						//number of charaters in the entry

				memset(output,0,  outputSize);
				memset(length,0, 10);
				memset(len,0, 10);
				memset(message,0, outputSize);

				for( i = 0; i < strlen(outbuf); i++) {
					output[i] = outbuf[i];
					if(outbuf[i] == 'c' || outbuf[i] == 'e' || outbuf[i] == 'p')
						break;
				}

				if(outbuf[i] == 'c') {
					int j=0;
					i++;
					while(outbuf[i]!= '\n') {
						length[j++] = outbuf[i];
						i++;
					}

					int lengthInt = atoi(length);
					i++;
					j=0;
					while(outbuf[i] != '\n') {
						message[j++] = outbuf[i];
						i++;
					}
					char * decodedData = base64decode(message, strlen(message));

					if (!keys) {
						printf("No keys provided to decrypt -> %s\n", message);
						continue;
					}
					else {
						for(i = 0; i < total_keys; i++) {
							decrypt(decodedData,strlen(message), keys[i]);
							//printf("decrypt: %s\n", debuf);
							if (strncmp(debuf, "CMPUT379 Whiteboard Encrypted v0\n", 32)==0) {

								break;
							}
						}
					}
					if (i == total_keys) {
						printf("Proper key not found to decrypt -> %s\n", message);
						continue;
					}

					//printf("decrypt: %s", debuf);
					i=0;
					while(debuf[i] != '0') {
						i++;
					}
					i+=2; j=0;
					memset(message, 0, sizeof(message));
					while(debuf[i] != '\n') {
						message[j++] = debuf[i];
						i++;
					}

					snprintf(nullChar, 1, "%c", '\0');

					strcat(debuf, nullChar);

					snprintf(len, sizeof(len), "%zu", strlen(message));
					strcat(output, len);
					char line[] = "\n";
					strcat(output, line);
					strcat(output,  message);
					strcat(output, line);
					printf("%s", output);
				}
				else {
					printf("%s", outbuf);
				}
			}

			else  if (strcmp(queryType,"@") == 0){
				printf("Entry number > 0: ");
				int success = scanf("\n%d", &entryNumber);
				if (!success) {
					printf("Invalid Input! Try Again type here!\n");
					continue;
				}
				printf("Plaintext or encryted (p or c): ");
				scanf(" %c", &entryType);
				if (entryType != 'c' && entryType != 'p') {
					printf("Invalid Input! Try Again!\n");
					continue;
				}
				printf("How long is the message? ");
				success = scanf("%d", &messageSize);
				if (!success) {
					printf("Invalid Input! Try Again!\n");
					continue;
				}
				char message[2*messageSize+1];


				if(messageSize != 0) {
					printf("Enter the message: ");
					scanf("%s", message);
				}
				else
					sprintf(message,"%c",'\0');

				if(strlen(message) > messageSize) {
					printf("Input is too long! Try Again!\n");
					continue;
				}
				char query[messageSize + 20];

				sprintf(query, "%s",queryType);
				sprintf(arr, "%d", entryNumber);
				strcat(query, arr);
				sprintf(arr, "%c", entryType);
				strcat(query, arr);

				if(entryType == 'p') {
					sprintf(arr, "%d", messageSize);
					strcat(query, arr);
					sprintf(arr, "%s", "\n");
					strcat(query, arr);
					strcat(query, message);
					strcat(query, arr);
				}

				else {
					char cmput[] = "CMPUT379 Whiteboard Encrypted v0\n";
					char messageToEncrypt[sizeof(message)+ sizeof(cmput)+10];

					snprintf(messageToEncrypt,  sizeof(messageToEncrypt),"%s%s\n", cmput, message);

					int err = encrypt(messageToEncrypt);
					if (err == 0) {
						printf("No keys provided to encrypt -> %s", messageToEncrypt);
						continue;
					}

					char * encodedData = base64encode(outbuf, outlen);

					sprintf(arr, "%d", (unsigned int)strlen(encodedData));
					strcat(query, arr);
					sprintf(arr, "%c", '\n');
					strcat(query, arr);
					strcat(query, encodedData);
					strcat(query, arr);
				}
				send(sockfd, query, strlen(query), 0);
				memset(outbuf, 0, 1000000);
				recv(sockfd, outbuf, 1000000, 0);
				printf("%s", outbuf);
			}
			else {
				printf("Invalid Input! Try Again!\n");
				continue;
			}
		}
		close (sockfd);
		return 0;
	}
