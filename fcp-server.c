#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include "commons.h"
#include <sys/time.h>
#include <errno.h>
#include <stdlib.h>

#define MAX_LIMIT 20
int main(int argc, char **argv)
{
    int sockfd,retval,newsockfd;
    int sentbytes,max_clients, conn_clients = 0;
    struct sockaddr_in serveraddr,clientaddr;
    filechunk_t fc;
    FILE* file;
    char com_str[10];
    int client_sockets[MAX_LIMIT] = {0};
    int activity,i,sd,max_sd;
    fd_set readfds;

    if ( argc < 2 )
        max_clients = 1;
    else
        max_clients = atoi (argv[1]);

    sockfd = socket (AF_INET,SOCK_STREAM,0);
    assert (sockfd != -1);

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons (4050);
    serveraddr.sin_addr.s_addr = htonl (INADDR_ANY);

    // special stuff to make the kernel reuse the old port/socket
    int enable =1;
    retval = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable,
            sizeof(int));
    assert (retval >= 0);


    retval = bind (sockfd,(struct sockaddr*) &serveraddr,
            sizeof (serveraddr));
    assert (retval != -1);

    printf (BLU"Listening for %d incoming client(s)"RESET"\n",
            max_clients);
    retval = listen (sockfd, max_clients);
    assert (retval != -1);

    int actuallen = sizeof(clientaddr);

    FD_ZERO (&readfds);

    FD_SET (sockfd, &readfds);
    max_sd = sockfd;
    for ( i = 0; i < max_clients;i++)
    {
        sd = client_sockets[i];
        if ( sd > 0 )
            FD_SET (sd, &readfds);
        if ( sd > max_sd )
            max_sd = sd;
    }
    activity = select (max_sd + 1, &readfds, NULL, NULL,
            NULL);

    if ( (activity < 0) && (errno != EINTR))
        printf ("Select error\n");

    // wait till all clients are connected
    // keeps looping till all clients are on board
    while ( conn_clients < max_clients )
    {

        // if detect any new connection
        // accept the connection and add it to socket list
        // after connection prints msg on server
        if ( FD_ISSET (sockfd, &readfds))
        {
            newsockfd = accept (sockfd,
                    (struct sockaddr*) &clientaddr, &actuallen);
            assert (newsockfd != -1);
            for (i = 0;i < max_clients;i++)
            {
                if ( client_sockets[i] == 0 )
                {
                    client_sockets[i] = newsockfd;
                    printf (BLU"New connection detected"RESET"\n");
                    break;
                }
            }
            conn_clients++;
        }
    }

    // server command routine starts here
    // accepts commands from server user and performs appropriate actions
    while (1)
    {

        printf ("> ");
        scanf ("%s", com_str);
        if ( strcmp (com_str, "info") == 0 )
        {
            printf (BLU
                    "Server hosted at %s"RESET"\n",
                    inet_ntoa(serveraddr.sin_addr));
            printf (BLU"%d client(s) currently connected to server"RESET
                    "\n" ,max_clients);
            printf (BLU"Client info:"RESET"\n");
            for ( i = 0;i < max_clients;i++)
            {
                getpeername (client_sockets[i], (struct sockaddr*)
                        &clientaddr,(socklen_t*) &actuallen);
                printf (BLU
                        "\nHost ip:%s\nPort:%d\n"RESET,
                        inet_ntoa(clientaddr.sin_addr),
                        ntohs(clientaddr.sin_port));
            }

        }
        else if ( strcmp (com_str,"message") == 0 )
        {
            printf ("Message body:");
            scanf (" %[^\n]s", fc.msg_str);
            fc.op = MSG_OP;
            for ( i = 0;i < max_clients;i++)
            {
                if ( client_sockets[i] != 0 )
                {
                    sentbytes = send (client_sockets[i], &fc,
                            sizeof(fc), 0);
                    assert (sentbytes != -1);
                }
            }
        }
        else if ( strcmp (com_str,"send") == 0 )
        {
            fc.op = FILE_OP;
            fc.done = false;
            sentbytes = send (newsockfd, &fc, sizeof(fc), 0);
            assert (sentbytes != -1);
            printf ("Path to file : ");
            scanf (" %s", fc.filename);
            file = fopen (fc.filename, "r");
            if ( ! file )
                printf (BLU"File reading error"RESET"\n");
            else
            {
                while ((  fc.b_size = fread ( fc.buf, sizeof(char),
                                BUFSIZ, file)))
                {
                    fc.done = feof(file);
                    for ( i = 0;i < max_clients;i++)
                    {
                        if ( client_sockets[i] != 0 )
                        {
                            sentbytes = send (client_sockets[i], &fc,
                                    sizeof(fc), 0);
                            assert (sentbytes != -1);
                        }
                    }
                }
                fclose (file);
            }
        }
        else if ( strcmp (com_str,"exit") == 0 )
        {
            fc.op = EXIT_OP;
            for ( i = 0;i < max_clients;i++)
            {
                if ( client_sockets[i] != 0 )
                {
                    sentbytes = send (client_sockets[i], &fc,
                            sizeof(fc), 0);
                    assert (sentbytes != -1);
                }
            }
            break;
        }
    }

    close(newsockfd);
    close(sockfd);
    return 0;
}
