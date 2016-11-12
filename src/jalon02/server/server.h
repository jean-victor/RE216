/////////////////////////////////////////////////////////////////////////////////////////
//BORDEAUX INP ENSEIRB-MATMECA
//DEPARTEMENT TELECOM
//RE216 PROGRAMMATION RESEAUX
//{daniel.negru,joachim.bruneau_-_queyreix,nicolas.herbaut}@ipb.fr
////////////////////////////////////////////////////////////////////////////////////////

#ifndef SERVER_H_QSGPNSGPOQNSPGOSNDFG
#define SERVER_H_QSGPNSGPOQNSPGOSNDFG

#include "contrib.h"

void quit(int sock_client_i, int channel_length,Dlist * channel, Dlist * sock_list);
int  nick(char * buffer,int list_lengt,Dlist *sock_list,int sock_client_i,int notlogon);
void whois(char *buffer, int list_length, Dlist *sock_list,int sock_client_i);
void salon(char *buffer,int channel_length,Dlist* channel,int sock_client_i);
void leave(char *buffer,Dlist *channel,int sock_client_i,Dlist *sock_list);
void join(char *buffer,Dlist *channel,int sock_client_i,Dlist *sock_list);
void channel_function(char *buffer,Dlist *channel,int sock_client_i,char nickname[NICK_SIZE]);
void unicast(char *buffer,int list_length,Dlist *sock_list,int sock_client_i,char nickname[NICK_SIZE]);
void broadcast(char *buffer,int list_length,Dlist *sock_list,char nickname[NICK_SIZE],int sock_client_i);
void list_channel(int list_length,Dlist *channel,int sock_client_i);


#endif //SERVER_H_QSGPNSGPOQNSPGOSNDFG
