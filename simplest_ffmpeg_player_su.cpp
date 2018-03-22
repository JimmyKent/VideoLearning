/**
 * 最简单的基于FFmpeg的视频播放器2(SDL升级版)
 * Simplest FFmpeg Player 2(SDL Update)
 *
 * 雷霄骅 Lei Xiaohua
 * leixiaohua1020@126.com
 * 中国传媒大学/数字电视技术
 * Communication University of China / Digital TV Technology
 * http://blog.csdn.net/leixiaohua1020
 *
 * 第2版使用SDL2.0取代了第一版中的SDL1.2
 * Version 2 use SDL 2.0 instead of SDL 1.2 in version 1.
 *
 * 本程序实现了视频文件的解码和显示(支持HEVC，H.264，MPEG2等)。
 * 是最简单的FFmpeg视频解码方面的教程。
 * 通过学习本例子可以了解FFmpeg的解码流程。
 * 本版本中使用SDL消息机制刷新视频画面。
 * This software is a simplest video player based on FFmpeg.
 * Suitable for beginner of FFmpeg.
 *
 * 备注:
 * 标准版在播放视频的时候，画面显示使用延时40ms的方式。这么做有两个后果：
 * （1）SDL弹出的窗口无法移动，一直显示是忙碌状态
 * （2）画面显示并不是严格的40ms一帧，因为还没有考虑解码的时间。
 * SU（SDL Update）版在视频解码的过程中，不再使用延时40ms的方式，而是创建了
 * 一个线程，每隔40ms发送一个自定义的消息，告知主函数进行解码显示。这样做之后：
 * （1）SDL弹出的窗口可以移动了
 * （2）画面显示是严格的40ms一帧
 * Remark:
 * Standard Version use's SDL_Delay() to control video's frame rate, it has 2
 * disadvantages:
 * (1)SDL's Screen can't be moved and always "Busy".
 * (2)Frame rate can't be accurate because it doesn't consider the time consumed
 * by avcodec_decode_video2()
 * SU（SDL Update）Version solved 2 problems above. It create a thread to send SDL
 * Event every 40ms to tell the main loop to decode and show video frames.
 */

#include <stdio.h>
#include <iostream>

using namespace std;

#ifdef _WIN32
//Windows
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
#include "SDL2/SDL.h"
};
#else
//Linux...
#ifdef __cplusplus
extern "C"
{
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/imgutils.h>
#include <SDL2/SDL.h>
#ifdef __cplusplus
};
#endif
#endif

//Refresh Event
#define SFM_REFRESH_EVENT  (SDL_USEREVENT + 1)

#define SFM_BREAK_EVENT  (SDL_USEREVENT + 2)

int thread_exit = 0;
int thread_pause = 0;

int sfp_refresh_thread1(void *opaque) {
    thread_exit = 0;
    thread_pause = 0;

    while (!thread_exit) {
        if (!thread_pause) {
            SDL_Event event;
//            SDL_USEREVENT
            event.type = SFM_REFRESH_EVENT;
//            printf("event.type %d\n",event.type);
            SDL_PushEvent(&event);
        }
        SDL_Delay(40);
    }
    thread_exit = 0;
    thread_pause = 0;
    //Break
    SDL_Event event;
    event.type = SFM_BREAK_EVENT;
    SDL_PushEvent(&event);

    return 0;
}


//Buffer:
//|-----------|-------------|
//chunk-------pos---len-----|
static Uint8 *audio_chunk;
static Uint32 audio_len;
static Uint8 *audio_pos;

/* The audio function callback takes the following parameters:
 * stream: A pointer to the audio buffer to be filled
 * len: The length (in bytes) of the audio buffer
*/
void fill_audio(void *udata, Uint8 *stream, int len) {
    //SDL 2.0
    SDL_memset(stream, 0, len);
    if (audio_len == 0)
        return;

    len = (len > audio_len ? audio_len : len);    /*  Mix  as  much  data  as  possible  */

    SDL_MixAudio(stream, audio_pos, len, SDL_MIX_MAXVOLUME);
    audio_pos += len;
    audio_len -= len;
}

#define MAX_AUDIO_FRAME_SIZE 192000

