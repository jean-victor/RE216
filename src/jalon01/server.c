#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#define MAX_LENGTH 512

void error(const char *msg)
{
    perror(msg);
    exit(1);
}
void init_serv_addr(const char* port, struct sockaddr_in *serv_addr) {

    int portno;

//clean the serv_add structure
    memset(serv_addr,0,sizeof(serv_addr));

//cast the port from a string to an int
    portno   = atoi(port);

//internet family protocol
    serv_addr->sin_family =AF_INET; 

//we bind to any ip form the host
    serv_addr->sin_addr.s_addr = htonl(INADDR_ANY);

//we bind on the tcp port specified
         serv_addr->sin_port = htons(portno);

}
    
int do_socket(int domain, int type, int protocol){
	int sockfd;
	int yes=1;
	
	sockfd= socket(domain,type,protocol);
	if(sockfd == -1){
		error("socket");
	}
	if(setsockopt(sockfd, SOL_SOCKET,SO_REUSEADDR,&yes, sizeof(int)) == -1){
		error("ERROE setting socket options");
		return sockfd;
	}
}
int do_bind(int sockfd, struct sockaddr* serv_addr, int adrlen){
	int bindno = bind(sockfd, (struct sockaddr *)&serv_addr, adrlen);
	if(bindno == -1) {
		error("bind");
	}
}
void do_listen (int sockfd){
	int listenno= listen(sockfd,20);
	if(listenno == -1) {
		error("listen");
	}
}
int do_accept(int sockfd, struct sockaddr* addr, socklen_t* addrlen){
	int acceptno = accept(sockfd, (struct sockaddr *)&addr,  addrlen);
	if ( acceptno == -1){
		error("accept");
		return(acceptno);
	}
}
int do_read(int clientsocket, char *buffer){
	int n = 0;
	if((n = recv(clientsocket, buffer,strlen(buffer) -1, 0)) <0){
		error("recv");
		n=0;
		buffer[n] = '\0';
	}
	return(n);
}
void do_write(int sockfd, const char *buffer){
	write(sockfd,buffer,MAX_LENGTH);
	if(send(sockfd, buffer, strlen(buffer), 0) <0) {
	perror("send");
	exit(EXIT_FAILURE);	
	}
}



int main(int argc, char** argv)
{

    if (argc != 2)
    {
        fprintf(stderr, "usage: RE216_SERVER port\n");
        return 1;
    }
 


    //init the serv_add structure
   int client_socket, ma_socket;
struct sockaddr_in mon_address, client_address;
int mon_address_longueur, lg;
bzero(&mon_address,sizeof(mon_address));
    //create the socket, check for validity!
	ma_socket=do_socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    //saddr_in=malloc(sizeof(saddr_in));
	init_serv_addr(argv[1],&mon_address);
	
	    //perform the binding
	    //we bind on the tcp port specified
	do_bind(ma_socket, (struct sockaddr *)&mon_address, sizeof(mon_address));
	
	    //specify the socket to be a server socket and listen for at most 20 concurrent client
	do_listen(ma_socket);


mon_address_longueur = sizeof(client_address);
    for (;;)
    {

        //accept connection from client
        do_accept(ma_socket, (struct sockaddr *)&client_address,&mon_address_longueur);

        //read what the client has to say
        char buffer[MAX_LENGTH];
        do_read(client_socket,buffer);

        //we write back to the client
        do_write(client_socket, buffer);

        //clean up client socket
        close(client_socket);
    }

    //clean up server socket
	close(ma_socket);
    return 0;
}
