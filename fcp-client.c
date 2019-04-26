#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <assert.h>
#include "commons.h"

int main(int argc, char **argv)
{
    int sockfd,retval;
    int rec_bytes;
    struct sockaddr_in serveraddr;
    filechunk_t fc;
    FILE *f = NULL;
    char *ip;
    if ( argc < 2 )
    {
        /* if no args given connect to localhost */
        ip = LOCALHOST;
    }
    else
        ip = argv[1];

    sockfd = socket (AF_INET,SOCK_STREAM,0);
    assert (sockfd != -1 );

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons (4050);
    serveraddr.sin_addr.s_addr = inet_addr(ip);

    retval = connect (sockfd,(struct sockaddr*) &serveraddr,sizeof
            (serveraddr));
    assert (retval != -1);
    printf (GREEN"Connected to IP:%s"RESET"\n",
            inet_ntoa(serveraddr.sin_addr));
    while (1)
    {
        rec_bytes = recv (sockfd, &fc, sizeof(fc), 0);
        assert (rec_bytes != -1);
        if ( fc.op == EXIT_OP )
        {
            printf (GREEN"Server issued exit"RESET"\n");
            break;
        }
        else if ( fc.op == MSG_OP )
        {
            printf (GREEN"Message from server"RESET"\n");
            printf (BLU"%s"RESET"\n", fc.msg_str);
        }
        else if ( fc.op == FILE_OP )
        {
            printf (GREEN"Incoming file from server"RESET"\n");
            while ( ! fc.done )
            {
                rec_bytes = recv (sockfd, &fc, sizeof(fc), 0);
                if ( ! f )
                    f = fopen (fc.filename, "w");
                if ( fwrite (fc.buf, sizeof(char), fc.b_size, f) )
                    printf ("written to file\n");
            };
            printf (GREEN"File written to '%s'"RESET"\n",fc.filename);
            fclose (f);
        }
        else if ( fc.op == HELLO_OP )
        {
            printf (BLU"%s\n"RESET, fc.msg_str);
        }
    }
    close (sockfd);
    return 0;
}
