//
// Created by 金国充 on 29/03/2018.
//
#include <iostream>


using namespace std;

#include "StringUtils.h"

void getYuvName(char **name, char **yuvName) {
    char *pRename = *yuvName;

    char *temp = *name;

    printf("name %s\n", temp);
    int flag = -1;
    for (int i = (int) strlen(temp); i > 0; --i) {
        char c = temp[i];
        if (c == '.') {
            flag = i;
            break;
        }
    }
    if (flag == -1) {
        printf("无法解析文件名!");
        return ;
    }
    strncpy(pRename, &*temp, flag);
    strcat(pRename, ".yuv");
    printf("yuvName %s \n", pRename);

}