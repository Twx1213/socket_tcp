//
//  main.c
//  socket_tcp
//
//  Created by Tiko on 11/30/18.
//  Copyright © 2018 Tiko. All rights reserved.
//


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mail_regex.h"
#include "get_send_mail.h"
#include "get_send_mail_ssl.h"


int main(){
    
    while(1){
        //getcwd(local, sizeof(local));
        
        printf("%s\n\n",local);
        num_to_addr=0;
        //ssl_getmail();
        getmail();
        printf("//================== from_addr: <%s> ==================//\r\n",from_addr);
        for(int i=0;i<num_to_addr;i++){
            printf("//================== to_addr[%d]: <%s> ==================//\r\n",i+1,to_addr[i]);
        }
    }
    return 0;
}
