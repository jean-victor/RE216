#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#define MAX_LENGTH 10

void get_addr_info(const char *address, const char*port, struct addrinfo **res)
{
  int s;
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET ;
  hints.ai_socktype = SOCK_STREAM ;
  hints.ai_protocol = 0 ;
  s = getaddrinfo(address,port, &hints, res);

  if (s != 0) {
                 fprintf(stdout, "getaddrinfo: %s\n", gai_strerror(s));
                 exit(EXIT_FAILURE);
             }

}


int do_socket(int domain, int type, int protocol)
{
  int sockfd;
  int yes = 1;
  sockfd = socket(domain,type,protocol);
  if(sockfd<0)
    {
      printf("error creating a socket\n");
    }
  if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
    {
      error("ERROR setting socket options");
    }
  return sockfd;
}


int do_connect(int socket, const struct sockaddr *address, int address_len)
{
  int sock_co = connect(socket, address, address_len);
  return sock_co;
}

ssize_t readline(int fd, char *str, size_t maxlen)
{
  memset(str, 0,maxlen);
  fgets(str, maxlen, stdin);
  int len_str = strlen(str);
  int i , w = 0;
  while (i!=len_str)
  {
    w = write(fd,str,len_str);
    i += w;
  }
  printf("sended\n");
}


int main(int argc,char** argv)
{
  int s;
  if (argc != 3)
    {
      fprintf(stderr,"usage: RE216_CLIENT hostname port\n");
      return 1;
    }
  struct addrinfo *res;
  res=malloc(sizeof(struct addrinfo));
  memset(res,'\0',sizeof(struct addrinfo));
  const char * address = argv[1];
  const char *port = argv[2];

  //get address info from the server
  get_addr_info(address,port,&res);
  //INPUT : args cli
  //OUPUT : socket api data structure

  //get the socket
  s = do_socket(AF_INET,SOCK_STREAM,0);
  //IN: socket API data structure
  //OUPUT : socket

  //connect to remote socket
   do_connect(s,res->ai_addr,res->ai_addrlen);
  printf("socker server %d\n",s);
  //char buffer[50];
  //size_t ss = write(s,"salut\n",6);
  //read(s,buffer,sizeof(buffer));
  //printf("%d\n",ss);
  //:IN : socket
  //OUT : connected socket
  while(1)
  {
    //get user input
    char message[MAX_LENGTH];
    printf("Message to send :\n");
    fgets(message,sizeof(message),stdin);
    //IN: socket
    //OUtput: char*
    int taille= sizeof(message);
printf("longueur: %d",taille);
    //send message to the server
    write(s,message,sizeof(message));
    memset(message,'\0',sizeof(message));
    //INPUT: socket, char*
    //OUPUT : error message if error

    //handle_server_message
    //INPUT : socket, buffer
    //OUPUT : filled buffer

    //DISPLAY SERVER MESSAGE TO USER
    //INPUT : buffer
    //OUPUT : nil
}

  return 0;


}
