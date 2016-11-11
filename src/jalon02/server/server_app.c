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
#include "../commons/dll.h"
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
	//bind(server_socket_fd, (struct sockaddr *) &serv_addr,                       sizeof(serv_addr));
	//specify the socket to be a server socket and listen for at most 20 concurrent client

	listen(server_socket_fd, MAX_CLIENT);

	//init the fdset
	fd_set fd_set_read;



	int * newsockfd;
	Dlist *sock_list=dlist_new();
	Dlist *channel=dlist_new();
	//Dlist *channel_user=dlist_new();
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
				//dlist_display(dlist_find_pos(sock_list,i));
				int sock_client_i=dlist_find_pos(sock_list,i);
				FD_SET(sock_client_i,&fd_set_read);

			}

		// SELECT, renvoi le nombre de sockets qui veulent nous parler

        int nb_read;
		if((nb_read = select(max + 1, &fd_set_read, NULL, NULL, NULL)) == -1){
         perror("select()");
         exit(errno);
		}
		

		if(FD_ISSET(server_socket_fd,&fd_set_read)){ // si un client veut rejoindre le servueur

			size_t size_cli = sizeof cli_addr;
			int sock_client=accept(server_socket_fd, (struct sockaddr *) &cli_addr,&addrlen);
			char *IP;
			//char* TI;
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
							char msg[MSG_SIZE]={"[serveur] déconnection\n"};
							n = do_write(sock_client_i, msg);
							int i;
							for (i=1;i<=channel_length;i++){ // boucle pour enlever le client des differents channel
								int client_in_channel=is_in_channel(channel,i ,sock_client_i);
								
									if(client_in_channel){
										int IDchannel=dlist_find_pos(channel, i); //recupere ID
										char channel_name[NICK_SIZE];
										dlist_get_nickname(channel, IDchannel,channel_name); //recupere nom du channel
										delete_in_channel(channel,channel_name,sock_client_i); // delete la socket du tableau
									}
							}

							dlist_remove(sock_list,sock_client_i);//enleve le client de la liste

							close(sock_client_i);
							break;
						}
						
						//===NICK============================//
						else if (strncmp (buffer, "/nick\n",5)==0){ // le client veut entrer un nom
							//recuperation pseudo
							char* nick;
							int nameok=0;
							nick= buffer +6; //enleve le /nick
							nick[strlen(nick)-1]='\0';
							int k;
							for (k=1 ; k<=list_length;k++){ // parcours de la liste des users, verification si le nom est deja utilisé
								char name[NICK_SIZE];
								memset(name,0,sizeof(name));
								int number=dlist_find_pos(sock_list,k);//Identifiant salon
								dlist_get_nickname(sock_list,number,name);
								
								if( strncmp (nick, name,strlen(nick))==0){
									char msg[MSG_SIZE]="[serveur] Pseudo deja utilise \n";
									do_write(sock_client_i, msg);
									nameok=1;
									break;
								}
							}
							if(nameok==0){
							//mise du pseudo dans la structure
							dlist_put_nickname(sock_list,sock_client_i,nick);

							char msg[MSG_SIZE]="[serveur] Vous êtes loggé au nom de: ";
							strcat(msg,nick);
							strcat(msg,"\n");
							do_write(sock_client_i, msg);
							notlogon = 1;
						}

						}
						//===WHO============================//
						else if (strncmp (buffer, "/who\n",4)==0 && strncmp (buffer, "/whois\n",6)!=0 && notlogon == 0){ //commande WHO
							char msg[MSG_SIZE]="[server] : Online users are \n";
							int j;
							for (j=1 ; j<=list_length;j++){ //parcours de la liste clients
								int sock_client_j=dlist_find_pos(sock_list,j); 
								memset(nickname, 0, sizeof(nickname));
								dlist_get_nickname(sock_list,sock_client_j,nickname); //récupere le pseudo

								if(strcmp(nickname, "")!=0){ //si le nickname est pas vide
									strcat(msg,"     ");
									strcat(msg,nickname);
									strcat(msg,"\n");

								}
							}
							do_write(sock_client_i, msg);
							notlogon =1;
						}
						//===WHOIS============================//
						else if (strncmp (buffer, "/whois\n",6)==0 && notlogon == 0){ //commande whois
							char* nick;
							char nickname[NICK_SIZE];
							nick= buffer +7; //enleve /whois
							//printf("longueur nick: %d\n",strlen(nick));
							nick[strlen(nick)-1]='\0';

							char msg[MSG_SIZE]="[server] ";
							memset(nickname, 0, sizeof(nickname));
							int j;
							for (j=1 ; j<=list_length;j++){
								int sock_client_j=dlist_find_pos(sock_list,j);
								dlist_get_nickname(sock_list,sock_client_j,nickname);
								//printf("nick: %s\n",nick);
								//printf("trouvé: %s demandé: %s\n",nickname,nick);
								if(strncmp (nick, nickname,strlen(nick)-1)==0){
									printf("trouvé utilisateur\n");
									int port;
									char IP2[20];
									time_t TIME;
									char *TI;
									port = dlist_get_ip_port(sock_list, sock_client_j, IP2,&TIME);
									TI=ctime(&TIME);
									//mise des differentes information dans le buffer
									strcat(msg,nickname);
									strcat(msg, " connected since ");
									strcat(msg, TI);
									strcat(msg, " with IP address ");
									strcat(msg, IP2);
									strcat(msg, " and port number ");
									char PORT[6];
									sprintf(PORT,"%d",port);
									//char PORT=(char)port;
									printf("port: %s \n",PORT);
									strcat(msg, PORT);
									strcat(msg, "\n");
									do_write(sock_client_i, msg);
								}
							}

							notlogon=1;

						}

						//SALON
						//on garde la meme structure que les clients
						// on remplace: numero socket par l'identifiant du salon
						//				nickname client par le nom du salon
						else if (strncmp (buffer, "/create\n",7)==0 && notlogon == 0){ //CREATE
							char *name;
							name = buffer + 8;
							name[strlen(name)-1]='\0';
							int IDchannel=1;
							if(channel_length==0){ // initialisation si liste vide
								dlist_append(channel,IDchannel);//met lID channel dans la structure
								dlist_put_nickname(channel,IDchannel,name); //met le nom du channel
								//add_in_channel(channel,name,sock_client_i);
								char msg[MSG_SIZE]="[serveur] Vous avez créé le salon:";
								strcat(msg,name);
								strcat(msg,"\n");
								do_write(sock_client_i, msg);
							}
							
							int number, previous=0, nameok=0;
							int k;
							
							
							
							for (k=1 ; k<=channel_length;k++){ // parcours de la liste des channel, verification si le nom est deja utilisé
								char channel_name[NICK_SIZE];
								memset(channel_name,0,sizeof(channel_name));
								number=dlist_find_pos(channel,k);//Identifiant salon
								dlist_get_nickname(channel,number,channel_name);
								if( strncmp (name, channel_name,strlen(name))==0){
									char msg[MSG_SIZE]="[serveur] Nom de salon deja utilise \n";
									do_write(sock_client_i, msg);
									nameok=1;
									break;
								}
							}
							
							int j;
							for (j=1 ; j<=channel_length;j++){ // parcours des ID et creation du salon

								number=dlist_find_pos(channel,j);
							//printf("number:%dprevious;%d\n",number,previous);
							if(number-previous != 1 && nameok ==0){ //si début de liste l'ID n'est pas à 1 il faut utiliser prepend
							  if(previous==0){
								IDchannel=1;
								dlist_prepend(channel,IDchannel);
							  }
							  else{ //sinon

								IDchannel = previous + 1;  //printf("sinon %d len %d\n",IDchannel,channel_length);
								dlist_insert(channel,IDchannel,IDchannel); // !!!probleme!!! si 2->4 probleme avec insert
							  }
							  //printf("deuxieboucle,number:%d\n",number);
							  dlist_put_nickname(channel,IDchannel,name);
							  //add_in_channel(channel,name,sock_client_i);
							  char msg[MSG_SIZE]="[serveur] Vous avez créé le salon:";
								strcat(msg,name);
								strcat(msg,"\n");
								do_write(sock_client_i, msg);
							  break;
							}

											if(number==channel_length && nameok==0){
												IDchannel=number+1;
							  //printf("premiereboucle,number:%d\n",number);
												dlist_append(channel,IDchannel);
												dlist_put_nickname(channel,IDchannel,name);
							  //add_in_channel(channel,name,sock_client_i);
							  char msg[MSG_SIZE]="[serveur] Vous avez créé le salon:";
								strcat(msg,name);
								strcat(msg,"\n");
								do_write(sock_client_i, msg);
											}

											previous++;

										}

									}

						else if (strncmp (buffer, "/leave\n",6)==0 && notlogon == 0){ //LEAVE
						  char *name; //nom salon
						  name = buffer + 7;
						  name[strlen(name)-1]='\0';
						  int ID = get_channel_ID(channel,name);						 
						  if(ID !=0){
							int res=channel_is_empty(channel,name);							
							delete_in_channel(channel,name,sock_client_i);
							if(channel_is_empty(channel,name)==1){							  
							  dlist_remove(channel,ID);
						  }
						}
						else{
						  char msg[MSG_SIZE]="[serveur] Vous n'etes pas dans ce channel \n";
						  do_write(sock_client_i, msg);
						}
					  }
					  //===JOIN============================//
					  else if (strncmp (buffer, "/join\n",5)==0 && notlogon == 0){
						  char *name; //nom salon
						  name = buffer + 6;
						  name[strlen(name)-1]='\0';
						  int ID = get_channel_ID(channel,name);
						  if( ID ==0){
							  char msg[MSG_SIZE]="[serveur] Aucun serveur trouvé \n";
								do_write(sock_client_i, msg);
						  }
						  else{
							  add_in_channel(channel,name,sock_client_i);
							  char msg[MSG_SIZE]="[serveur] Vous avez rejoint le channel: ";
							  strcat(msg,name);
							  strcat(msg,"\n");
								do_write(sock_client_i, msg);
						  }
					  }
					  
					  //===HELP============================//
					  else if (strncmp (buffer, "/help\n",8)==0 && notlogon == 0){
						  char msg[MSG_SIZE]="[serveur] List of command:\n /quit \n /nick <username> \n /who \n /whois <username> \n /create <channel_name> \n /join <channel_name> \n /leave <channel_name> \n /channel <channel_name> <message> \n /msg <username> <message> \n /msgall \n ";
						  do_write(sock_client_i,msg);
					  }
					  
					  
					  //===CHANNEL============================//
					  else if (strncmp (buffer, "/channel\n",8)==0 && notlogon == 0){
						  //on recupere le pseudo
						  
							char *pch;
							char channel_name[NICK_SIZE];
							//char * buffer_temp;
							char buffer_temp[MSG_SIZE];
							strcpy(buffer_temp,buffer);
							pch=strtok(buffer_temp," ");
							pch=strtok(NULL," ");
							strcpy(channel_name,pch);
							int IDchannel = get_channel_ID(channel,channel_name);
							int *tab_users;
							
							tab_users=get_users_in_channel(channel,IDchannel);
							int i;
							char msg[MSG_SIZE];
							char *buffer_temp2;
							buffer_temp2 = buffer + strlen("channel\n ") + strlen(channel_name);
							memset(msg, 0, sizeof(msg));
							strcat(msg,"[");
							//channel_name[strlen(channel_name)-1]='\0';
							strcat(msg,channel_name);
							strcat(msg,"][");
							strcat(msg,nickname);
							strcat(msg,"]");
							strcat(msg,buffer_temp2);
							for(i=1;i<=MAX_CLIENT;i++){
								int sock_user= tab_users[i];
								printf("sock%d\n",sock_user);
								if(sock_user!=sock_client_i && sock_user!=0){
									
										n = do_write(sock_user, msg);
								}
							}
							
							dlist_put_nickname(channel,IDchannel,channel_name);//solution provisoire, la fonction get_users_in_cannel modifie le nom
					  }

						//UNICAST
						else if (strncmp (buffer, "/msg\n",4)==0 && strncmp (buffer, "/msgall\n",7)!=0 && notlogon == 0){
							//on recupere le pseudo
							char *pch;
							char pseudo[NICK_SIZE];
							//char * buffer_temp;
							char buffer_temp[MSG_SIZE];
							strcpy(buffer_temp,buffer);
							pch=strtok(buffer_temp," ");
							pch=strtok(NULL," ");
							strcpy(pseudo,pch);

							//on cherche le pseudo dans la liste client
							int found=0;
							int j;
							for (j=1 ; j<=list_length;j++){
								int sock_client_j=dlist_find_pos(sock_list,j);
								char nickname_other[NICK_SIZE];
								dlist_get_nickname(sock_list,sock_client_j,nickname_other);

								if( strcmp(pseudo,nickname_other)==0){ //on a trouvé le client

									char msg[MSG_SIZE];
									char *buffer_temp;
									//memset(buffer_temp, 0, sizeof(buffer_temp));
										buffer_temp = buffer + 5 + strlen(pseudo);


										memset(msg, 0, sizeof(msg));
										strcat(msg,"[private][");
										strcat(msg,nickname);
										strcat(msg,"]");
										strcat(msg,buffer_temp);
										n = do_write(sock_client_j, msg);
										found=1;
									}
								}
							if(found==0){
								char msg[MSG_SIZE]="[serveur] Aucun utilisateur trouvé \n";
								do_write(sock_client_i, msg);
							}

						}
						//BROADCAST
						else if (strncmp (buffer, "/msgall\n",7)==0 && notlogon == 0){
							char *buffer_temp;
							//memset(buffer_temp, 0, sizeof(buffer_temp));
							buffer_temp = buffer + 8;
							int j;
							for (j=1 ; j<=list_length;j++){
								int sock_client_j=dlist_find_pos(sock_list,j);
								char nickname_other[NICK_SIZE];
								dlist_get_nickname(sock_list,sock_client_j,nickname_other);

									if(sock_client_i != sock_client_j){

										char msg[MSG_SIZE];
										memset(msg, 0, sizeof(msg));
										strcat(msg,"[");
										strcat(msg,nickname);
										strcat(msg,"]");
										strcat(msg,buffer_temp);
										n = do_write(sock_client_j, msg);
									}


							}
							}
							else {
								char msg[MSG_SIZE]=" [serveur] type /help to have a list of command \n";
								do_write(sock_client_i,msg);
							}
						//printf("do write: %d\n",n);
						//a server -> client error occured, deal with it
						if (n < 0) {
							error("ERROR writing to socket");
							break;
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



		//}
		//use select to listen to sockets

		//implement the listening for incoming sockets

		// then implement the listening for already connected socket that write data to the server

	}

//clean up server socket
int j;
//for (j=1 ; j<=dlist_length(sock_list);j++){
	//int sock_client_j=dlist_find_pos(sock_list,j);
	//close(sock_client_j);
//}
close(server_socket_fd);
}
