//
// Created by 金国充 on 01/03/2018.
//

#include "StringUtils.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
#include "SDL2/SDL.h"

};

//Refresh Event
#define SFM_REFRESH_EVENT  (SDL_USEREVENT + 1)

#define SFM_BREAK_EVENT  (SDL_USEREVENT + 2)

int thread_exit = 0;
int sfp_refresh_thread2(void *opaque) {
    thread_exit = 0;
    while (!thread_exit) {
        SDL_Event event;
        event.type = SFM_REFRESH_EVENT;
        SDL_PushEvent(&event);
        SDL_Delay(40);
    }
    thread_exit = 0;
    //Break
    SDL_Event event;
    event.type = SFM_BREAK_EVENT;
    SDL_PushEvent(&event);

    return 0;
}

int main() {
    AVFormatContext *pFormatCtx;
    AVCodecParameters *pCodecParams;
    AVCodecContext *pCodecCtx;
    AVCodec *pCodec;
    //注册所有组件。
    av_register_all();
    avformat_network_init();
    pFormatCtx = avformat_alloc_context();
    char videoName[] = "屌丝男士.mov";
    //打开输入视频文件。
    if (avformat_open_input(&pFormatCtx, videoName, nullptr, nullptr) != 0) {
        printf("Couldn't open input stream.\n");
        return -1;
    }
    //获取视频文件信息。
    if (avformat_find_stream_info(pFormatCtx, nullptr) < 0) {
        printf("Couldn't find stream information.\n");
        return -1;
    }
    int i, videoIndex = -1;
    for (i = 0; i < pFormatCtx->nb_streams; i++)
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoIndex = i;
            break;
        }
    if (videoIndex == -1) {//videoIndex == 0
        printf("Didn't find a video stream.\n");
        return -1;
    }
    pCodecParams = pFormatCtx->streams[videoIndex]->codecpar;
    pCodecCtx = pFormatCtx->streams[videoIndex]->codec;
    //查找解码器。
    pCodec = avcodec_find_decoder(pCodecParams->codec_id);
    if (pCodec == nullptr) {
        printf("Codec not found.\n");
        return -1;
    }

//    pCodecCtx = avcodec_alloc_context3(nullptr);
    //打开解码器。
    if (avcodec_open2(pCodecCtx, pCodec, nullptr)) {
        printf("Could not open codec.\n");
        return -1;
    }

    singleCharReplace(videoName, '.', '_');
    strcat(videoName, "_info.txt");
    printf("%s\n", videoName);

    //AVFormatContext
    printf("时长 %" PRIu64 "\n", pFormatCtx->duration);//int64_t
    printf("视频的码率 %" PRIu64 "\n", pFormatCtx->bit_rate);
    printf("AVStream 个数 %zd\n", sizeof(pFormatCtx->nb_streams));//size_t
    printf("streams 个数 %zd\n", sizeof(pFormatCtx->streams));//size_t
    //XXX streams 个数 8 AVStream 个数 4 是不是还有4个是音频?
    printf("filename %s\n", pFormatCtx->filename);

    //AVInputFormat
    AVInputFormat *pInputFormat = pFormatCtx->iformat;
    printf("long name %s\n", pInputFormat->long_name);
    printf("name %s\n", pInputFormat->name);

    //AVStream
    AVStream *pStream = pFormatCtx->streams[videoIndex];
    printf("w * h %d*%d\n", pStream->codecpar->width, pStream->codecpar->height);
    printf("w * h 2 %d*%d\n", pCodecCtx->width, pCodecCtx->height);//0*0 error

    //AVCodecParameters
    AVCodecParameters *pParameters = pStream->codecpar;

    //AVCodecContext pix_fmt
    printf("pix_fmt %d\n", pCodecCtx->pix_fmt);
    printf("sample_rate %d\n", pCodecCtx->sample_rate);
    printf("channels %d\n", pCodecCtx->channels);
    printf("sample_fmt %d\n", pCodecCtx->sample_fmt);

    //AVCodec
    printf("编解码器名称 %s\n", pCodec->name);
    printf("编解码器长名称 %s\n", pCodec->long_name);
    printf("编解码器类型 %d\n", pCodec->type);
    printf("编解码器id %d\n", pCodec->id);

    //AVPacket
    AVPacket *packet;
    packet = (AVPacket *) av_malloc(sizeof(AVPacket));

    //AVFrame

    //存储一帧解码后像素(采样)数据
    AVFrame *pFrame = av_frame_alloc();
    AVFrame *pFrameYUV = av_frame_alloc();
    uint8_t *out_buffer;
    int mmSize = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pParameters->width, pParameters->height, 0);
    printf("mmSize %d\n", mmSize);

    int mmSize2 = avpicture_get_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);
    printf("mmSize2 %d\n", mmSize2);
    out_buffer = (uint8_t *) av_malloc(mmSize2);