int main(int argc, char *argv[]) {
    AVFormatContext *pFormatCtx;
    int i, videoindex;
    AVCodecContext *pCodecCtx;
    AVCodec *pCodec;
    AVFrame *pFrame, *pFrameYUV;
    unsigned char *out_buffer;

    AVPacket *packet;
    int ret, got_picture;

    //------------SDL----------------
    int screen_w, screen_h;
    SDL_Window *screen;
    SDL_Renderer *sdlRenderer;
    SDL_Texture *sdlTexture;
    SDL_Rect sdlRect;
    SDL_Thread *video_tid;
    SDL_Event event;

    struct SwsContext *img_convert_ctx;


    //char filepath[]="bigbuckbunny_480x272.h265";
    char filepath[] = "屌丝男士.mov";

    av_register_all();
    avformat_network_init();
    pFormatCtx = avformat_alloc_context();

    if (avformat_open_input(&pFormatCtx, filepath, NULL, NULL) != 0) {
        printf("Couldn't open input stream.\n");
        return -1;
    }
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        printf("Couldn't find stream information.\n");
        return -1;
    }
    videoindex = -1;
    for (i = 0; i < pFormatCtx->nb_streams; i++) {
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoindex = i;//videoindex ==0
            break;
        }
    }
    cout << "videoindex " << videoindex << "\n";//0
    int audioindex = 1;
    if (videoindex == -1) {
        printf("Didn't find a video stream.\n");
        return -1;
    }
    pCodecCtx = pFormatCtx->streams[videoindex]->codec;
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if (pCodec == NULL) {
        printf("Codec not found.\n");
        return -1;
    }
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        printf("Could not open codec.\n");
        return -1;
    }
    pFrame = av_frame_alloc();
    pFrameYUV = av_frame_alloc();

    out_buffer = (unsigned char *) av_malloc(
            av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1));
    av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, out_buffer,
                         AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);

    //Output Info-----------------------------
    //printf("---------------- File Information ---------------\n");
    av_dump_format(pFormatCtx, 0, filepath, 0);

    //printf("-------------------------------------------------\n");

    img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
                                     pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL,
                                     NULL);


    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
        printf("Could not initialize SDL - %s\n", SDL_GetError());
        return -1;
    }
    //SDL 2.0 Support for multiple windows
    screen_w = pCodecCtx->width;
    screen_h = pCodecCtx->height;
    screen = SDL_CreateWindow("Simplest ffmpeg player's Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              screen_w, screen_h, SDL_WINDOW_OPENGL);

    if (!screen) {
        printf("SDL: could not create window - exiting:%s\n", SDL_GetError());
        return -1;
    }
    sdlRenderer = SDL_CreateRenderer(screen, -1, 0);
    //IYUV: Y + U + V  (3 planes)
    //YV12: Y + V + U  (3 planes)
    sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, pCodecCtx->width,
                                   pCodecCtx->height);

    sdlRect.x = 0;
    sdlRect.y = 0;
    sdlRect.w = screen_w;
    sdlRect.h = screen_h;

    packet = (AVPacket *) av_malloc(sizeof(AVPacket));

    video_tid = SDL_CreateThread(sfp_refresh_thread1, NULL, NULL);
    //------------SDL End------------


    //print video info

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
    AVStream *pVideoStream = pFormatCtx->streams[videoindex];
