make:	compileServer compileClient

compileClient:	client.c base64.c
	gcc client.c base64.c -o wbc379 -lcrypto -lm

compileServer:	server.c
	gcc server.c -o wbs379 -pthread
