//
//  main.c
//  socket_tcp
//
//  Created by Tiko on 11/30/18.
//  Copyright © 2018 Tiko. All rights reserved.
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
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <err.h>
#include <time.h>



char* mail[20]={0};
char buf[BUFSIZ+1];
char* from_addr;
char* to_addr[10];
int num_to_addr=0;
int n=0;
long len=0;
char* errort[5]={
    "503 Error: need EHLO and AUTH first !",
    "502 Error: auth command not implemented"
};
char* errr;
char local[180];

//=====Log Date=====//
char* log_path(){
    struct tm *ptr;
    time_t localTime;
    char* str;
    str = malloc(sizeof(char)*100);
    localTime=time(NULL);
    ptr=localtime(&localTime);
    char* logs_path;
    logs_path=malloc(sizeof(char)*120);
    logs_path = strcat(*(&local),"/Log-%Y%m%d%H%M.txt");
    strftime(str,100,logs_path,ptr);
    printf("%s\n",*(&str));
    return str;
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
int ssl_send_line(SSL* ssl,char* cmd)
{
    unsigned long err;
    printf("C: %s",cmd);
    err = SSL_write (ssl, cmd, (int)strlen(cmd));
    return 1;
}
int ssl_get_line(SSL* ssl)
{
    int err;
    err = SSL_read (ssl, buf, sizeof(buf) - 1);
    buf[err] = '\0';
    if(strncmp(buf,"5",1)==0){
        err=-1;
        errr=malloc(sizeof(char)*sizeof(buf));
        strcpy(errr,&(*buf));
    }
    printf ("S: %s", buf);
    return err;
}

int ssl_sendmail(){
    int sock;
    struct sockaddr_in server;
    struct hostent *hp, *gethostbyname(const char* name);
    char *host_id="smtp.qq.com";
    int err;
    SSL_CTX* ctx;
    SSL* ssl;
    SSL_METHOD* meth;
    
    //=====SSL init=====//
    SSL_library_init();
    SSL_load_error_strings();
    meth=(SSL_METHOD*)SSLv23_method();
    ctx = SSL_CTX_new (meth);
    
    /*=====Create Socket=====*/
    printf("socket creating...\n");
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock==-1)
    {
        perror("opening stream socket");
        return -1;
    }
    else
        printf("socket created\n");
    /*=====Verify host=====*/
    server.sin_family = AF_INET;
    hp = gethostbyname(host_id);
    
    if (hp==(struct hostent *) 0)
    {
        fprintf(stderr, "%s: unknown host\n", host_id);
        return -2;
    }
    
    //=====Connect to port 25 on remote host=====//
    memcpy((char *) &server.sin_addr, (char *) hp->h_addr, hp->h_length);
    server.sin_port=htons(25);
    if (connect(sock, (struct sockaddr *) &server, sizeof server)==-1)
    {
        perror("connecting stream socket");
        return -1;
    }
    else
        printf("Connected to %s\n",inet_ntoa(server.sin_addr));
    
    //=====Write some data then read some =====//
    read_socket(sock); // SMTP Server WELCOME string
    
    //=====HELO=====//
    sendtext(sock, "", "", "HELO twx\r\n", 0);
    read_socket(sock);
    
    sendtext(sock, "", "", "STARTTLS\r\n", 0);
    read_socket(sock);
    
    
    SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2);
    ssl = SSL_new (ctx);
    if(ssl>0) printf("new_SSL:%d\n", (int)ssl);
    SSL_set_fd (ssl, sock);
    
    err = SSL_connect (ssl);
    printf ("SSL connection using %s\n", SSL_get_cipher (ssl));
    printf("Begin SSL data exchange\n");
    //=====SSL HELO=====//
    ssl_send_line(ssl, mail[0]);
    len=ssl_get_line(ssl);
    if(len<0) return -1;
    //=====AUTH LOGIN=====//
    ssl_send_line(ssl, mail[1]);
    len=ssl_get_line(ssl);
    if(len<0) return -1;
    
    //=====base64 of username=====//
    ssl_send_line(ssl, mail[2]);
    len=ssl_get_line(ssl);
    if(len<0) return -1;
    
    //=====base64 of password=====//
    ssl_send_line(ssl, mail[3]);
    len=ssl_get_line(ssl);
    if(len<0) return -1;
    
    //=====mail from=====//
    ssl_send_line(ssl, mail[4]);
    len=ssl_get_line(ssl);
    if(len<0) return -1;
    
    //=====rcpt to=====//
    ssl_send_line(ssl, mail[5]);
    len=ssl_get_line(ssl);
    if(len<0) return -1;
    
    //=====DATA=====//
    ssl_send_line(ssl, mail[6]);
    len=ssl_get_line(ssl);
    if(len<0) return -1;
    for(int j=7;j<n;j++){
        ssl_send_line(ssl, mail[j]);
    }
    len=ssl_get_line(ssl);
    if(len<0) return -1;
    
    //=====QUIT=====//
    ssl_send_line(ssl, "QUIT\r\n");
    len=ssl_get_line(ssl);
    if(len<0) return -1;
    printf("\r\n==================================OK====================================\r\n");
    //=====Close socket and finish=====//
    close(sock);
    SSL_shutdown (ssl); // send SSL/TLS close_notify
    shutdown (sock,2);
    SSL_free (ssl);
    SSL_CTX_free (ctx);
    return 1;
}

