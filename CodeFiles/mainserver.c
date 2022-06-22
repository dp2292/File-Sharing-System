#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <dirent.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>

#define	MAXLINE	 8192  /* Max text line length */
#define LISTENQ  1024  /* Second argument to listen() */

struct thread_info {
    int client_no;
    char client_name[256];
    int conn_fd;
};

int open_listenfd(char *port) 
{
    struct addrinfo hints, *listp, *p;
    int listenfd, optval=1;
	char host[MAXLINE],service[MAXLINE];
    int flags;

    /* Get a list of potential server addresses */
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;             /* Accept connections */
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG; /* ... on any IP address AI_PASSIVE - used on server for TCP passive connection, AI_ADDRCONFIG - to use both IPv4 and IPv6 addresses */
    hints.ai_flags |= AI_NUMERICSERV;            /* ... using port number instead of service name*/
    getaddrinfo(NULL, port, &hints, &listp);

    /* Walk the list for one that we can bind to */
    for (p = listp; p; p = p->ai_next) {
        /* Create a socket descriptor */
        if ((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) 
            continue;  /* Socket failed, try the next */

        /* Eliminates "Address already in use" error from bind */
        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,    //line:netp:csapp:setsockopt
                   (const void *)&optval , sizeof(int));

		flags = NI_NUMERICHOST | NI_NUMERICSERV; /* Display address string instead of domain name and port number instead of service name */
		getnameinfo(p->ai_addr, p->ai_addrlen, host, MAXLINE, service, MAXLINE, flags);
        printf("host:%s, service:%s\n", host, service);

        /* Bind the descriptor to the address */
        if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0)
            break; /* Success */
        close(listenfd); /* Bind failed, try the next */
    }

    /* Clean up */
    freeaddrinfo(listp);
    if (!p) /* No address worked */
        return -1;

    /* Make it a listening socket ready to accept connection requests */
    if (listen(listenfd, LISTENQ) < 0) {
        close(listenfd);
	return -1;
    }
    return listenfd;
}

void fileshare(int connfd,char *client_name,char indicator)
{
    char buf[MAXLINE];
    size_t n;
    n = read(connfd, buf, MAXLINE);
    buf[n]='\0';
    printf("File name received from client %s.\n",client_name);
    printf("Fie name : %s\n",buf);
    char tempbuff[1];
    if(indicator=='1')
    {
        struct stat st;
        stat(client_name,&st);
        if(!(S_ISDIR(st.st_mode)))
        {
            mkdir(client_name,st.st_mode);
        }
        char t[100];
        strcpy(t,client_name);
        strcat(t,"/");
        strcat(t,buf);
        int nfd = open(t,O_CREAT | O_APPEND | O_WRONLY,0777);
        char sizebuff[21];
        int ptrsize = 0;
        while((n = read(connfd,tempbuff,1)))
        {
            sizebuff[ptrsize++] = tempbuff[0];
            if(tempbuff[0]=='\0')
            {
                break;
            }
        }
        long long int orsize = atoll(sizebuff);
        long long int bitcounter = 0;
        while((n = read(connfd,tempbuff,1)))
        {
            bitcounter++;
            write(nfd,&tempbuff[0],1);
            if(bitcounter==orsize)
            {
                break;
            }
        }
        printf("File received successfully...\n");
    }
    else
    {
        int fd = open(buf,O_RDONLY);
        char c;
        FILE *fpcount = fopen(buf,"r");
        fseek(fpcount, 0LL, SEEK_END);
        long long int res = ftell(fpcount);
        // printf("%lld\n",res);
        fclose(fpcount);
        char sizebuff[21];
        // lltoa (res,sizebuff,10);
        sprintf(sizebuff, "%lld", res);
        for(int i=0;i<strlen(sizebuff);i++)
        {
            write(connfd,&sizebuff[i],1);
        }
        c = '\0';
        write(connfd,&c,1);
        while(read(fd,&c,1)) 
        {
            write(connfd,&c,1);
        }
        close(fd);
        printf("File sent successfully...\n");
    }
}

