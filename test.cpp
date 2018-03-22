//
// Created by 金国充 on 24/02/2018.
//

//#include <iostream>
//
//using namespace std;
#include <iostream>

extern "C"
{
#include <libavcodec/avcodec.h>

}

int main(int argc, char* argv[]){
    printf("%s", avcodec_configuration());
    return 0;
}