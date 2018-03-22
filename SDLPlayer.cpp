//
// Created by 金国充 on 05/03/2018.
//

#include <iostream>

using namespace std;

extern "C" {
#include <SDL2/SDL.h>
}


int sdlPlay(unsigned int width, unsigned int height);


int main() {
    sdlPlay(640, 360);
    return 0;
}

int sdlPlay(unsigned int width, unsigned int height) {


    //------------SDL----------------
    int screen_w, screen_h;
    SDL_Window *screen;
    SDL_Renderer *sdlRenderer;
    SDL_Texture *sdlTexture;
    SDL_Rect *sdlRect;
    SDL_Thread *video_tid;
    SDL_Event event;
    //每个像素占了多少比特
    unsigned int bpp = 12;
    unsigned char buffer[width * height * bpp / 8];

    FILE *fp = nullptr;
    fp = fopen("sintel_640_360.yuv", "rb+");

    if (fp == nullptr) {
        printf("cannot open this file\n");
        return -1;
    }

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
        printf("Could not initialize SDL - %s\n", SDL_GetError());
        return -1;
    }
    //SDL 2.0 Support for multiple windows
    screen_w = width;
    screen_h = height;
    screen = SDL_CreateWindow("Simplest ffmpeg player's Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              screen_w, screen_h, SDL_WINDOW_OPENGL);

    if (!screen) {
        printf("SDL: could not create window - exiting:%s\n", SDL_GetError());
        return -1;
    }
    sdlRenderer = SDL_CreateRenderer(screen, -1, 0);
    //IYUV: Y + U + V  (3 planes)
    //YV12: Y + V + U  (3 planes)
    sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, width, height);

    sdlRect->x = 0;
    sdlRect->y = 0;
    sdlRect->w = screen_w;
    sdlRect->h = screen_h;

    //packet = (AVPacket *) av_malloc(sizeof(AVPacket));

    //------------SDL End------------
    //Event Loop

    //file 长度 34560000

    //播放一遍
    //for (unsigned int i = 0; i < 34560000 / (width * height * bpp / 8); i++) {
    for (;;) {

        if (fread(buffer, 1, width * height * bpp / 8, fp) != width * height * bpp / 8) {
            // Loop
            fseek(fp, 0, SEEK_SET);
            fread(buffer, 1, width * height * bpp / 8, fp);
        }
        //SDL---------------------------
        SDL_UpdateTexture(sdlTexture, nullptr, buffer, width);//XXX 耗时
        SDL_RenderClear(sdlRenderer);
        SDL_RenderCopy(sdlRenderer, sdlTexture, nullptr, nullptr);
        SDL_RenderPresent(sdlRenderer);
        SDL_Delay(40);
        //SDL End-----------------------
    }
    fclose(fp);
    SDL_Quit();
    //--------------
    return 0;
}








