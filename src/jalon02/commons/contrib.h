/*
 * contrib.h
 *
 *  Created on: Sep 26, 2014
 *      Author: nicolas
 */

#ifndef CONTRIB_H_
#define CONTRIB_H_

#include <unistd.h>
#include <time.h>
#define MSG_SIZE 256
#define NICK_SIZE 20
#define MAX_CLIENT 3
/**
 * function to read a line on a fd
 */
ssize_t
readLine(int fd, void *buffer, size_t n);

struct node;
typedef struct dlist Dlist;
Dlist *dlist_new(void);
Dlist *dlist_append(Dlist *p_list, int data);
Dlist *dlist_prepend(Dlist *p_list, int data);
Dlist *dlist_insert(Dlist *p_list, int data, int position);
void dlist_delete(Dlist **p_list);
void dlist_display(Dlist *p_list);
Dlist *dlist_remove(Dlist *p_list, int data);
size_t dlist_length(Dlist *p_list);
int dlist_find_pos(Dlist *p_list, int position);
Dlist *dlist_put_nickname(Dlist *p_list, int data,char *nickname);
void dlist_get_nickname(Dlist *p_list, int position, char nickname[NICK_SIZE]);
Dlist *dlist_put_ip_port(Dlist *p_list, int data,char *IP,int port);
int dlist_get_ip_port(Dlist *p_list, int data, char IP[20],time_t *TIME);
Dlist * add_in_channel(Dlist *p_list, char channelname[20], int sockfd);
Dlist *delete_in_channel(Dlist *p_list,char channel_name[20], int sockfd);
int channel_is_empty(Dlist *p_list,char nickname[20]);
int get_channel_ID(Dlist *p_list,char nickname[20]);
int *get_users_in_channel(Dlist *p_list,int IDchannel);
int is_in_channel(Dlist *p_list,int position ,int sockfd);
int is_name_use(Dlist *p_list,int position ,char name [NICK_SIZE]);
#endif /* CONTRIB_H_ */
