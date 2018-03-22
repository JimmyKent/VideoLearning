//
// Created by 金国充 on 24/02/2018.
//



#include <iostream>

using namespace std;


extern "C"
{
//#include <stdio.h>
#include "SDL2/SDL.h"
}
//每个像素占了多少比特
const int bpp = 12;

int screen_w = 640, screen_h = 360;
const int pixel_w = 640, pixel_h = 360;

unsigned char buffer[pixel_w * pixel_h * bpp / 8];


//Refresh Event
#define REFRESH_EVENT  (SDL_USEREVENT + 1)
int thread_exit = 0;
int thread_pause = 0;

int refresh_video(void *opaque) {
    while (1) {
        if (thread_pause == 0) {
            SDL_Event event;
            event.type = REFRESH_EVENT;
            SDL_PushEvent(&event);
        }
        SDL_Delay(1000);
    }
    return 0;
}

int main(int argc, char *argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO)) {
        printf("Could not initialize SDL - %s\n", SDL_GetError());
        return -1;
    }

    SDL_Window *screen;
    //SDL 2.0 Support for multiple windows
    screen = SDL_CreateWindow("Simplest Video Play SDL2", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              screen_w + 50, screen_h + 50, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!screen) {
        printf("SDL: could not create window - exiting:%s\n", SDL_GetError());
        return -1;
    }
    SDL_Renderer *sdlRenderer = SDL_CreateRenderer(screen, -1, 0);

    Uint32 pixformat = 0;
    //IYUV: Y + U + V  (3 planes)
    //YV12: Y + V + U  (3 planes)
    pixformat = SDL_PIXELFORMAT_IYUV;

    SDL_Texture *sdlTexture = SDL_CreateTexture(sdlRenderer, pixformat, SDL_TEXTUREACCESS_STREAMING, pixel_w, pixel_h);

    FILE *fp = NULL;
    fp = fopen("sintel_640_360.yuv", "rb+");

    if (fp == NULL) {
        printf("cannot open this file\n");
        return -1;
    }
//    fseek(fp, 0, SEEK_END); // seek to end of file
//    long size = ftell(fp); // get current file pointer
//    printf("  size %ld ",size);

    SDL_Event event;SDL_Rect sdlRect;

    SDL_Thread *refresh_thread = SDL_CreateThread(refresh_video, NULL, NULL);

    unsigned long i = 0;
    while (i < 1 * 230400) {
        i++;
        //Wait
        SDL_WaitEvent(&event);
        //printf("Event trigger!\n");
        if (event.type == REFRESH_EVENT) {
            printf("Receiveing REFRESH_EVENT\n");
        } else if (event.type == SDL_KEYDOWN) {
            //Pause
            if (event.key.keysym.sym == SDLK_SPACE) {
                thread_pause = !thread_pause;
                printf("Key SPACE has been pressed!!\n");
            } else if (event.key.keysym.sym == SDLK_a)
                printf("Key a has been pressed!!\n");
            else if (event.key.keysym.sym == SDLK_b)
                printf("Key b has been pressed!!\n");

        }
        //bpp / 8 == 1.5 : y 1 u 1/4 v 1/4 所以总数是1.5
        if (fread(buffer, 1, pixel_w * pixel_h * bpp / 8, fp) != pixel_w * pixel_h * bpp / 8) {
            // Loop
            fseek(fp, 0, SEEK_SET);
            fread(buffer, 1, pixel_w * pixel_h * bpp / 8, fp);
        }

        SDL_UpdateTexture(sdlTexture, NULL, buffer, pixel_w);

        sdlRect.x = 0;
        sdlRect.y = 0;
        sdlRect.w = screen_w;
        sdlRect.h = screen_h;

        SDL_RenderClear(sdlRenderer);
        SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, &sdlRect);
        SDL_RenderPresent(sdlRenderer);
        //Delay 40ms 速度 因为每秒25帧,所以延时40毫秒
        SDL_Delay(40);

    }
    SDL_Quit();
    return 0;
}
