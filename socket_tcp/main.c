//
//  main.c
//  socket_tcp
//
//  Created by 汤文骁 on 11/30/18.
//  Copyright © 2018 twx. All rights reserved.
//

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <unistd.h>

char* mail[20]={0};
char buf[BUFSIZ+1];
char* from_addr;
char* to_addr[10];
int n=0;
long len=0;

void sendtext(int sock, char* cmpstr, char* buf1, char* text, int ifserver){
    if(ifserver==1){
        if(strncmp(buf1,cmpstr,strlen(cmpstr))==0){
            len=send(sock,text,strlen(text),0);
            printf("%s\n",text);
        }
    }
    else{
        write(sock,text,strlen(text));
        printf("Client:%s\n",text);
    }
}
void read_socket(int sock)
{
    len = read(sock,buf,BUFSIZ);
    write(1,buf,len);
}

int sendmail(){
    int sock;
    struct sockaddr_in server;
    struct hostent *hp, *gethostbyname(const char *name);
    char *host_id="smtp.qq.com";
    
    /*=====Create Socket=====*/
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock==-1)
    {
        perror("opening stream socket");
        return 1;
    }
    else
        printf("socket created\n");
    /*=====Verify host=====*/
    server.sin_family = AF_INET;
    hp = gethostbyname(host_id);
    if (hp==(struct hostent *) 0)
    {
        fprintf(stderr, "%s: unknown host\n", host_id);
        return 2;
    }
    /*=====Connect to port 25 on remote host=====*/
    memcpy((char *) &server.sin_addr, (char *) hp->h_addr, hp->h_length);
    server.sin_port=htons(25);
    if (connect(sock, (struct sockaddr *) &server, sizeof server)==-1)
    {
        perror("connecting stream socket");
        return 1;
    }
    else
        printf("Connected\n");
    
    /*=====Write some data then read some =====*/
    read_socket(sock); /* SMTP Server WELCOME string */
    
    /*HELO*/
    sendtext(sock, "", "", mail[0], 0);
    read_socket(sock);
    
    /*AUTH LOGIN*/
    sendtext(sock, "", "", mail[1], 0);
    read_socket(sock);
    
    /*base64 of username*/
    sendtext(sock, "", "", mail[2], 0);
    read_socket(sock);
    
    /*base64 of password*/
    sendtext(sock, "", "", mail[3], 0);
    read_socket(sock);
    
    /*mail from*/
    sendtext(sock, "", "", mail[4], 0);
    read_socket(sock);
    
    /*rcpt to*/
    sendtext(sock, "", "", mail[5], 0);
    read_socket(sock);
    
    /*DATA*/
    sendtext(sock, "", "", mail[6], 0);
    read_socket(sock);
    for(int j=7;j<n-1;j++){
        sendtext(sock, "", "", mail[j], 0);
    }
    read_socket(sock);
    
    /*QUIT*/
    sendtext(sock, "", "", mail[n-1], 0);
    read_socket(sock);
    
    /*=====Close socket and finish=====*/
    close(sock);
    return 0;
}

int getmail()
{
    FILE *fp=NULL;
    char* mailtxt="/Users/twx/Desktop/error.txt";//Path of txt file
    int server_sockfd;
    int client_sockfd = 0;
    struct sockaddr_in my_addr;
    struct sockaddr_in remote_addr;
    int sin_size;
    memset(&my_addr,0,sizeof(my_addr));
    my_addr.sin_family=AF_INET;//ipv4
    my_addr.sin_addr.s_addr=INADDR_ANY;
    my_addr.sin_port=htons(25);//SMTP port
    char* text[10]={
        "220 smtp.TT.com\r\n",
        "250-smtp.TT.com\r\n250-PIPELINING\r\n250-SIZE 73400320\r\n250-STARTTLS\r\n250-AUTH LOGIN PLAIN\r\n250-AUTH=LOGIN\r\n250-MAILCOMPRESS\r\n250 8BITMIME\r\n",
        "334 VXNlcm5hbWU6\r\n",
        "334 UGFzc3dvcmQ6\r\n",
        "235 Authentication successful\r\n",
        "250 OK\r\n",
        "354 End data with <CR><LF>.<CR><LF>\r\n",
        "250 Ok: queued as\r\n",
    };
    
    /*=====TCP on IPv4=====*/
    if((server_sockfd=socket(PF_INET,SOCK_STREAM,0))<0)
    {
        perror("socket");
        return 1;
    }
    /*=====reuseaddr=====*/
    int on=1;
    if(setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int))<0){
        perror("reuseaddr");
        return 1;
    }
    /*=====bind socket to server address=====*/
    if (bind(server_sockfd,(struct sockaddr *)&my_addr,sizeof(struct sockaddr))<0)
    {
        perror("bind");
        return 1;
    }
    /*=====listen queue=====*/
    listen(server_sockfd,5);
    printf("wait for client.\n");
    sin_size=sizeof(struct sockaddr_in);
    /*=====wait for client=====*/
    client_sockfd=(accept(server_sockfd,(struct sockaddr *)&remote_addr,(socklen_t *)&sin_size));
    if(client_sockfd<0)
    {
        perror("accept");
        return 1;
    }
    fp=fopen(mailtxt, "w+");
    printf("accept client %s\n",inet_ntoa(remote_addr.sin_addr));
    sendtext(client_sockfd, "", "", text[0],1);//WELCOME
    
    /*=====recv and send messang=====*/
    int i=0;
    int flag=0;
    while(len>=2)
    {
        len=(recv(client_sockfd,buf,BUFSIZ,0));
        buf[len]='\0';
        mail[i] = malloc(sizeof(char)*len);
        strcpy(mail[i],&(*buf));
        i++;
        printf("%s\n",buf);
        fprintf(fp,"%s", buf);
        if(strcmp(buf,"QUIT")==0){
            close(client_sockfd);
        }
        switch(flag){
            case 0:
                sendtext(client_sockfd, "EHLO", buf, text[1],1);
                flag++;
                break;
            case 1:
                sendtext(client_sockfd, "AUTH LOGIN", buf, text[2],1);
                flag++;
                break;
            case 2:
                sendtext(client_sockfd, "", buf, text[3],1);//mail[2] is base64 of username Mjc3ODAxMDYwNkBxcS5jb20=  
                flag++;
                break;
            case 3:
                sendtext(client_sockfd, "", buf, text[4],1);//mail[3] is base64 of password bmdlb2t2a3RmZml1ZGVjYg==
                flag++;
                break;
            case 4:
                sendtext(client_sockfd, "MAIL FROM", buf, text[5],1);
                flag++;
                break;
        }
        sendtext(client_sockfd, "RCPT TO", buf, text[5],1);
        sendtext(client_sockfd, "DATA", buf, text[6],1);
        sendtext(client_sockfd, ".", buf+strlen(buf)-3, text[7],1);
        
    }
    n=i-1;
    if(i>7) sendmail();
    
    printf("client %s closed\n\n",inet_ntoa(remote_addr.sin_addr));
    close(client_sockfd);
    fclose(fp);
    
    close(server_sockfd);
    return 0;
}

int main(){
    while(1){
        getmail();
    }
    return 0;
}
