//
//  mail_regex.h
//  socket_tcp
//
//  Created by Tiko on 12/16/18.
//  Copyright © 2018 Tiko. All rights reserved.
//

#ifndef mail_regex_h
#define mail_regex_h

#include <regex.h>

int mail_path(const char* str){
    const char* pattern="([0-9A-Za-z\\-_\\.]+)@([0-9a-z]+\\.[a-z]{2,3}(\\.[a-z]{2})?)";
    regmatch_t pmatch;
    regex_t reg;
    regcomp(&reg, pattern, REG_EXTENDED);
    unsigned long offset = 0;
    int err=0;
    
    while((offset < strlen(str))&&err==0)
    {
        int status = regexec(&reg, str + offset, 1, &pmatch, 0);
        /* 匹配正则表达式，注意regexec()函数一次只能匹配一个，不能连续匹配，网上很多示例并没有说明这一点 */
        if(status == REG_NOMATCH){
            //printf("No Match\n");
            printf("\x1b[0;%dm[No Match] \n\x1b[0m", 31);//red font
            err=-1;
        }
        else if(pmatch.rm_so != -1)
        {
            //printf("Matched\n");
            printf("\x1b[0;%dm[Matched] \n\x1b[0m", 32);//green font
            //char *p = getsubstr(str + offset, &pmatch);
            //printf("[%d, %d]: %s\n", offset + pmatch.rm_so + 1, offset + pmatch.rm_eo, p);
        }
        offset += pmatch.rm_eo;
    }
    regfree(&reg);
    return err;
}

#endif /* mail_regex_h */
