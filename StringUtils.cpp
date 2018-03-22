//
// Created by 金国充 on 02/03/2018.
//

#include <cstring>
#include "StringUtils.h"
void singleCharReplace(char string[], char a, char b){
    for (int i = 0; i < strlen(string); ++i) {
        char c = string[i];
        if (c == a){
            string[i] = b;
        }
    }
}