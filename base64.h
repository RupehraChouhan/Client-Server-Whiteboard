/* A BASE-64 ENCODER AND DECODER USING OPENSSL */
#include <string.h> //Only needed for strlen().
#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/pem.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <setjmp.h>

char *base64encode (const void *b64_encode_this, int encode_this_many_bytes);
char *base64decode (const void *b64_decode_this, int decode_this_many_bytes);