void tree(char *pth,int subdir,int connfd) {
    DIR *ptr=opendir(pth);
    struct dirent *ptr1;
    char c;
    while((ptr1=readdir(ptr))!=NULL) {
        char s1[1]=".",s2[2]="..";
        if(strlen(ptr1->d_name)==1 || strlen(ptr1->d_name)==2) {
            if(strlen(ptr1->d_name)==1) {
                char s1[1];
                strcpy(s1,ptr1->d_name);
                if(s1[0]=='.') continue;
            }
            else {
                char s2[2];
                strcpy(s2,ptr1->d_name);
                if(s2[0]=='.' && s2[1]=='.') {
                    continue;
                }
            }
        }
        char pth1[1000];
        strcpy(pth1,pth);
        strcat(pth1,"/");
        strcat(pth1,ptr1->d_name);
        struct stat st1;
        stat(pth1,&st1);
        if(S_ISDIR(st1.st_mode)) {
            if(subdir==0) {
                // char c;
                // while(read(fd,&c,1)) 
                // {
                //     write(connfd,&c,1);
                // }
                // c = '\0';
                // write(connfd,&c,1);
                c = '|';
                write(connfd,&c,1);
                c = '-';
                write(connfd,&c,1);
                for(int i=0;i<strlen(ptr1->d_name);i++)
                {
                    write(connfd,&ptr1->d_name[i],1);
                }
                c = '\n';
                write(connfd,&c,1);
                // printf("|-%s\n",ptr1->d_name);
            }
            else {
                // printf("|");
                c = '|';
                write(connfd,&c,1);
                for(int i=1;i<=4*subdir;i++) {
                    // printf(" ");
                    c = ' ';
                    write(connfd,&c,1);
                }
                c = '|';
                write(connfd,&c,1);
                c = '-';
                write(connfd,&c,1);
                for(int i=0;i<strlen(ptr1->d_name);i++)
                {
                    write(connfd,&ptr1->d_name[i],1);
                }
                c = '\n';
                write(connfd,&c,1);
                    // printf("|-%s\n",ptr1->d_name);

            }
            tree(pth1,subdir+1,connfd);
        }
        else {
            if(subdir==0) {
                c = '|';
                write(connfd,&c,1);
                c = '-';
                write(connfd,&c,1);
                for(int i=0;i<strlen(ptr1->d_name);i++)
                {
                    write(connfd,&ptr1->d_name[i],1);
                }
                c = '\n';
                write(connfd,&c,1);
                // printf("|-%s\n",ptr1->d_name);
            }
            else {
                // printf("|");
                c = '|';
                write(connfd,&c,1);
                for(int i=1;i<=4*subdir;i++) {
                    // printf(" ");
                    c = ' ';
                    write(connfd,&c,1);
                }
                c = '|';
                write(connfd,&c,1);
                c = '-';
                write(connfd,&c,1);
                for(int i=0;i<strlen(ptr1->d_name);i++)
                {
                    write(connfd,&ptr1->d_name[i],1);
                }
                c = '\n';
                write(connfd,&c,1);
                // printf("|-%s\n",ptr1->d_name);
            }
        }
    }
    if(subdir==0)
    {
        c = '\0';
        write(connfd,&c,1);
    }
}

void *pthread_handler(void *arg) {
    struct thread_info *thisthread = arg;
    printf("Connection established with client %s.\n",thisthread->client_name);
    tree(".",0,thisthread->conn_fd);
    char indicator;
    int byte = read(thisthread->conn_fd, &indicator, 1);
    //printf("Server recieved idicator :%c.\n",indicator);
    while(byte)
    {
        fileshare(thisthread->conn_fd,thisthread->client_name,indicator);
        byte = read(thisthread->conn_fd, &indicator, 1);
    }
    printf("End communication with client %s.\n",thisthread->client_name);
    close(thisthread->conn_fd);
    return 0;
}

int main(int argc, char **argv)
{
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr; /* Enough room for any addr */
    char client_hostname[MAXLINE], client_port[MAXLINE];
    listenfd = open_listenfd(argv[1]);
    pthread_t tid[LISTENQ];
    struct thread_info threads[LISTENQ];
    int ind=0;
    while (1) 
    {
	    printf("Waiting for a new Client to connect\n");
        clientlen = sizeof(struct sockaddr_storage); /* Important! */
        connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);
        getnameinfo((struct sockaddr *) &clientaddr, clientlen,
        client_hostname, MAXLINE, client_port, MAXLINE, 0);
        // printf("Connected to (%s, %s)\n", client_hostname, client_port);
	    // printf("Start Communication with Client\n");
        //printf("Connection established with client.\n");
        threads[ind].client_no=ind+1;
        threads[ind].conn_fd=connfd;
        int ccount = 0;
        char a;
        while(read(connfd,&a,1))
        {
            threads[ind].client_name[ccount++] = a;
            if(a=='\0')
            {
                break;
            }
        }
        pthread_create(&tid[ind],NULL,pthread_handler,(void *)(&threads[ind]));
        ind++;
        //fileshare(connfd);
	    // printf("End Communication with Client\n");
    }
}