/*int sendmail(){
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
    read_socket(sock);
    
    //=====AUTH LOGIN=====//
    sendtext(sock, "", "", mail[1], 0);
    read_socket(sock);
    
    //=====base64 of username=====//
    sendtext(sock, "", "", mail[2], 0);
    read_socket(sock);
    
    //=====base64 of password=====//
    sendtext(sock, "", "", mail[3], 0);
    read_socket(sock);
    
    //=====mail from=====//
    sendtext(sock, "", "", mail[4], 0);
    read_socket(sock);
    
    //=====rcpt to=====//
    sendtext(sock, "", "", mail[5], 0);
    read_socket(sock);
    
    //=====DATA=====//
    sendtext(sock, "", "", mail[6], 0);
    read_socket(sock);
    for(int j=7;j<n-1;j++){
        sendtext(sock, "", "", mail[j], 0);\
    }
    read_socket(sock);
    
    //=====QUIT=====//
    sendtext(sock, "", "",  "QUIT", 0);
    read_socket(sock);
    
    //====Close socket and finish=====//
    close(sock);
    return 0;
}*/

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
                sendtext(client_sockfd, "", buf, text[3],1);//mail[2] is base64 of username Mjc3ODAxMDYwNkBxcS5jb20=  
                flag++;
                break;
            case 3:
                sendtext(client_sockfd, "", buf, text[4],1);//mail[3] is base64 of password bmdlb2t2a3RmZml1ZGVjYg==
                flag++;
                break;
            case 4:
                //=====Get from_addr=====//
                if(strncmp(buf,"MAIL FROM",strlen("MAIL FROM"))==0){
                    printf("%s\n",text[5]);
                    from_addr=malloc(sizeof(char)*100);
                    strncpy(from_addr, buf+strlen("MAIL FROM:<"), len-strlen("MAIL FROM:<")-3);
                    len=send(client_sockfd,text[5],strlen(text[5]),0);
                }
                flag++;
                break;
        }
        //=====Get to_addr=====//
        if(strncmp(buf,"RCPT TO",strlen("RCPT TO"))==0){
            printf("%s\n",text[5]);
            to_addr[num_to_addr]=malloc(sizeof(char)*100);
            strncpy(to_addr[num_to_addr], buf+strlen("RCPT TO:<"), len-strlen("RCPT TO:<")-3);
            num_to_addr+=1;
            len=send(client_sockfd,text[5],strlen(text[5]),0);
        }
        
        sendtext(client_sockfd, "DATA", buf, text[6],1);
        if(strncmp(buf+strlen(buf)-3,".",strlen("."))==0){
            n=i;
            printf("\r\n============================ sending %d messages ===========================\r\n",i);
            if(i>7 && flag>=5) len=ssl_sendmail();
            if(len>0){
                len=send(client_sockfd,text[7],strlen(text[7]),0);
                printf("%s\n",text[7]);
            }
            else{
                len=send(client_sockfd,errort[0],strlen(errort[0]),0);
                printf("%s\n",errort[0]);
            }
            
        }
        
    }
    
    printf("client %s closed\n\n",inet_ntoa(remote_addr.sin_addr));
    close(client_sockfd);
    fclose(fp);
    
    close(server_sockfd);
    return 0;
}

