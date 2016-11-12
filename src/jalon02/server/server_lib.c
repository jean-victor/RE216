/////////////////////////////////////////////////////////////////////////////////////////
//BORDEAUX INP ENSEIRB-MATMECA
//DEPARTEMENT TELECOM
//RE216 PROGRAMMATION RESEAUX
//{daniel.negru,joachim.bruneau_-_queyreix,nicolas.herbaut}@ipb.fr
////////////////////////////////////////////////////////////////////////////////////////

#include <asm-generic/socket.h>
#include <regex.h>
#include <resolv.h>
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <stdbool.h>
#include "../commons/network.h"
#include "server.h"
#include <stdio.h>
#include "../commons/network.h"
#include "contrib.h"

void quit(int sock_client_i, int channel_length,Dlist * channel, Dlist * sock_list){
  char msg[MSG_SIZE]={"[serveur] déconnection\n"};
  int n;
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
}
	
int  nick(char * buffer,int list_length,Dlist *sock_list,int sock_client_i,int notlogon){
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
  return notlogon;
 }	
}				

void who(int list_length,Dlist* sock_list,int sock_client_i){
char msg[MSG_SIZE]="[server] : Online users are \n";
char nickname[NICK_SIZE];
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
}

void whois(char *buffer, int list_length, Dlist *sock_list,int sock_client_i){
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
}

void salon(char *buffer,int channel_length,Dlist* channel,int sock_client_i){
  char *name;
  name = buffer + 8;
  name[strlen(name)-1]='\0';
  int IDchannel=1;

  //====Si liste vide ====
  if(channel_length==0){ // initialisation si liste vide
    dlist_append(channel,IDchannel);//met lID channel dans la structure
    dlist_put_nickname(channel,IDchannel,name); //met le nom du channel
    //add_in_channel(channel,name,sock_client_i);
    char msg[MSG_SIZE]="[serveur] Vous avez créé le salon:";
    strcat(msg,name);
    strcat(msg,"\n");
    do_write(sock_client_i, msg);
  }
							
  int number, previous=0, nameok=0; //
  int k;
	
  //====Verification nom disponible====
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
  
  //====Parcours des salons====
  int j;
  for (j=1 ; j<=channel_length;j++){ // parcours des ID et creation du salon

    number=dlist_find_pos(channel,j);
    
    //==si il y a un "trou" au milieu de la liste des salons (ex: 1-2-5-6)==
    if(number-previous != 1 && nameok ==0){ //si début de liste l'ID n'est pas à 1 il faut utiliser prepend
      if(previous==0){
	IDchannel=1;
	dlist_prepend(channel,IDchannel);
      }
      else{ //sinon

	IDchannel = previous + 1; 
	dlist_insert(channel,IDchannel,IDchannel); // !!!probleme!!! si 2->4 probleme avec insert
      }
      dlist_put_nickname(channel,IDchannel,name);
      char msg[MSG_SIZE]="[serveur] Vous avez créé le salon:";
      strcat(msg,name);
      strcat(msg,"\n");
      do_write(sock_client_i, msg);
      break;
    }
    
    //==si il n'y a pas de trou, c'est a dire que l'on a une suite (ex: 1-2-3-4)==
    if(number==channel_length && nameok==0){
      IDchannel=number+1;
      dlist_append(channel,IDchannel);
      dlist_put_nickname(channel,IDchannel,name);
      char msg[MSG_SIZE]="[serveur] Vous avez créé le salon:";
      strcat(msg,name);
      strcat(msg,"\n");
      do_write(sock_client_i, msg);
    }

    previous++;

  }
}

