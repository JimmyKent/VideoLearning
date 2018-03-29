//
// Created by 金国充 on 23/03/2018.
//


#include <iostream>

using namespace std;
struct Test1 {
    char c1;
    char c2;
    short s1;
    int i1;
};

struct Test2 {
    char c1;
    short s1;
    char c2;
    int i1;
};
struct Test2_2 {
    char c1;
    int i1;
    short s1;
    char c2;

};

struct Test3 {
    char c1;
    short s1;
    char c2;
    int i1;

    char c3;
    short s2;
    char c4;
    int i2;
};

int main() {
    Test1 t1;
    Test2 t2;
    Test3 t3;

    printf("%zu\n", sizeof(t1));
    printf("%zu\n", sizeof(t2));
    printf("%zu\n", sizeof(t3));

}