//    out_buffer = (uint8_t *) av_malloc((size_t) mmSize);
    avpicture_fill((AVPicture *) pFrameYUV, out_buffer, AV_PIX_FMT_YUV420P, pParameters->width, pParameters->height);

    SwsContext *img_convert_ctx = sws_getContext(pCodecParams->width, pCodecParams->height, pCodecCtx->pix_fmt,
                                                 pParameters->width, pParameters->height, AV_PIX_FMT_YUV420P,
                                                 SWS_BICUBIC, nullptr, nullptr,
                                                 nullptr);

    if (SDL_Init(SDL_INIT_VIDEO)) {
        printf("Could not initialize SDL - %s\n", SDL_GetError());
        return -1;
    }

    //SDL 2.0 Support for multiple windows
    SDL_Window *screen = SDL_CreateWindow("Simplest Video Play SDL2", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                          pParameters->width + 50, pParameters->height + 50,
                                          SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    if (!screen) {
        printf("SDL: could not create window - exiting:%s\n", SDL_GetError());
        return -1;
    }
    SDL_Renderer *sdlRenderer = SDL_CreateRenderer(screen, -1, 0);

    SDL_Texture *sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, pParameters->width,
                                                pParameters->height);

    SDL_Rect sdlRect;
    sdlRect.x = 0;
    sdlRect.y = 0;
    sdlRect.w = pParameters->width;
    sdlRect.h = pParameters->height;

    SDL_Thread *video_tid = SDL_CreateThread(sfp_refresh_thread2, nullptr, nullptr);
    SDL_Event event;
    int ret, got_picture;
    for (;;) {
        if(SDL_PushEvent(&event)){
            if (event.type == SDL_KEYDOWN){
                printf("SDL_KEYDOWN\n");
            }
        }
        //Wait
        SDL_WaitEvent(&event);
        if (event.type == SFM_REFRESH_EVENT) {
            //------------------------------
            //从输入文件读取一帧压缩数据。
            if (av_read_frame(pFormatCtx, packet) >= 0) {
                if (packet->stream_index == videoIndex) {
                    //解码一帧压缩数据。
                    ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
                    if (ret < 0) {
                        printf("Decode Error.\n");
                        return -1;
                    }
                    if (got_picture) {
                        sws_scale(img_convert_ctx, (const uint8_t *const *) pFrame->data, pFrame->linesize, 0,
                                  pCodecParams->height, pFrameYUV->data, pFrameYUV->linesize);
                        //SDL---------------------------
                        SDL_UpdateTexture(sdlTexture, nullptr, pFrameYUV->data[0], pFrameYUV->linesize[0]);
                        SDL_RenderClear(sdlRenderer);
                        //SDL_RenderCopy( sdlRenderer, sdlTexture, &sdlRect, &sdlRect );
                        SDL_RenderCopy(sdlRenderer, sdlTexture, nullptr, nullptr);
                        SDL_RenderPresent(sdlRenderer);
                        //SDL End-----------------------
                    }
                }
                av_free_packet(packet);
            } else {
                //Exit Thread
                thread_exit = 1;
            }
        } else if (event.type == SDL_QUIT) {
            thread_exit = 1;
        } else if (event.type == SFM_BREAK_EVENT) {
            break;
        }

    }

    sws_freeContext(img_convert_ctx);

    SDL_Quit();
    //--------------
    av_frame_free(&pFrameYUV);
    av_frame_free(&pFrame);
    avcodec_close(pCodecCtx);//关闭解码器。
    avformat_close_input(&pFormatCtx);//关闭输入视频文件。

    return 0;

    /*FILE *videoInfo = fopen(videoName, "wb+,ccs=UTF-8");
    fprintf(videoInfo, "时长 pFormatCtx->duration %" PRIu64 "\n", pFormatCtx->duration);
    fprintf(videoInfo, "w*h %d*%d", pFormatCtx->streams[videoIndex]->codecpar->width,
            pFormatCtx->streams[videoIndex]->codecpar->height);
    fclose(videoInfo);*/

}