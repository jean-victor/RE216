
#include <unistd.h>
#include <errno.h>
#include "contrib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define MAX_CLIENT 3
ssize_t readLine(int fd, void *buf, size_t len) {
/* Variables */
	int i;
	char c;
	int ret;
	char * ptr;
	ptr = buf;
	int cnt = 0;

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

struct node
{
    int data;
    char nickname[NICK_SIZE] ;
    //char time[20];
		time_t time;
    char IP[20];
    int port;
    int clients[MAX_CLIENT];
    struct node *p_next;
    struct node *p_prev;
};

struct dlist
{
    size_t length;
    struct node *p_tail;
    struct node *p_head;
};

Dlist *dlist_new(void)
{
    Dlist *p_new = malloc(sizeof *p_new);
    if (p_new != NULL)
    {
        p_new->length = 0;
        p_new->p_head = NULL;
        p_new->p_tail = NULL;
    }
    return p_new;
}


//ajout fin de liste
Dlist *dlist_append(Dlist *p_list, int data)
{
    if (p_list != NULL) /* On vérifie si notre liste a été allouée */
    {
        struct node *p_new = malloc(sizeof *p_new); /* Création d'un nouveau node */
        if (p_new != NULL) /* On vérifie si le malloc n'a pas échoué */
        {
            p_new->data = data; /* On 'enregistre' notre donnée */
            p_new->p_next = NULL; /* On fait pointer p_next vers NULL */
            memset(p_new->clients,0,sizeof(p_new->clients));
            if (p_list->p_tail == NULL) /* Cas où notre liste est vide (pointeur vers fin de liste à  NULL) */
            {
                p_new->p_prev = NULL; /* On fait pointer p_prev vers NULL */
                p_list->p_head = p_new; /* On fait pointer la tête de liste vers le nouvel élément */
                p_list->p_tail = p_new; /* On fait pointer la fin de liste vers le nouvel élément */
            }
            else /* Cas où des éléments sont déjà présents dans notre liste */
            {
                p_list->p_tail->p_next = p_new; /* On relie le dernier élément de la liste vers notre nouvel élément (début du chaînage) */
                p_new->p_prev = p_list->p_tail; /* On fait pointer p_prev vers le dernier élément de la liste */
                p_list->p_tail = p_new; /* On fait pointer la fin de liste vers notre nouvel élément (fin du chaînage: 3 étapes) */
            }
            p_list->length++; /* Incrémentation de la taille de la liste */
        }
    }
    return p_list; /* on retourne notre nouvelle liste */
}

//ajout debut de liste
Dlist *dlist_prepend(Dlist *p_list, int data)
{
    if (p_list != NULL)
    {
        struct node *p_new = malloc(sizeof *p_new);
        if (p_new != NULL)
        {
            p_new->data = data;
            p_new->p_prev = NULL;
            if (p_list->p_tail == NULL)
            {
                p_new->p_next = NULL;
                p_list->p_head = p_new;
                p_list->p_tail = p_new;
            }
            else
            {
                p_list->p_head->p_prev = p_new;
                p_new->p_next = p_list->p_head;
                p_list->p_head = p_new;
            }
            p_list->length++;
       }
    }
    return p_list;
}

//ajout a certaine position
Dlist *dlist_insert(Dlist *p_list, int data, int position)
{
    if (p_list != NULL)
    {
        struct node *p_temp = p_list->p_head;
        int i = 1;
        while (p_temp != NULL && i <= position)
        {
            if (position == i)
            {
                if (p_temp->p_next == NULL)
                {
                    p_list = dlist_append(p_list, data);
										printf("tric");
                }
                else if (p_temp->p_prev == NULL)
                {
                    p_list = dlist_prepend(p_list, data);
											printf("machin");
                }
                else
                {
                    struct node *p_new = malloc(sizeof *p_new);;
                    if (p_new != NULL)
                    {
												printf("bidule");
                        p_new->data = data;
                        p_temp->p_next->p_prev = p_new;
                        p_temp->p_prev->p_next = p_new;
                        p_new->p_prev = p_temp->p_prev;
                        p_new->p_next = p_temp;
                        p_list->length++;
                    }
                }
            }
            else
            {
                p_temp = p_temp->p_next;
            }
            i++;
        }
    }
    return p_list;
}

//liberer liste
void dlist_delete(Dlist **p_list)
{
    if (*p_list != NULL)
    {
        struct node *p_temp = (*p_list)->p_head;
        while (p_temp != NULL)
        {
            struct node *p_del = p_temp;
            p_temp = p_temp->p_next;
            free(p_del);
        }
        free(*p_list), *p_list = NULL;
    }
}

//afficher liste
void dlist_display(Dlist *p_list)
{
    if (p_list != NULL)
    {
        struct node *p_temp = p_list->p_head;
        while (p_temp != NULL)
        {
            printf("%d.%s.", p_temp->data,p_temp->nickname);
            int k;
            for(k=1;k<=MAX_CLIENT;k++){

				if(p_temp->clients[k]!=0){
					printf("[%d]",p_temp->clients[k]);
				}
			}
			printf("->");
            fflush(stdout);
            p_temp = p_temp->p_next;
        }

    }
    printf("NULL\n");
}

//enlever un element d'une liste
Dlist *dlist_remove(Dlist *p_list, int data)
{
    if (p_list != NULL)
    {
        struct node *p_temp = p_list->p_head;
        int found = 0;
        while (p_temp != NULL && !found)
        {
            if (p_temp->data == data)
            {
			if(p_temp->p_next == NULL && p_temp->p_prev == NULL){

				 p_list->length = 0;
					p_list->p_head = NULL;
					p_list->p_tail = NULL;


				break;
			}
                if (p_temp->p_next == NULL)
                {
                    p_list->p_tail = p_temp->p_prev;
                    p_list->p_tail->p_next = NULL;
                }
                else if (p_temp->p_prev == NULL)
                {
                    p_list->p_head = p_temp->p_next;
                    p_list->p_head->p_prev = NULL;
                }
                else
                {
                    p_temp->p_next->p_prev = p_temp->p_prev;
                    p_temp->p_prev->p_next = p_temp->p_next;
                }
                free(p_temp);
                p_list->length--;
                found = 1;
            }
            else
            {
                p_temp = p_temp->p_next;
            }
        }
    }
    return p_list;
}

//longueur d'une liste
size_t dlist_length(Dlist *p_list)
{
   size_t ret = 0;
    if (p_list != NULL)
    {
        ret = p_list->length;
    }
    return ret;
}

//renvoi l'entier data à la position donnée
int dlist_find_pos(Dlist *p_list, int position)
{

	Dlist *ret=NULL;
	int data;
    if (p_list != NULL)
    {
        struct node *p_temp = p_list->p_head;
        int i = 1;
        int found=0;
        while (p_temp != NULL && !found)
        {
            if (position == i)
            {
                return p_temp->data;
            }
            else
            {
                p_temp = p_temp->p_next;
            }
            i++;
        }
    }
}

//met un nom par rapport à une data donnée
Dlist *dlist_put_nickname(Dlist *p_list, int data,char nickname[NICK_SIZE])
{
    if (p_list != NULL)
    {
        struct node *p_temp = p_list->p_head;
        int found = 0;
        while (p_temp != NULL && !found)
        {
            if (p_temp->data == data)
            {
				strcpy(p_temp->nickname,nickname);
                return p_list;
                found = 1;
            }
            else
            {
                p_temp = p_temp->p_next;
            }
        }
    }
    return p_list;
}

//recupere le nickname dans la structure par rapport à l'entier data et le met dans le char nickname
void dlist_get_nickname(Dlist *p_list, int data, char nickname[NICK_SIZE])
{
	//char nickname[NICK_SIZE];
    if (p_list != NULL)
    {
        struct node *p_temp = p_list->p_head;
        //int i = 1;
        int found=0;
        while (p_temp != NULL && !found)
       {
            if (p_temp->data == data)
            {
				strcpy(nickname,p_temp->nickname);
				found=1;
                //return nickname;
            }
            else
            {
                p_temp = p_temp->p_next;
            }
    }
    }
}


//met une adresse IP, un port et un temps dans la structure
Dlist *dlist_put_ip_port(Dlist *p_list, int data,char *IP,int port)
{
    if (p_list != NULL)
    {
        struct node *p_temp = p_list->p_head;
        int found = 0;
        while (p_temp != NULL && !found)
        {
            if (p_temp->data == data)
            {
				strncpy(p_temp->IP,IP,19); // mise de l'IP
				p_temp->port=port;//mise du PORT
				//temps
				time_t TIME;
				time(&TIME);
				p_temp->time=TIME;
				return p_list;
                found = 1;
            }
            else
           {
                p_temp = p_temp->p_next;
            }
        }
    }
    return p_list;
}

//permet de récuperer l'IP, le temps et le port(par le return)
int dlist_get_ip_port(Dlist *p_list, int data, char IP[20],time_t *TIME)

{
	//char nickname[NICK_SIZE];
    if (p_list != NULL)
    {
        struct node *p_temp = p_list->p_head;
        //int i = 1;
        int found=0;
        while (p_temp != NULL && !found)
        {
            if (p_temp->data == data)
            {
				strcpy(IP,p_temp->IP);
				*TIME=p_temp->time;
				return p_temp->port;
				found=1;
                //return nickname;
            }
            else
            {
                p_temp = p_temp->p_next;
            }
        }
    }
}

//ajoute un entier(une socket) par rapport à un nom donné, dans le tableau clients de la structure
Dlist * add_in_channel(Dlist *p_list, char channelname[20], int sockfd){
	if (p_list != NULL)
    {
        struct node *p_temp = p_list->p_head;
        int i = 1;
        int found=0;
        while (p_temp != NULL && !found)
        {
            if (strcmp(p_temp->nickname,channelname)==0){
				while (p_temp->clients[i] != 0 && i<=MAX_CLIENT){
					i++;
				}
				if(i<=MAX_CLIENT){
					p_temp->clients[i] = sockfd;
					return p_list;
				}
			}
			else
            {
                p_temp = p_temp->p_next;
           }
		}
	}
}

//eleve une socket du tableau clients, par rapport au nom donné en entrée
Dlist *delete_in_channel(Dlist *p_list,char channel_name[20], int sockfd){
	if (p_list != NULL)
		{
				struct node *p_temp = p_list->p_head;
				int i = 1;
				int found=0;
				while (p_temp != NULL && !found)
				{
					if(strcmp(p_temp->nickname,channel_name)==0)
					{
						while(p_temp->clients[i] != sockfd && i <= MAX_CLIENT+1){
							i = i+1;
						}
						if(i== MAX_CLIENT+1){
							printf("client not in channel\n");
							return p_list;
						}
						else{
							p_temp->clients[i] = 0;
							printf("client left teh channel\n");
							return p_list;
						}
					}
					else{
					p_temp = p_temp->p_next;
				}
			}
		}
	}

//verifie si un channel a des clients
int channel_is_empty(Dlist *p_list,char nickname[20]){
	if (p_list != NULL)
		{
				struct node *p_temp = p_list->p_head;
				int i = 1;
				int found=0;
				while (p_temp != NULL && !found)
				{
					if(strcmp(p_temp->nickname,nickname)==0)
					{
						while(i <= MAX_CLIENT){

							if(p_temp->clients[i]!=0){
								return 0;
							}
							i = i+1;
						}

						return 1;
					}
					else{
						p_temp = p_temp->p_next;
					}
				}
			}
}

//renvoi l'identifiant du channel donné en entrée
int get_channel_ID(Dlist *p_list,char nickname[20]){
	if (p_list != NULL)
		{
				struct node *p_temp = p_list->p_head;
				int i = 1;
				int found=0;
				while (p_temp != NULL && !found)
				{
					if(strcmp(p_temp->nickname,nickname)==0){
						return p_temp->data;
					}
					else{
						p_temp = p_temp->p_next;
					}
				}
					return 0;
				}
}

int *get_users_in_channel(Dlist *p_list,int IDchannel){ // donne un tableau contenant les sockets presentes dans un channel donné
	if (p_list != NULL)
		{
				struct node *p_temp = p_list->p_head;
				int i = 1;
				int found=0;
				int * tab_users;
				int k=1;
				while (p_temp != NULL && !found)
				{
					if(p_temp->data==IDchannel){
						
						//memcpy(tab_users,p_temp->clients,MAX_CLIENT);
						
						
						for (i=1;i<=MAX_CLIENT;i++){
							if(p_temp->clients[i] != 0 ){
								
								tab_users[k]=p_temp->clients[i];
								
								k++;
							}
						}
						return tab_users;
						
						
					}
					else{
						p_temp = p_temp->p_next;
					}
				}
			}
	
}

int is_in_channel(Dlist *p_list,int position ,int sockfd){
	if (p_list != NULL)
    {
		int j=1;
        struct node *p_temp = p_list->p_head;
        while (p_temp != NULL)
        {
			if(j==position){
				int i;
				for (i=1;i<=MAX_CLIENT;i++){
					if(p_temp->clients[i]==sockfd){
						return 1;
					}	
				}
			
		}
		j++;
		p_temp = p_temp->p_next;
		}
			
            
     }
	
	return 0;
}
int is_in_channel_data(Dlist *p_list,int data ,int sockfd){
	if (p_list != NULL)
    {
		int j=1;
        struct node *p_temp = p_list->p_head;
        while (p_temp != NULL)
        {
			if(p_temp->data==data){
				int i;
				for (i=1;i<=MAX_CLIENT;i++){
					if(p_temp->clients[i]==sockfd){
						return 1;
					}	
				}
			
		}
		
		p_temp = p_temp->p_next;
		}
			
            
     }
	
	return 0;
}
int is_name_use(Dlist *p_list,int position ,char name [NICK_SIZE]){
	if (p_list != NULL)
    {
		int j=1;
        struct node *p_temp = p_list->p_head;
        while (p_temp != NULL)
        {
			if(strcmp(name,p_temp->nickname)==0){
					return 1;
				}
			
		
		j++;
		p_temp = p_temp->p_next;
		}
			
            
     }
	
	return 0;
}


