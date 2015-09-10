/*
 * server3.c
 *
 *  Created on: Apr 7, 2015
 *      Author: tianqixiong
 */

#include	<sys/types.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<errno.h>
#include	<string.h>
#include	<sys/socket.h>
#include	<netdb.h>
#include	<pthread.h>
#include	"bank.h"

// Miniature server to exercise getaddrinfo(2).

int claim_port(const char * port) {
	struct addrinfo addrinfo;
	struct addrinfo * result;
	int sd;
	char message[256];
	int on = 1;

	addrinfo.ai_flags = AI_PASSIVE;		// for bind()
	addrinfo.ai_family = AF_INET;		// IPv4 only
	addrinfo.ai_socktype = SOCK_STREAM;	// Want TCP/IP
	addrinfo.ai_protocol = 0;		// Any protocol
	addrinfo.ai_addrlen = 0;
	addrinfo.ai_addr = NULL;
	addrinfo.ai_canonname = NULL;
	addrinfo.ai_next = NULL;
	if (getaddrinfo(0, port, &addrinfo, &result) != 0)		// want port 59267
			{
		fprintf( stderr,
				"\x1b[1;31mgetaddrinfo( %s ) failed errno is %s.  File %s line %d.\x1b[0m\n",
				port, strerror( errno), __FILE__, __LINE__);
		return -1;
	} else if ( errno = 0, (sd = socket(result->ai_family, result->ai_socktype,
			result->ai_protocol)) == -1) {
		write(1, message,
				sprintf(message, "socket() failed.  File %s line %d.\n",
						__FILE__, __LINE__));
		freeaddrinfo(result);
		return -1;
	} else if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))
			== -1) {
		write(1, message,
				sprintf(message, "setsockopt() failed.  File %s line %d.\n",
						__FILE__, __LINE__));
		freeaddrinfo(result);
		close(sd);
		return -1;
	} else if (bind(sd, result->ai_addr, result->ai_addrlen) == -1) {
		freeaddrinfo(result);
		close(sd);
		write(1, message,
				sprintf(message, "\x1b[2;33mBinding to port %s ...\x1b[0m\n",
						port));
		return -1;
	} else {
		write(1, message,
				sprintf(message, "\x1b[1;32mSUCCESS : Bind to port %s\x1b[0m\n",
						port));
		freeaddrinfo(result);
		return sd;			// bind() succeeded;
	}
}

void *
client_session_thread(void * arg) {
	int sd;
	char request[2048];
	char response[2048];
	char *messagetoclient;
	char temp;
	int i;
	int limit, size;
	float ignore;
	long senderIPaddr;

	sd = *(int *) arg;
	free(arg);					// keeping to memory management covenant
	pthread_detach(pthread_self());		// Don't join on this thread
	while (read(sd, request, sizeof(request)) > 0) {
		printf("server receives input:  %s\n", request);
		messagetoclient = GetCommand(request);
		write(sd, messagetoclient, strlen(messagetoclient) + 1);
		if(strcmp(messagetoclient,"quit")==0){
			printf("client is disconnected from server\n");
			pthread_exit(pthread_self());
			break;
		}
	}
	close(sd);
	return 0;
}

int main( int argc, char ** argv ) {
	int sd;
	char message[256];
	pthread_t tid;
	pthread_attr_t kernel_attr;
	socklen_t ic;
	int fd;
	struct sockaddr_in senderAddr;
	int * fdptr;

	if (pthread_attr_init(&kernel_attr) != 0) {
		printf("pthread_attr_init() failed in file %s line %d\n", __FILE__,
				__LINE__);
		return 0;
	} else if (pthread_attr_setscope(&kernel_attr, PTHREAD_SCOPE_SYSTEM) != 0) {
		printf("pthread_attr_setscope() failed in file %s line %d\n", __FILE__,
				__LINE__);
		return 0;
	} else if ((sd = claim_port("59267")) == -1) {
		write(1, message,
				sprintf(message,
						"\x1b[1;31mCould not bind to port %s errno %s\x1b[0m\n",
						"59267", strerror( errno )));
		return 1;
	} else if (listen(sd, 100) == -1) {
		printf("listen() failed in file %s line %d\n", __FILE__, __LINE__);
		close(sd);
		return 0;
	} else {
		printf( "server is ready to receive client connections ...\n" );
		ic = sizeof(senderAddr);
		while ((fd = accept(sd, (struct sockaddr *) &senderAddr, &ic)) != -1) {
			printf("Accepted client connection ...\n");
			fdptr = (int *) malloc(sizeof(int));
			*fdptr = fd;	// pointers are not the same size as ints any more.
			if (pthread_create(&tid, &kernel_attr, client_session_thread, fdptr)
					!= 0) {
				printf("pthread_create() failed in file %s line %d\n", __FILE__,
						__LINE__);
				return 0;
			} else {
				continue;
			}
		}
		close(sd);
		return 0;
	}
}
