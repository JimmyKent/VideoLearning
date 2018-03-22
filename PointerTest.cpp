//
// Created by 金国充 on 05/03/2018.
//
#include<iostream>

using namespace std;

struct Student {
    char name[1];
    int id;

};
extern void structTest();
void structTest() {

    struct Student *student1;
    printf("student1        %p\n", &student1);
    cout << "student1: \t\t" << "所占字节数：" << sizeof(student1) << "\n";
    printf("student1 id     %p\n", &(student1->id));
    printf("student1 name   %p\n", &(student1->name));

    char name[1];
    cout << "char: \t\t" << "所占字节数：" << sizeof(char) << "\n";
    cout << "char: \t\t" << "所占字节数：" << sizeof(name);
}


