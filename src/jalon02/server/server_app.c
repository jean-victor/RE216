/////////////////////////////////////////////////////////////////////////////////////////
//BORDEAUX INP ENSEIRB-MATMECA
//DEPARTEMENT TELECOM
//RE216 PROGRAMMATION RESEAUX
//{daniel.negru,joachim.bruneau_-_queyreix,nicolas.herbaut}@ipb.fr
////////////////////////////////////////////////////////////////////////////////////////

#include <resolv.h>
#include <string.h>
#include <sys/select.h>
#include <stdlib.h>
#include "../commons/network.h"
#include "server.h"
#include "contrib.h"
#include <errno.h>
#include <sys/socket.h>
       #include <netinet/in.h>
       #include <arpa/inet.h>
#include <time.h>
#define MSG_SIZE 256
#define MAX_CLIENT 3
#define NICK_SIZE 20

int server_socket_fd;




void main(int argc, char** argv) {

	if (argc != 2) {
		fprintf(stderr, "plese specify local port to bind to\n");
		exit(-1);
	}

struct sockaddr_in serv_addr, cli_addr;
socklen_t addrlen;
int server_socket_fd;
socklen_t clilen;
char buffer[MSG_SIZE];

	//init a fresh socket
    server_socket_fd =do_socket(AF_INET, SOCK_STREAM, 0);

	//init server address structure
	init_serv_addr(argv[1],&serv_addr);

	//perform the binding
	do_bind(server_socket_fd,&serv_addr);
		
	listen(server_socket_fd, MAX_CLIENT);

	//init the fdset
	fd_set fd_set_read;

	int * newsockfd;
	Dlist *sock_list=dlist_new();
	Dlist *channel=dlist_new();
	addrlen=sizeof(struct sockaddr_in);
	int max=server_socket_fd;
	//in an infinite loop, listen to the stored sockets


	for (;;) {

		int list_length=dlist_length(sock_list);
		int channel_length=dlist_length(channel);
		// Remet les sockets dans fd_set_read
		FD_ZERO(&fd_set_read);
        FD_SET(server_socket_fd , &fd_set_read);
        int i;
        dlist_display(channel);
        for (i=1 ; i<=list_length;i++){ // on parcours toutes les sockets clients
				int sock_client_i=dlist_find_pos(sock_list,i);
				FD_SET(sock_client_i,&fd_set_read);

			}

		// SELECT, renvoi le nombre de sockets qui veulent nous parler

        int nb_read;
		if((nb_read = select(max + 1, &fd_set_read, NULL, NULL, NULL)) == -1){
         perror("select()");
         exit(errno);
		}
		
		//====SOCKET SERVER ACTIVE====
		if(FD_ISSET(server_socket_fd,&fd_set_read)){ // si un client veut rejoindre le servueur

			size_t size_cli = sizeof cli_addr;
			int sock_client=accept(server_socket_fd, (struct sockaddr *) &cli_addr,&addrlen);
			char *IP;
			IP=inet_ntoa(cli_addr.sin_addr); //mise de l'IP sous le bon format
			printf("port: %d, IP: %s \n" , cli_addr.sin_port,IP);
			if(list_length==MAX_CLIENT){ //si un 21e client veut se connecter
				char msg[256]={"[serveur]Server cannot accept incoming connections anymore. Try again later.\n"};
				do_write(sock_client, msg);
				close(sock_client);
			}
			else{
				printf("client: %d\n",sock_client);
				dlist_append(sock_list,sock_client);//met le client dans la structure
				dlist_put_ip_port(sock_list,sock_client,IP,cli_addr.sin_port); // on met les informations (ip,port,date de co) dans la structure
				do_write(sock_client, "[serveur] please logon with /nick <your pseudo> ");			
				dlist_display(sock_list);			
				nb_read-=1;
			}
		}

		//====SOCKET CLIENT ACTIVE====
		if(nb_read !=0){ //nb_read != 0 donc au moins un client veut nous parlr
			for (i=1 ; i<=dlist_length(sock_list);i++){
				
				int sock_client_i=dlist_find_pos(sock_list,i);
				
				if(FD_ISSET(sock_client_i,&fd_set_read)){ // recherche des clients qui veulent nous parler
						char nickname[NICK_SIZE];
						int notlogon = 0; // variable qui permet de bloquer utilisateur si pas de nickname
						memset(nickname, 0, sizeof(nickname));
						dlist_get_nickname(sock_list,sock_client_i,nickname); //récupere le pseudo

						//reset the buffer
						memset(buffer, 0, MSG_SIZE);
						//read what the client has to say
						int n;
						n = do_read(sock_client_i, buffer);
						printf("do read: %d\n",n);
						if (n < 0) {
							error("ERROR reading from socket");
							break;
						}
						
						else if(strcmp(nickname, "")==0 && strncmp (buffer, "/nick\n",5)!=0){ //verifie si il y a un pseudo
							notlogon=1;
							char msg[MSG_SIZE]={"[serveur] please logon with /nick <your pseudo>\n"};
							do_write(sock_client_i, msg);
						}
						

						//===QUIT============================//
						else if (strncmp (buffer, "/quit\n",5)==0){ //le client veut quitter
							quit(sock_client_i,channel_length,channel,sock_list);
							break;
						}
						
						//===NICK============================//
						else if (strncmp (buffer, "/nick\n",5)==0){ // le client veut entrer un nom
							notlogon = nick(buffer, list_length, sock_list, sock_client_i, notlogon);
						}
						//===WHO============================//
						else if (strncmp (buffer, "/who\n",4)==0 && strncmp (buffer, "/whois\n",6)!=0 && notlogon == 0){ //commande WHO
							who(list_length,sock_list,sock_client_i);							
						}
						//===WHOIS============================//
						else if (strncmp (buffer, "/whois\n",6)==0 && notlogon == 0){ //commande whois
							whois(buffer, list_length,sock_list,sock_client_i);
						}

						//SALON
						//on garde la meme structure que les clients
						// on remplace: numero socket par l'identifiant du salon
						//				nickname client par le nom du salon
						else if (strncmp (buffer, "/create\n",7)==0 && notlogon == 0){ //CREATE
							salon(buffer,channel_length,channel,sock_client_i);
						}
						//===LEAVE============================//
						else if (strncmp (buffer, "/leave\n",6)==0 && notlogon == 0){ //LEAVE
						  leave(buffer,channel,sock_client_i,sock_list);
						}
					  //===JOIN============================//
					  else if (strncmp (buffer, "/join\n",5)==0 && notlogon == 0){
						  join(buffer,channel,sock_client_i,sock_list);
					  }
					  
					  //===HELP============================//
					  else if (strncmp (buffer, "/help\n",8)==0 && notlogon == 0){
						  char msg[MSG_SIZE]="[serveur] List of command:\n /quit \n /nick <username> \n /who \n /whois <username> \n /create <channel_name> \n /join <channel_name> \n /leave <channel_name> \n /channel <channel_name> <message> \n /msg <username> <message> \n /msgall \n /listchannel \n";
						  do_write(sock_client_i,msg);
					  }
					  //===LISTCHANNEL============================//
					  else if (strncmp (buffer, "/listchannel\n",12)==0 && notlogon == 0){
						  list_channel(channel_length,channel,sock_client_i);
					  }
					  
					  
					  //===CHANNEL============================//
					  else if (strncmp (buffer, "/channel\n",8)==0 && notlogon == 0){
						  channel_function(buffer,channel,sock_client_i,nickname);
					  }

						//UNICAST
						else if (strncmp (buffer, "/msg\n",4)==0 && strncmp (buffer, "/msgall\n",7)!=0 && notlogon == 0){
							unicast(buffer,list_length,sock_list,sock_client_i,nickname);

						}
						//BROADCAST
						else if (strncmp (buffer, "/msgall\n",7)==0 && notlogon == 0){
							broadcast(buffer,list_length,sock_list,nickname,sock_client_i);
						}
							else {
								char msg[MSG_SIZE]=" [serveur] type /help to have a list of command \n";
								do_write(sock_client_i,msg);
							}
						
					nb_read-=1;
				}
				if(nb_read==0){
					break;
				}

			}
			}

			//MAJ du max
		int list_length2=dlist_length(sock_list);
		if(list_length2==0){
			max=server_socket_fd;}
			else{
		max=dlist_find_pos(sock_list,list_length2);}

	}

close(server_socket_fd);
}