int ssl_getmail()
{
    FILE *fp=NULL;
    char* mailtxt=log_path();//Path of txt file
    int server_sockfd;
    int client_sockfd = 0;
    struct sockaddr_in my_addr;
    struct sockaddr_in remote_addr;
    int sin_size;
    
    SSL *ssl;
    SSL_CTX *ctx;
    char* text[10]={
        "220 smtp.TT.com\r\n",
        "250-smtp.TT.com\r\n250-PIPELINING\r\n250-AUTH LOGIN\r\n250-MAILCOMPRESS\r\n250 8BITMIME\r\n",
        "334 VXNlcm5hbWU6\r\n",
        "334 UGFzc3dvcmQ6\r\n",
        "235 Authentication successful\r\n",
        "250 OK\r\n",
        "354 End data with <CR><LF>.<CR><LF>\r\n",
        "250 Ok: queued as\r\n",
        "221 Bye\r\n",
    };
    
    
    memset(&my_addr,0,sizeof(my_addr));
    my_addr.sin_family=AF_INET;//ipv4
    my_addr.sin_addr.s_addr=INADDR_ANY;
    my_addr.sin_port=htons(25);//SMTP port
    /* SSL 库初始化 */
    SSL_library_init();
    /* 载入所有 SSL 算法 */
    OpenSSL_add_all_algorithms();
    /* 载入所有 SSL 错误消息 */
    SSL_load_error_strings();
    /* 以 SSL V2 和 V3 标准兼容方式产生一个 SSL_CTX ，即 SSL Content Text */
    ctx = SSL_CTX_new(SSLv23_server_method());
    /* 也可以用 SSLv2_server_method() 或 SSLv3_server_method() 单独表示 V2 或 V3标准 */
    if (ctx == NULL)
    {
        ERR_print_errors_fp(stdout);
        return 1;
    }
    /* 载入用户的数字证书， 此证书用来发送给客户端。 证书里包含有公钥 */
    char* cert_path;
    cert_path=malloc(sizeof(char)*120);
    cert_path = strcat(*(&local),"/cert.pem");
    if (SSL_CTX_use_certificate_file(ctx, cert_path, SSL_FILETYPE_PEM) <= 0)
    {
        ERR_print_errors_fp(stdout);
        exit(999);
    }
    
    /* 载入用户私钥 */
    char* key_path;
    key_path=malloc(sizeof(char)*120);
    key_path = strcat(*(&local),"/key.pem");
    if (SSL_CTX_use_PrivateKey_file(ctx, key_path, SSL_FILETYPE_PEM) <= 0)
    {
        ERR_print_errors_fp(stdout);
        exit(998);
    }
    /* 检查用户私钥是否正确 */
    if (!SSL_CTX_check_private_key(ctx))
    {
        ERR_print_errors_fp(stdout);
        exit(1);
    }
    
    
    
    
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
    
    //=====wait for client=====//
    if ((client_sockfd = accept(server_sockfd, (struct sockaddr *) &remote_addr, (socklen_t *)&sin_size)) == -1)
    {
        perror("accept");
        exit(errno);
    }
    else
        printf("server: got connection from %s, port %d, socket %d\n", inet_ntoa(remote_addr.sin_addr), ntohs(remote_addr.sin_port), client_sockfd);
    fp=fopen(mailtxt, "w+");
    
    
    
    
    // 基于 ctx 产生一个新的 SSL //
    ssl = SSL_new(ctx);
    // 将连接用户的 socket 加入到 SSL //
    SSL_set_fd(ssl, client_sockfd);
    printf("Add to SSl.\n");
    // 建立 SSL 连接 //
    if (SSL_accept(ssl) == -1)
    {
        perror("accept");
        close(client_sockfd);
        return -3;
    }
    printf("Connected.\n");
    len=SSL_write(ssl,text[0], (int)strlen(text[0]));//WELCOME
    printf("%s\r\n",text[0]);
    
    
    
    //=====recv and send messang=====//
    int i=0;
    int flag=0;
    while(len>=0)
    {
        len=SSL_read(ssl, buf, BUFSIZ);
        //len=(recv(client_sockfd,buf,BUFSIZ,0));
        buf[len]='\0';
        mail[i] = malloc(sizeof(char)*len);
        strcpy(mail[i],&(*buf));
        i++;
        printf("%s\n",buf);
        fprintf(fp,"%s", buf);
        if(strncmp(buf,"QUIT",4)==0){
            len=-2;
            SSL_write(ssl,text[8], (int)strlen(text[8]));
            close(client_sockfd);
            break;
        }
        switch(flag){
            case 0:
                ssl_sendtext(ssl, "EHLO", buf, text[1],1);
                flag++;
                break;
            case 1:
                ssl_sendtext(ssl, "AUTH LOGIN", buf, text[2],1);
                flag++;
                break;
            case 2:
                ssl_sendtext(ssl, "", buf, text[3],1);//mail[2] is base64 of username Mjc3ODAxMDYwNkBxcS5jb20=
                flag++;
                break;
            case 3:
                ssl_sendtext(ssl, "", buf, text[4],1);//mail[3] is base64 of password bmdlb2t2a3RmZml1ZGVjYg==
                flag++;
                break;
            case 4:
                //=====Get from_addr=====//
                
                if(strncmp(buf,"MAIL FROM",strlen("MAIL FROM"))==0){
                    printf("%s\n",text[5]);
                    from_addr=malloc(sizeof(char)*100);
                    void* start=memchr(buf,'<',sizeof(buf));
                    void* stop = memchr(buf, '>', sizeof(buf));
                    //strncpy(from_addr, buf+strlen("MAIL FROM:<"), len-strlen("MAIL FROM:<")-3);
                    strncpy(from_addr, start+1, stop-start-1);
                    printf("//================== from_addr: <%s> ==================//\r\n",from_addr);
                    len=SSL_write(ssl, text[5], (int)strlen(text[5]));
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
            //strncpy(to_addr[num_to_addr], buf+strlen("RCPT TO:<"), len-strlen("RCPT TO:<")-3);
            strncpy(to_addr[num_to_addr], start+1, stop-start-1);
            printf("//================== to_addr[%d]: <%s> ==================//\r\n",num_to_addr+1,to_addr[num_to_addr]);
            num_to_addr+=1;
            
            len=SSL_write(ssl, text[5], (int)strlen(text[5]));
        }
        
        ssl_sendtext(ssl, "DATA", buf, text[6],1);
        if(strncmp(buf+strlen(buf)-3,".",strlen("."))==0){
            n=i;
            printf("\r\n============================ sending %d messages ===========================\r\n",i);
            if(i>7 && flag>=5) len=ssl_sendmail();
            if(len>0){
                len=SSL_write(ssl, text[7], (int)strlen(text[7]));
                printf("%s\n",text[7]);
            }
            else{
                len=SSL_write(ssl, errr, (int)strlen(errr));
                printf("%s\n",errr);
            }
            //break;
        }
        
    }
    
    printf("client %s closed\n\n",inet_ntoa(remote_addr.sin_addr));
    close(client_sockfd);
    fclose(fp);
    // 关闭 SSL 连接 //
    SSL_shutdown(ssl);
    // 释放 SSL //
    SSL_free(ssl);
    close(server_sockfd);
    SSL_CTX_free(ctx);
    return 0;
}




int main(){
    while(1){
        getcwd(local, sizeof(local));
        printf("%s\n\n",local);
        num_to_addr=0;
        ssl_getmail();
        printf("//================== from_addr: <%s> ==================//\r\n",from_addr);
        for(int i=0;i<num_to_addr;i++){
            printf("//================== to_addr[%d]: <%s> ==================//\r\n",i+1,to_addr[i]);
        }
    }
    return 0;
}
