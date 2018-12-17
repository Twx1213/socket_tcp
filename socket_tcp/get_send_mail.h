//
//  get_send_mail.h
//  socket_tcp
//
//  Created by Tiko on 12/16/18.
//  Copyright © 2018 Tiko. All rights reserved.
//

#ifndef get_send_mail_h
#define get_send_mail_h

#include "Header.h"



char* log_path(){
    struct tm *ptr;
    time_t localTime;
    char* str;
    str = malloc(sizeof(char)*100);
    localTime=time(NULL);
    ptr=localtime(&localTime);
    char* logs_path = "/Users/twx/Desktop/Log-%Y%m%d%H%M.txt";;
    //logs_path=malloc(sizeof(char)*120);
    //logs_path = "/Users/twx/Desktop/Log-%Y%m%d%H%M.txt";
    strftime(str,100,logs_path,ptr);
    printf("%s\n",*(&str));
    return str;
}



int sendmail(){
    int sock;
    struct sockaddr_in server;
    struct hostent *hp, *gethostbyname(const char* name);
    char *host_id="smtp.qq.com";
    
    //=====Create Socket=====//
    printf("\r\n\r\nsocket creating...\n");
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock==-1)
    {
        perror("opening stream socket");
        return 1;
    }
    else
        printf("socket created\n");
    //=====Verify host=====//
    server.sin_family = AF_INET;
    hp = gethostbyname(host_id);
    if (hp==(struct hostent *) 0)
    {
        fprintf(stderr, "%s: unknown host\n", host_id);
        return 2;
    }
    //=====Connect to port 25 on remote host=====//
    memcpy((char *) &server.sin_addr, (char *) hp->h_addr, hp->h_length);
    server.sin_port=htons(25);
    if (connect(sock, (struct sockaddr *) &server, sizeof server)==-1)
    {
        perror("connecting stream socket");
        return 1;
    }
    else
        printf("Connected\n");
    
    //=====Write some data then read some =====//
    read_socket(sock); // SMTP Server WELCOME string
    
    //=====HELO=====//
    sendtext(sock, "", "", mail[0], 0);
    len=read_socket(sock);
    if(len<0) return -1;
    
    //=====AUTH LOGIN=====//
    sendtext(sock, "", "", mail[1], 0);
    len=read_socket(sock);
    if(len<0) return -1;
    
    //=====base64 of username=====//
    sendtext(sock, "", "", mail[2], 0);
    len=read_socket(sock);
    if(len<0) return -1;
    
    //=====base64 of password=====//
    sendtext(sock, "", "", mail[3], 0);
    len=read_socket(sock);
    if(len<0) return -1;
    
    //=====mail from=====//
    sendtext(sock, "", "", mail[4], 0);
    len=read_socket(sock);
    if(len<0) return -1;
    
    //=====rcpt to=====//
    sendtext(sock, "", "", mail[5], 0);
    len=read_socket(sock);
    if(len<0) return -1;
    
    //=====DATA=====//
    sendtext(sock, "", "", mail[6], 0);
    len=read_socket(sock);
    if(len<0) return -1;
    for(int j=7;j<=n-1;j++){
        sendtext(sock, "", "", mail[j], 0);
    }
    len=read_socket(sock);
    if(len<0) return -1;
    
    //=====QUIT=====//
    sendtext(sock, "", "",  "QUIT\r\n", 0);
    len=read_socket(sock);
    if(len<0) return -1;
    
    //====Close socket and finish=====//
    close(sock);
    return 1;
}

int getmail()
{
    FILE *fp=NULL;
    char* mailtxt=log_path();//Path of txt file
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
    
    
    //=====TCP on IPv4=====//
    if((server_sockfd=socket(PF_INET,SOCK_STREAM,0))<0)
    {
        perror("socket");
        return 1;
    }
    //=====reuseaddr=====//
    int on=1;
    if(setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int))<0){
        perror("reuseaddr");
        return 1;
    }
    //=====bind socket to server address=====//
    if (bind(server_sockfd,(struct sockaddr *)&my_addr,sizeof(struct sockaddr))<0)
    {
        perror("bind");
        return 1;
    }
    //=====listen queue=====//
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
    printf("accept client %s by port %d\n",inet_ntoa(remote_addr.sin_addr),htons(remote_addr.sin_port));
    sendtext(client_sockfd, "", "", text[0],1);//WELCOME
    
    
    //=====recv and send messang=====//
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
                sendtext(client_sockfd, "", buf, text[3],1);//mail[2] is base64 of username
                flag++;
                break;
            case 3:
                sendtext(client_sockfd, "", buf, text[4],1);//mail[3] is base64 of password
                flag++;
                break;
            case 4:
                //=====Get from_addr=====//
                if(strncmp(buf,"MAIL FROM",strlen("MAIL FROM"))==0){
                    printf("%s\n",text[5]);
                    from_addr=malloc(sizeof(char)*100);
                    void* start=memchr(buf,'<',sizeof(buf));
                    void* stop = memchr(buf, '>', sizeof(buf));
                    strncpy(from_addr, start+1, stop-start-1);
                    int p=mail_path(from_addr);
                    
                    printf("//================== from_addr: <%s> ==================//\r\n",from_addr);
                    if(p==0)len=send(client_sockfd,text[5],strlen(text[5]),0);
                    else len=send(client_sockfd,errort[1], (int)strlen(errort[1]),0);
                }
                flag++;
                break;
        }
        //=====Get to_addr=====//
        if(strncmp(buf,"RCPT TO",strlen("RCPT TO"))==0){
            printf("%s\n",text[5]);
            to_addr[num_to_addr]=malloc(sizeof(char)*100);
            void* start=memchr(buf,'<',sizeof(buf));
            void* stop = memchr(buf, '>', sizeof(buf));
            strncpy(to_addr[num_to_addr], start+1, stop-start-1);
            int p=mail_path(from_addr);
            printf("//================== to_addr[%d]: <%s> ==================//\r\n",num_to_addr+1,to_addr[num_to_addr]);
            num_to_addr+=1;
            
            if(p==0) len=send(client_sockfd,text[5],strlen(text[5]),0);
            else len=send(client_sockfd,errort[0], (int)strlen(errort[0]),0);
        }
        
        sendtext(client_sockfd, "DATA", buf, text[6],1);
        if(strncmp(buf+strlen(buf)-3,".",strlen("."))==0){
            n=i;
            printf("\r\n=============== sending %d messages ===============\r\n",i);
            if(i>7 && flag>=5) len=sendmail();
            if(len>0){
                len=send(client_sockfd,text[7],strlen(text[7]),0);
                printf("%s\n",text[7]);
            }
            else{
                len=send(client_sockfd,errr,strlen(errr),0);
                printf("%s\n",errr);
            }
            
        }
        
    }
    
    //printf("client %s closed\n\n",inet_ntoa(remote_addr.sin_addr));
    printf("\x1b[%d;%dclient %s closed\n\n\x1b[0m", 46, 43, inet_ntoa(remote_addr.sin_addr));
    close(client_sockfd);
    fclose(fp);
    
    close(server_sockfd);
    return 0;
}


#endif /* get_send_mail_h */
