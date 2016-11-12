/////////////////////////////////////////////////////////////////////////////////////////
//BORDEAUX INP ENSEIRB-MATMECA
//DEPARTEMENT TELECOM
//RE216 PROGRAMMATION RESEAUX
//{daniel.negru,joachim.bruneau_-_queyreix,nicolas.herbaut}@ipb.fr
////////////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <regex.h>
#include <resolv.h>
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <regex.h>
#include <errno.h>
#include "colors.h"
#include "network.h"
#define MSG_SIZE 256

void error(const char* msg) {
	perror(msg);
	exit(-1);
}

/**************************************************
 * In this file, you should implement every code that
 * can be used by both client and server
 */

int do_socket(int domain, int type, int protocol) {
int yes=1;
int sockfd = socket(domain, type, protocol);
 if (sockfd < 0)
        error("ERROR opening socket");
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
		perror("Error");
return sockfd;
}

void do_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {


}

void init_serv_addr(const char* port, struct sockaddr_in *serv_addr) {
 //clean the serv_add structure
    memset(serv_addr, 0, sizeof(struct sockaddr_in));

    //cast the port from a string to a char*
    int portno = atoi(port);

    //internet family protocol
    serv_addr->sin_family = AF_INET;

    //we bind to any ip form the host
    serv_addr->sin_addr.s_addr = INADDR_ANY;

    //we bind on the tcp port specified
    serv_addr->sin_port = htons(portno);
}

void do_bind(const int fd, struct sockaddr_in *serv_addr) {
	int status;
if ((status = bind(fd, (struct sockaddr *) serv_addr,
                       sizeof(struct sockaddr))) < 0) {
        perror("ERROR on binding %s");

    }
}

void do_accept(const int server_fd, int * client_fd,
		struct sockaddr_in * client_addr) {
int clilen = sizeof(client_addr);
int newsockfd;
  newsockfd= accept(server_fd, (struct sockaddr *) &client_addr, &clilen);

  if (newsockfd < 0)
   error("ERROR on accept");
client_fd= &newsockfd;
 
 //return client_fd;


}

int do_read(const int fd, char buffer[256]) {
/* Variables */
	int i;
	char c;
	int ret;
	char * ptr;
	ptr = buffer;
	int cnt = 0;
	int len=256;
	/* Perform the read */
	for (i = 0 ; i < len; i++){

		ret = read(fd, &c, 1);

		if( ret == 1 ){
			ptr[cnt++] = c;

			if( c == '\n'){
				ptr[cnt] = '\0';
				return i+1;
			}
		}
		else if( 0 == ret ) {
			ptr[cnt] = '\0';
			break;
		}
	}
	ptr[len] = '\0';

	/* Empty stdin buffer in the case of too large user_input */
	if( fd == STDIN_FILENO && i == len ){
		char ss[10*MSG_SIZE];
		ret = read(fd, ss, 10*MSG_SIZE);
	}

	return i;
}

int do_write(const int fd, const char buffer[256]) {
	/* Variables */
	int len=strlen(buffer);
	int nleft;
	int nwritten;
	char * ptr;
	ptr = (char*)buffer;
	nleft = len;

	/* perform the send */
	while( nleft > 0 ){
		if( (nwritten = write(fd, ptr, nleft)) <= 0) {
			if(errno == EINTR){
				nwritten = 0;
				perror("");
			}
			return -1;
		}
		nleft -= nwritten;
		ptr += nwritten;
	}
	return len;

}

