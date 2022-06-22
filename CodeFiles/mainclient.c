#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MAXLINE 8192  /* Max text line length */

int open_clientfd(char *hostname, char *port) {
    int clientfd;
    struct addrinfo hints, *listp, *p;
    char host[MAXLINE],service[MAXLINE];
    int flags;

    /* Get a list of potential server addresses */
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;  /* Open a connection */
    hints.ai_flags = AI_NUMERICSERV;  /* ... using a numeric port arg. */
    hints.ai_flags |= AI_ADDRCONFIG;  /* Recommended for connections where we get IPv4 or IPv6 addresses */
    getaddrinfo(hostname, port, &hints, &listp);
 
    /* Walk the list for one that we can successfully connect to */
    for (p = listp; p; p = p->ai_next) {
        /* Create a socket descriptor */
        if ((clientfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
            continue; /* Socket failed, try the next */

    flags = NI_NUMERICHOST | NI_NUMERICSERV; /* Display address string instead of domain name and port number instead of service name */
    getnameinfo(p->ai_addr, p->ai_addrlen, host, MAXLINE, service, MAXLINE, flags);
    printf("host:%s, service:%s\n", host, service);

        /* Connect to the server */
        if (connect(clientfd, p->ai_addr, p->ai_addrlen) != -1)
        {
                printf("Connected to server %s at port %s\n", host, service);
                break; /* Success */
        }
        else
        {
            printf("Not connected...\n");
        }
        close(clientfd); /* Connect failed, try another */  //line:netp:openclientfd:closefd
    }

    /* Clean up */
    freeaddrinfo(listp);
    if (!p) /* All connects failed */
        return -1;
    else    /* The last connect succeeded */
        return clientfd;
}

void tree(char *pth,int subdir) {
    DIR *ptr=opendir(pth);
    struct dirent *ptr1;
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
                printf("|-%s\n",ptr1->d_name);
            }
            else {
                printf("|");
                for(int i=1;i<=4*subdir;i++) {
                    printf(" ");
                }
                    printf("|-%s\n",ptr1->d_name);

            }
            tree(pth1,subdir+1);
        }
        else {
            if(subdir==0) {
                printf("|-%s\n",ptr1->d_name);
            }
            else {
                printf("|");
                for(int i=1;i<=4*subdir;i++) {
                    printf(" ");
                }
                printf("|-%s\n",ptr1->d_name);
            }
        }
    }
}

int main(int argc, char **argv)
{
    int clientfd;
    char *host, *port, buf[MAXLINE];
    host = argv[1];
    port = argv[2];
    char tempbuff[1];
    clientfd = open_clientfd(host, port);
    printf("Connection established with server.\n");
    // hostname = gethostname(hostbuffer, sizeof(hostbuffer));
    // checkHostName(hostname);
    char hostbuffer[256];
    int w = gethostname(hostbuffer, sizeof(hostbuffer));
    write(clientfd, hostbuffer, strlen(hostbuffer));
    write(clientfd,"\0",1);
    printf("\nFiles and Folders present in current directory are as below :\n");
    int n;
    while((n = read(clientfd,tempbuff,1)))
    {
        if(tempbuff[0]=='\0')
        {
            break;
        }
        printf("%c",tempbuff[0]);
    } // This loop is for tree...
    printf("Enter 1 to upload the file to the server else Enter 0 to download/view the file from server.\n");
    char indicator = 'n';
    char dummy;
    while(indicator!='1' && indicator!='0')
    {
        printf("Please enter valid input :");
        scanf("%c",&indicator);
        scanf("%c",&dummy);
    }
    write(clientfd,&indicator,1);
    if(indicator=='1')
    {
        printf("Files available to be uploaded:\n");
        tree("./",0);
        printf("\n");
    }
    printf("\nInput the file name :");
    while(fgets(buf,50,stdin)!=NULL)
    {
        int sig = -1;
        if(indicator=='1')
        {
            int len=strlen(buf);
            buf[len-1]='\0';
            write(clientfd, buf, strlen(buf));
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
                write(clientfd,&sizebuff[i],1);
            }
            c = '\0';
            write(clientfd,&c,1);
            while(read(fd,&c,1))
            {
                write(clientfd,&c,1);
            }
            close(fd);
            printf("File uploaded successfully...\n");
        }
        else
        {
            printf("\nDo you want to download the file?\n");
            printf("If your answer is \"YES\", then type 1, otherwise type 0.\n");
            while(sig!=0 && sig!=1)
            {
                printf("Please enter valid input :");
                scanf("%d",&sig);
                scanf("%c",&dummy);
            }
            ////////////////////////
            char npath[100];
            int nfd;
            if(sig)
            {
                printf("\nEnter the path for downloaded file :");
                scanf("%s",npath);
                nfd = open(npath,O_CREAT | O_APPEND | O_WRONLY,0777);
                scanf("%c",&dummy);
            }
            int len=strlen(buf);
            buf[len-1]='\0';
            write(clientfd, buf, strlen(buf));
            if(!sig)
            {
                printf("\nContent of the file :\n");
            }
            char sizebuff[21];
            int ptrsize = 0;
            while((n = read(clientfd,tempbuff,1)))
            {
                sizebuff[ptrsize++] = tempbuff[0];
                if(tempbuff[0]=='\0')
                {
                    break;
                }
            }
            long long int orsize = atoll(sizebuff);
            long long int bitcounter = 0;
            while((n = read(clientfd,tempbuff,1)))
            {
                bitcounter++;
                if(sig)
                {
                    write(nfd,&tempbuff[0],1);
                }
                else
                {
                    printf("%c",tempbuff[0]);
                }
                if(bitcounter==orsize)
                {
                    break;
                }
            }
            if(sig)
            {
                printf("\nFile successfully downloaded...\n");
                close(nfd);
            }
            else
            {
                printf("\nFile successfully recieved...\n");
            }
            ///////////////////////
        }
        printf("Enter 1 to upload the file to the server else Enter 0 to download/view the file from server.\n");
        scanf("%c",&indicator);
        scanf("%c",&dummy);
        write(clientfd,&indicator,1);
        if(indicator=='1')
        {
            printf("Files available to be uploaded:\n");
            tree("./",0);
            printf("\n");
        }
        printf("\nInput the file name :");
    }
    exit(0);
}
