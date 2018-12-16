//
//  mail_regex.h
//  socket_tcp
//
//  Created by 汤文骁 on 12/16/18.
//  Copyright © 2018 twx. All rights reserved.
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
    
    while(offset < strlen(str))
    {
        int status = regexec(&reg, str + offset, 1, &pmatch, 0);
        /* 匹配正则表达式，注意regexec()函数一次只能匹配一个，不能连续匹配，网上很多示例并没有说明这一点 */
        if(status == REG_NOMATCH){
            printf("No Match\n");
            return -1;
        }
        else if(pmatch.rm_so != -1)
        {
            printf("Matched\n");
            //char *p = getsubstr(str + offset, &pmatch);
            //printf("[%d, %d]: %s\n", offset + pmatch.rm_so + 1, offset + pmatch.rm_eo, p);
        }
        offset += pmatch.rm_eo;
    }
    regfree(&reg);
    return 0;
}

#endif /* mail_regex_h */