void leave(char *buffer,Dlist *channel,int sock_client_i,Dlist* sock_list){
  char *name; //nom salon
  name = buffer + 7;
  name[strlen(name)-1]='\0';
  int ID = get_channel_ID(channel,name);						 
  if(ID !=0){
      if(is_in_channel_data(channel,ID,sock_client_i)){
			//on indique aux autre clients qu'il quitte le salon
			int *tab_users;
			tab_users=get_users_in_channel(channel,ID);
			char msg2[MSG_SIZE]="[";
			strcat(msg2,name);
			strcat(msg2,"] ");
			char client_name[NICK_SIZE];
			dlist_get_nickname(sock_list,sock_client_i,client_name);
			strcat(msg2,client_name);
			strcat(msg2, " has left the channel\n");
			int i;
			for(i=1;i<=MAX_CLIENT;i++){
				int sock_user= tab_users[i];
				if(sock_user!=sock_client_i && sock_user!=0){
					do_write(sock_user, msg2);
				}
			}
			dlist_put_nickname(channel,ID,name);//solution provisoire, la fonction get_users_in_cannel modifie le nom
			//on enleve client
			int res=channel_is_empty(channel,name);							
			delete_in_channel(channel,name,sock_client_i);
			if(channel_is_empty(channel,name)==1){							  
			  dlist_remove(channel,ID);
			}
			char msg[MSG_SIZE]="[serveur] Vous avez quitté le channel \n";
			do_write(sock_client_i, msg);
      }
      else{
	char msg[MSG_SIZE]="[serveur] Vous n'etes pas dans ce channel \n";
	do_write(sock_client_i, msg);
      }
    }
  else{
    char msg[MSG_SIZE]="[serveur] Channel innexistant \n";
    do_write(sock_client_i, msg);
  }
}

void join(char *buffer,Dlist *channel,int sock_client_i,Dlist *sock_list){
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
    
    int *tab_users;
    tab_users=get_users_in_channel(channel,ID);
    char msg2[MSG_SIZE]="[";
    strcat(msg2,name);
    strcat(msg2,"] ");
    char client_name[NICK_SIZE];
    dlist_get_nickname(sock_list,sock_client_i,client_name);
    strcat(msg2,client_name);
    strcat(msg2, " has join the channel\n");
    int i;
    for(i=1;i<=MAX_CLIENT;i++){
    int sock_user= tab_users[i];
    if(sock_user!=sock_client_i && sock_user!=0){
									
      do_write(sock_user, msg2);
    }
  }
    
    
    
  }
}

void channel_function(char* buffer,Dlist *channel,int sock_client_i,char nickname[NICK_SIZE]){
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
	if(is_in_channel_data(channel,IDchannel,sock_client_i)==0){
		char msg[MSG_SIZE]="[serveur] Vous n'etes pas dans ce channel\n";
		do_write(sock_client_i, msg);
		return;
	}
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
									
      do_write(sock_user, msg);
    }
  }
							
  dlist_put_nickname(channel,IDchannel,channel_name);//solution provisoire, la fonction get_users_in_cannel modifie le nom
}

void unicast(char *buffer,int list_length,Dlist *sock_list,int sock_client_i,char nickname[NICK_SIZE]){
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
       do_write(sock_client_j, msg);
      found=1;
    }
  }
  if(found==0){
    char msg[MSG_SIZE]="[serveur] Aucun utilisateur trouvé \n";
    do_write(sock_client_i, msg);
  }
}

void broadcast(char *buffer,int list_length,Dlist *sock_list,char nickname[NICK_SIZE],int sock_client_i){
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
      strcat(msg,"[all][");
      strcat(msg,nickname);
      strcat(msg,"]");
      strcat(msg,buffer_temp);
      do_write(sock_client_j, msg);
    }
  }
}

void list_channel(int list_length,Dlist *channel,int sock_client_i){
	char msg[MSG_SIZE]="[server] : Channel: \n";
	char nickname[NICK_SIZE];
	int i;
	for(i=i;i<= list_length;i++){
		int IDchannel=dlist_find_pos(channel,i); 
		memset(nickname, 0, sizeof(nickname));
		dlist_get_nickname(channel,IDchannel,nickname); //récupere le nom du channel
		strcat(msg,"     ");
		strcat(msg,nickname);
		strcat(msg,"\n");
	}
	do_write(sock_client_i, msg);
}
