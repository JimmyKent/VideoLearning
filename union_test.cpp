//
// Created by 金国充 on 01/03/2018.
//

#include <stdio.h>
#include <string.h>

union Data
{
    int i;
    float f;
    char  str[20];
};

int main( )
{
    union Data data;
    printf( "data : %p\n", &data);
    data.i = 10;
    data.f = 220.5;
    strcpy( data.str, "C Programming");

    printf( "data.i : %d\n", data.i);
    printf( "data.f : %f\n", data.f);
    printf( "data.str : %s\n", data.str);

    return 0;
}