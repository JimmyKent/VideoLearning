//
// Created by 金国充 on 23/03/2018.
// 把视频解码成yuv格式储存
//
#include <iostream>


using namespace std;

//#include <stdio.h>

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
};

#include "StringUtils.h"

int decodeVideo(char **pName);

int decodeVideo(char **pName) {

    char *temp = *pName;
    //char *temp = *pName;
    //ffmpeg

    AVFormatContext *pFormatCtx = nullptr;
    AVCodecContext *pVideoCodecCtx = nullptr;
    AVCodec *pVideoCodec = nullptr;
    AVPacket *pVideoPacket = nullptr;

    int videoIndex = -1;

    int width = 0, height = 0;

    av_register_all();

    if (avformat_open_input(&pFormatCtx, temp, nullptr, nullptr) != 0) {
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
    pVideoCodecCtx = avcodec_alloc_context3(pVideoCodec);
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


    pVideoPacket = (AVPacket *) av_malloc(sizeof(AVPacket));


    AVFrame *pFrameOri = nullptr, *pFrameYUV = nullptr;
    pFrameOri = av_frame_alloc();
    pFrameYUV = av_frame_alloc();

    //初始化pFrameYUV
    pFrameYUV->width = width;
    pFrameYUV->height = height;
    pFrameYUV->format = AV_PIX_FMT_YUV420P;


    unsigned char *out_buffer = nullptr;
    out_buffer = (unsigned char *) av_malloc(
            (size_t) av_image_get_buffer_size(AV_PIX_FMT_YUV420P, width, height, 1));

    av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, out_buffer,
                         AV_PIX_FMT_YUV420P, width, height, 1);

    SwsContext *sws_ctx = sws_getContext(width, height, pVideoCodecCtx->pix_fmt, width, height,
                                         AV_PIX_FMT_YUV420P, SWS_BILINEAR, nullptr, nullptr, nullptr);

    int len = (int) strlen(temp) + 4;

    char yuvName[len];
    yuvName[len] = {'\0'};
    char *pYuvName = yuvName;
    char **ppYuvName = &pYuvName;
    getYuvName(pName, ppYuvName);

    printf("yuvName %s \n", yuvName);

    FILE *fpYUV = fopen(yuvName, "wb+");
    if (!fpYUV) {
        printf("文件打开失败！\n");
        return -1;
    }
    int size_temp = height * width;


    while (av_read_frame(pFormatCtx, pVideoPacket) >= 0) {
        if (pVideoPacket->stream_index == videoIndex) {
            if (avcodec_send_packet(pVideoCodecCtx, pVideoPacket) >= 0) {
                if (avcodec_receive_frame(pVideoCodecCtx, pFrameOri) >= 0) {

                    //XXX pFrameOri 这样一帧的数据是大的, 要切割
                    sws_scale(sws_ctx, (uint8_t const *const *) pFrameOri->data,
                              pFrameOri->linesize,
                              0, height, pFrameYUV->data, pFrameYUV->linesize);

//                    AVPictureType type = pFrameOri->pict_type;
//                    switch (type) {
//                        case AV_PICTURE_TYPE_I:
//                            printf("帧类型 I");
//                            break;
//                        case AV_PICTURE_TYPE_P:
//                            printf("帧类型 P");
//                            break;
//                        case AV_PICTURE_TYPE_B:
//                            printf("帧类型 B");
//                            break;
//                        default:
//                            break;
//                    }

                    fwrite(pFrameYUV->data[0], 1, size_temp, fpYUV);
                    fwrite(pFrameYUV->data[1], 1, size_temp / 4, fpYUV);
                    fwrite(pFrameYUV->data[2], 1, size_temp / 4, fpYUV);

                }


            }
        }
        av_packet_unref(pVideoPacket);
    }


    fclose(fpYUV);

    sws_freeContext(sws_ctx);
    //--------------
    av_packet_free(&pVideoPacket);
    av_frame_free(&pFrameOri);
    av_frame_free(&pFrameYUV);
    avcodec_close(pVideoCodecCtx);
    avformat_close_input(&pFormatCtx);


    return 0;

}