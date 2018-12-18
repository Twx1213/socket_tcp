//
//  get_send_mail_ssl.h
//  socket_tcp
//
//  Created by Tiko on 12/16/18.
//  Copyright © 2018 Tiko. All rights reserved.
//

#ifndef get_send_mail_ssl_h
#define get_send_mail_ssl_h



#include "Header.h"


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
    printf ("[\x1b[0;%dmsmtp.qq.com\x1b[0m]: \n%s\n", 34, buf);
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
        printf("Connected to \x1b[0;%dm%s\x1b[0m\n\n",34, inet_ntoa(server.sin_addr));
    
    //=====Write some data then read some =====//
    read_socket(sock); // SMTP Server WELCOME string
    
    //=====HELO=====//
    sendtext(sock, "", "", "HELO twx\r\n", 0);
    read_socket(sock);
    
    sendtext(sock, "", "", "STARTTLS\r\n", 0);
    read_socket(sock);
    
    
    SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2);
    ssl = SSL_new (ctx);
    if(ssl>0) printf("new_SSL:\x1b[0;%dm%d\x1b[0m\n", 34, (int)ssl);
    
    SSL_set_fd (ssl, sock);
    
    err = SSL_connect (ssl);
    printf("[\x1b[0;%dmBegin SSL data exchange\x1b[0m]\n",32);
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

int ssl_getmail()
{
    FILE *fp=NULL;
    char* mailtxt=NULL;
    mailtxt=log_path(mailtxt);//Path of txt file
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
    char* cert_path = "/Users/twx/Desktop/NetWork/SMTP/socket_tcp/socket_tcp/cert.pem";;
    //cert_path=malloc(sizeof(char)*120);
    //cert_path = strcat(local,"/cert.pem");
    if (SSL_CTX_use_certificate_file(ctx, cert_path, SSL_FILETYPE_PEM) <= 0)
    {
        ERR_print_errors_fp(stdout);
        exit(999);
    }
    
    /* 载入用户私钥 */
    char* key_path = "/Users/twx/Desktop/NetWork/SMTP/socket_tcp/socket_tcp/key.pem";
    //key_path=malloc(sizeof(char)*120);
    //key_path = strcat(local,"/key.pem");
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
    printf("[\x1b[0;%dmwait for client\x1b[0m]\n",33);
    
    sin_size=sizeof(struct sockaddr_in);
    
    //=====wait for client=====//
    if ((client_sockfd = accept(server_sockfd, (struct sockaddr *) &remote_addr, (socklen_t *)&sin_size)) == -1)
    {
        perror("accept");
        exit(errno);
    }
    else
        printf("[\x1b[0;%dmserver\x1b[0m]: got connection from %s, port %d, socket %d\n\n", 32, inet_ntoa(remote_addr.sin_addr), ntohs(remote_addr.sin_port), client_sockfd);
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
        printf("[\x1b[0;%dmFoxmail\x1b[0m]: \n%s\n",34, buf);
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
                ssl_sendtext(ssl, "", buf, text[3],1);//mail[2] is base64 of username
                flag++;
                break;
            case 3:
                ssl_sendtext(ssl, "", buf, text[4],1);//mail[3] is base64 of password
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
                    if(p==0)len=SSL_write(ssl, text[5], (int)strlen(text[5]));
                    else len=SSL_write(ssl, errort[1], (int)strlen(errort[1]));
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
            
            if(p==0) len=SSL_write(ssl, text[5], (int)strlen(text[5]));
            else len=SSL_write(ssl, errort[0], (int)strlen(errort[0]));
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
                free(errr);
            }
            //break;
        }
        
    }
    
    //printf("client %s closed\n\n",inet_ntoa(remote_addr.sin_addr));
    printf("[\x1b[0;%dmclient %s closed.\x1b[0m]\n\n", 33, inet_ntoa(remote_addr.sin_addr));
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

#endif /* get_send_mail_ssl_h */
