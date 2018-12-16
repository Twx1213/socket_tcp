//
//  Header.h
//  socket_tcp
//
//  Created by 汤文骁 on 12/16/18.
//  Copyright © 2018 twx. All rights reserved.
//

#ifndef Header_h
#define Header_h

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
#include <err.h>
#include <time.h>
#include <openssl/ssl.h>
#include <openssl/err.h>


char* mail[20]={0};
char buf[BUFSIZ+1];
char* from_addr;
char* to_addr[10];
int num_to_addr=0;
int n=0;
long len=0;
char* errort[5]={
    "501 Bad address syntax",
    "501 mail from address must be same as authorization user",
    "502 Error: auth command not implemented",
    "503 Error: need EHLO and AUTH first !",
    
};
char* errr;
char local[100]="/Users/twx/Desktop/NetWork/SMTP/socket_tcp/socket_tcp";

int sendtext(int sock, char* cmpstr, char* buf1, char* text, int ifserver){
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
    return 1;
}
void read_socket(int sock)
{
    len = read(sock,buf,BUFSIZ);
    write(1,buf,len);
}

int ssl_sendtext(SSL* ssl, char* cmpstr, char* buf1, char* text, int ifserver){
    if(ifserver==1){
        if(strncmp(buf1,cmpstr,strlen(cmpstr))==0){
            len=SSL_write(ssl, text, (int)strlen(text));
            printf("%s\n",text);
        }
    }
    else{
        len=SSL_write(ssl, text, (int)strlen(text));
        printf("Client:%s\n",text);
    }
    return 1;
}

int ssl_send_line(SSL* ssl,char* cmd)
{
    unsigned long err;
    printf("C: %s",cmd);
    err = SSL_write (ssl, cmd, (int)strlen(cmd));
    return 1;
}

#endif /* Header_h */