//    printf("w * h %d*%d\n", pVideoStream->codecpar->width, pVideoStream->codecpar->height);
//    printf("w * h 2 %d*%d\n", pCodecCtx->width, pCodecCtx->height);

    //AVCodecParameters
    AVCodecParameters *pVideoParameters = pVideoStream->codecpar;
    printf("w * h %d*%d\n", pVideoParameters->width, pVideoParameters->height);

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

    //--------------- audio -------------------------------
    //AVCodecParameters *pAudio = pFormatCtx->streams[audioindex]->codecpar;
    AVCodecContext *pAudioCodecCtx = pFormatCtx->streams[audioindex]->codec;
    AVCodec *pAudioCodec = avcodec_find_decoder(pAudioCodecCtx->codec_id);
    cout << "audio name " << *pAudioCodec->name << "\n";//a
    cout << "audio long name " << *pAudioCodec->long_name << "\n";//A
    // Open codec
    if (avcodec_open2(pAudioCodecCtx, pAudioCodec, nullptr) < 0) {
        printf("Could not open codec.\n");
        return -1;
    }

    AVPacket *pAudioPacket = (AVPacket *) av_malloc(sizeof(AVPacket));
    av_init_packet(pAudioPacket);
    uint8_t *audio_out_buffer;
    audio_out_buffer = (uint8_t *) av_malloc(MAX_AUDIO_FRAME_SIZE * 2);
    //Out Audio Param
    uint64_t out_channel_layout = AV_CH_LAYOUT_STEREO;
    //nb_samples: AAC-1024 MP3-1152
    int out_nb_samples = pAudioCodecCtx->frame_size;
    AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
    int out_sample_rate = 44100;
    int out_channels = av_get_channel_layout_nb_channels(out_channel_layout);
    //Out Buffer Size
    int out_buffer_size = av_samples_get_buffer_size(nullptr, out_channels, out_nb_samples, out_sample_fmt, 1);

    //SDL_AudioSpec
    SDL_AudioSpec wanted_spec;
    wanted_spec.freq = out_sample_rate;
    wanted_spec.format = AUDIO_S16SYS;
    wanted_spec.channels = out_channels;
    wanted_spec.silence = 0;
    wanted_spec.samples = out_nb_samples;
    wanted_spec.callback = fill_audio;
    wanted_spec.userdata = pAudioCodecCtx;

    if (SDL_OpenAudio(&wanted_spec, NULL) < 0) {
        printf("can't open audio.\n");
        return -1;
    }
    int index = 0;
    unsigned char *out_audio_buffer;
    out_audio_buffer = (uint8_t *) av_malloc(MAX_AUDIO_FRAME_SIZE * 2);
    AVFrame *pAudioFrame;
    pAudioFrame = av_frame_alloc();

    //FIX:Some Codec's Context Information is missing
    int64_t in_channel_layout = av_get_default_channel_layout(pAudioCodecCtx->channels);
    //Swr
    struct SwrContext *au_convert_ctx;
    au_convert_ctx = swr_alloc();
    au_convert_ctx = swr_alloc_set_opts(au_convert_ctx, out_channel_layout, out_sample_fmt, out_sample_rate,
                                        in_channel_layout, pAudioCodecCtx->sample_fmt, pAudioCodecCtx->sample_rate, 0,
                                        nullptr);
    swr_init(au_convert_ctx);

    //Play
    SDL_PauseAudio(0);


    //Event Loop
    for (;;) {
        //Wait
        SDL_WaitEvent(&event);
        if (event.type == SFM_REFRESH_EVENT) {
            while (1) {
                if (av_read_frame(pFormatCtx, packet) < 0) {//如果是视频读取完了
                    thread_exit = 1;
                }
                if (packet->stream_index == videoindex)
                    break;
            }
            ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);//got_picture==1
            /*while (av_read_frame(pFormatCtx, pAudioPacket) >= 0) {
                if (pAudioPacket->stream_index == audioindex) {
                    ret = avcodec_decode_audio4(pAudioCodecCtx, pAudioFrame, &got_picture, pAudioPacket);
                    if (ret < 0) {
                        printf("Error in decoding audio frame.\n");
                        return -1;
                    }
                    if (got_picture > 0) {
                        swr_convert(au_convert_ctx, &audio_out_buffer, MAX_AUDIO_FRAME_SIZE,
                                    (const uint8_t **) pAudioFrame->data,
                                    pAudioFrame->nb_samples);

                        index++;
                    }


                    while (audio_len > 0)//Wait until finish
                        SDL_Delay(1);

                    //Set audio buffer (PCM data)
                    audio_chunk = (Uint8 *) audio_out_buffer;
                    //Audio buffer length
                    audio_len = out_buffer_size;
                    audio_pos = audio_chunk;


                }
                av_free_packet(pAudioPacket);
            }*/



            if (ret < 0) {
                printf("Decode Error.\n");
                return -1;
            }
            if (got_picture) {
                sws_scale(img_convert_ctx, (const unsigned char *const *) pFrame->data, pFrame->linesize, 0,
                          pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);
                //SDL---------------------------
                SDL_UpdateTexture(sdlTexture, nullptr, pFrameYUV->data[0], pFrameYUV->linesize[0]);
                SDL_RenderClear(sdlRenderer);
                //SDL_RenderCopy( sdlRenderer, sdlTexture, &sdlRect, &sdlRect );
                SDL_RenderCopy(sdlRenderer, sdlTexture, nullptr, nullptr);
                SDL_RenderPresent(sdlRenderer);
                //SDL End-----------------------
            }
            av_free_packet(packet);
        } else if (event.type == SDL_KEYDOWN) {
            //Pause
            if (event.key.keysym.sym == SDLK_SPACE)
                thread_pause = !thread_pause;
        } else if (event.type == SDL_QUIT) {
            thread_exit = 1;
        } else if (event.type == SFM_BREAK_EVENT) {
            break;
        }

    }

    sws_freeContext(img_convert_ctx);
    SDL_CloseAudio();//Close SDL
    SDL_Quit();
    //--------------
    av_frame_free(&pFrameYUV);
    av_frame_free(&pFrame);
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);
    swr_free(&au_convert_ctx);


    av_free(audio_out_buffer);
    avcodec_close(pAudioCodecCtx);
    return 0;
}

