/////////////////////////////////////////////////////////////////////////////////////////
//BORDEAUX INP ENSEIRB-MATMECA
/*
* tuto.c
*
*  Created on: Oct 22, 2014
*      Author: nherbaut
*/
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/select.h>
#include "../commons/network.h"
#define MAX_MSG 256
int main(int argc, char** argv)
{
    //                       _                                   _   _               _
    //                      | |                                 | | (_)             | |
    //_    _ ___ _   _  __ _| |   ___ ___  _ __  _ __   ___  ___| |_ _  ___  _ __   | |_ ___    ___  ___ _ ____   _____ _ __
    //| | | / __| | | |/ _` | |  / __/ _ | '_ | '_  / _ / __| __| |/ _ | '_   | __/ _   / __|/ _  '__  / / _  '__|
    //| |_| __  |_| | (_| | | | (_| (_) | | | | | | |  __/ (__| |_| | (_) | | | | | || (_) | __   __/ |    V /  __/ |
    // __,_|___/__,_|__,_|_|  ______/|_| |_|_| |_|___|___|__|_|___/|_| |_|  _____/  |___/___|_|    _/ ___|_|
    struct addrinfo hints;
    struct addrinfo *add_info_server;
    int serverFd;
    int status;
    char buffer[MAX_MSG];
    int i;
    fd_set fd_set_read;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if ((status = getaddrinfo(argv[1], argv[2], &hints, &add_info_server))
    != 0)
    {
        printf("%sn", gai_strerror(status));
        exit(-3);
    }
    serverFd = socket(add_info_server->ai_family, add_info_server->ai_socktype,
    0);
    if (serverFd == -1)
    {
        perror("socket");
        exit(-1);
    }
    status = connect(serverFd, add_info_server->ai_addr,
    sizeof(*(add_info_server->ai_addr)));
    if (status == -1)
    {
        perror("connect");
        exit(-1);
    }

    for (;
    ;
    )
    {
        //clean the set before adding file descriptors
        FD_ZERO(&fd_set_read);
        //add the fd for server connection
        FD_SET(serverFd, &fd_set_read);
        //add the fd for user-input
        FD_SET(fileno(stdin), &fd_set_read);
        //as per man, nfds is the highest-numbered file descriptor in any of the three sets, plus 1.
        int max_fd = serverFd + 1;

        //we know wait for any file descriptor to be available for reading
        int select_ret_val = select(max_fd, &fd_set_read, NULL, NULL,
        NULL);
        //printf("oui\n");
        //now we loop over file descriptors until we have reached the highest one
        //open for reading OR we stop looping if we have already treated all file descriptors we should have.
        for (i = 0;
        i < max_fd && select_ret_val > 0;
        i++)
        {
			//printf("nbread: %d\n",select_ret_val);
            //don't forget to wipe buffer before using it
            memset(buffer, 0, sizeof buffer);

            //we check if the file descriptor is into the FD_ISSET
            if (FD_ISSET(i, &fd_set_read))
            {
                //yes it is! now, we have 2 cases, whether it's a user input or a server input.
                //that's a user input
                if (i == fileno(stdin))
                {//printf(" i:%d\n",i);
					//printf("user input\n");
                    //read from stdin
                    do_read(i, buffer);//client


                    //this is wrong, we should use readline
                    //write to the server
                    if(strcmp(buffer, "\n")!=0){
                    do_write(serverFd, buffer);}
                    //this is wrong, we should use writeline
                }
                else
                {//printf(" i:%d\n",i);
					//printf("server input\n");
                    //that's a server input
                    //read from the server
                    //do_read(i, buffer);
                    int res = read(i,buffer,sizeof(buffer));
                    //printf("res=%d",res);
                    //printf("buff=%s",buffer);
                    //this is wrong, we should use readline
                    //write to stdout

                    if(strcmp(buffer, "[serveur] déconnection\n")==0){
                      //printf("--%s--\n",buffer);
                      do_write(fileno(stdout), buffer);
                      close(serverFd);
                      freeaddrinfo(add_info_server);
                      return 0;}

                    do_write(fileno(stdout), buffer);


                    //this is wrong, we should use writeline
                }
                //decrement the count of fd we should treat
                select_ret_val--;
            }
        }
    }
    close(serverFd);
    freeaddrinfo(add_info_server);
    return 1;
}
