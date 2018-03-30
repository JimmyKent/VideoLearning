//
// Created by 金国充 on 11/03/2018.
// 添加队列
// 在PlayVideo2的基础,如果先切割再显示, 显示不对,
// 改成SDL_UpdateYUVTexture就行了
//

#include <iostream>
#include <queue>

using namespace std;

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
#include "SDL2/SDL.h"
};


//Titanic.ts
char FILE_NAME[] = "屌丝男士.mov";


bool enQueue(const AVFrame *frame);

std::queue<AVFrame *> queue1;

int main() {

    //ffmpeg

    AVFormatContext *pFormatCtx = nullptr;
    AVCodecContext *pVideoCodecCtx = nullptr;
    AVCodec *pVideoCodec = nullptr;
    AVPacket *pVideoPacket = nullptr;

    int videoIndex = -1;

    //------------SDL----------------
    SDL_Window *screen = nullptr;
    SDL_Renderer *sdlRenderer = nullptr;
    SDL_Texture *sdlTexture = nullptr;
    SDL_Rect sdlRect;

    //video info
    int width = 0, height = 0;

    av_register_all();

    if (avformat_open_input(&pFormatCtx, FILE_NAME, nullptr, nullptr) != 0) {
        printf("Can't open input stream.\n");
        return -1;
    }
    if (avformat_find_stream_info(pFormatCtx, nullptr) < 0) {
        printf("Can't find stream info.\n");
        return -1;
    }
    printf("pFormatContext->nb_streams : %d \n", pFormatCtx->nb_streams);
    for (int i = 0; i < pFormatCtx->nb_streams; i++) {
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoIndex = i;
            break;
        }
    }

    printf("videoIndex : %d \n", videoIndex);

    if (videoIndex == -1) {
        printf("Can't find video stream.\n");
        return -1;
    }

    pVideoCodec = avcodec_find_decoder(pFormatCtx->streams[videoIndex]->codecpar->codec_id);
    pVideoCodecCtx = avcodec_alloc_context3(pVideoCodec);// 需要释放
    avcodec_parameters_to_context(pVideoCodecCtx, pFormatCtx->streams[videoIndex]->codecpar);

    if (pVideoCodec == nullptr) {
        printf("pVideoCodec not found.\n");
        return -1;
    }

    if (avcodec_open2(pVideoCodecCtx, pVideoCodec, nullptr) < 0) {
        printf("Could not open pVideoCodec.\n");
        return -1;
    }

    width = pVideoCodecCtx->width;
    height = pVideoCodecCtx->height;
    printf("pVideoCodecCtx->width,pVideoCodecCtx->height : %d, %d \n", pVideoCodecCtx->width, pVideoCodecCtx->height);


    //---------------SDL-----------------------
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
        printf("Could not initialize SDL - %s\n", SDL_GetError());
        return -1;
    }

    //SDL 2.0 Support for multiple windows

    screen = SDL_CreateWindow("Simplest ffmpeg player's Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              width, height, SDL_WINDOW_OPENGL);

    if (!screen) {
        printf("SDL: could not create window - exiting:%s\n", SDL_GetError());
        return -1;
    }
    sdlRenderer = SDL_CreateRenderer(screen, -1, 0);

    sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, width, height);

    sdlRect.x = 0;
    sdlRect.y = 0;
    sdlRect.w = width;
    sdlRect.h = height;

    pVideoPacket = (AVPacket *) av_malloc(sizeof(AVPacket));

    //------------SDL End------------

    AVFrame *pFrameOri = nullptr, *pFrameYUV = nullptr;
    pFrameOri = av_frame_alloc();
    pFrameYUV = av_frame_alloc();

    //初始化pFrameYUV
    pFrameYUV->width = width;
    pFrameYUV->height = height;
    pFrameYUV->format = AV_PIX_FMT_YUV420P;

//    int numBytes = avpicture_get_size((AVPixelFormat) pFrameYUV->format, pFrameYUV->width, pFrameYUV->height);
//    uint8_t *buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));
//
//    avpicture_fill((AVPicture *) pFrameYUV, buffer, (AVPixelFormat) pFrameYUV->format, width, height);

    unsigned char *out_buffer;
    out_buffer = (unsigned char *) av_malloc(
            (size_t) av_image_get_buffer_size(AV_PIX_FMT_YUV420P, width, height, 1));

    av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, out_buffer,
                         AV_PIX_FMT_YUV420P, width, height, 1);

    SwsContext *sws_ctx = sws_getContext(width, height, pVideoCodecCtx->pix_fmt, width, height,
                                         AV_PIX_FMT_YUV420P, SWS_BILINEAR, nullptr, nullptr, nullptr);


    queue<AVFrame *> queue1;

    while (av_read_frame(pFormatCtx, pVideoPacket) >= 0) {
        if (pVideoPacket->stream_index == videoIndex) {
            if (avcodec_send_packet(pVideoCodecCtx, pVideoPacket) >= 0) {
                if (avcodec_receive_frame(pVideoCodecCtx, pFrameOri) >= 0) {
                    // pFrameOri 这样一帧的数据是大的, 要切割
                    int sliceHeight = sws_scale(sws_ctx, (uint8_t const *const *) pFrameOri->data,
                                                pFrameOri->linesize,
                                                0, height, pFrameYUV->data, pFrameYUV->linesize);
                    //enQueue(pFrameYUV);
                    AVFrame *p = av_frame_alloc();
                    int ret = av_frame_ref(p, pFrameYUV);
                    if (ret < 0) {
                        break;
                    }
                    queue1.push(p);
                }
            }
        }
        av_packet_unref(pVideoPacket);
    }

    while (!queue1.empty()) {

        auto displayFrame = queue1.front();
        queue1.pop();

        //SDL---------------------------
        SDL_UpdateYUVTexture(sdlTexture, nullptr,
                             displayFrame->data[0], displayFrame->linesize[0],
                             displayFrame->data[1], displayFrame->linesize[1],
                             displayFrame->data[2], displayFrame->linesize[2]
        );
        SDL_RenderClear(sdlRenderer);
        SDL_RenderCopy(sdlRenderer, sdlTexture, &sdlRect, &sdlRect);
        SDL_RenderPresent(sdlRenderer);
        //SDL End-----------------------

        SDL_Delay(20);

    }

    SDL_Quit();
    //--------------
    av_packet_free(&pVideoPacket);
    av_frame_free(&pFrameOri);
    av_frame_free(&pFrameYUV);
    avcodec_close(pVideoCodecCtx);
    avformat_close_input(&pFormatCtx);

    return 0;
}

bool enQueue(const AVFrame *frame) {
    AVFrame *p = av_frame_alloc();

    int ret = av_frame_ref(p, frame);
    if (ret < 0)
        return false;

    //p->opaque = (void *) new double(*(double *) p->opaque); //上一个指向的是一个局部的变量，这里重新分配pts空间

    queue1.push(p);


    return true;
}



