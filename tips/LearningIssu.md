
学习过程遇到问题  
导入SDL2问题  
SDL2中的include目录是这样的  
/usr/local/Cellar/sdl2/2.0.7/include/SDL2/*.h  

而ffmpeg的目录是这样的  
/usr/local/Cellar/ffmpeg/3.4.1/include/libavcodec/*.h  

cmake会把lib开头的目录去掉lib作为库引入  
而不会把SDL2作为库引入  

所以在include SDL2的时候应该这么写,  
\#include\<SDL2/SDL.h>

https://stackoverflow.com/questions/41988983/using-sdl2-with-clion-on-os-x  
用 find_library 代替 find_package  


CLion中使用CMake导入第三方库的方法
http://blog.csdn.net/Haoran823/article/details/71657602

访问文件
fopen null
文件应该放在 cmake-build-debug 里面


--#if vs. if
条件编译是C语言中预处理部分的内容，它是编译器编译代码时最先处理的部分，

条件编译里面有判断语句，比如 #if 、#else 、#elif 及 #endif

它的意思是如果宏条件符合，编译器就编译这段代码，否则，编译器就忽略这段代码而不编译


## --Renderer 渲染器

--fseek
fseek(fp, 0, SEEK_SET);
If the stream is open in binary mode, the new position is exactly offset bytes measured from
the beginning of the file if origin is SEEK_SET,
from the current file position if origin is SEEK_CUR,
and from the end of the file if origin is SEEK_END. Binary streams are not required to support SEEK_END,
in particular if additional null bytes are output.


## --Texture 纹理


## --渐隐区  

先scale
```
while (av_read_frame(pFormatCtx, pVideoPacket) >= 0) {
        if (pVideoPacket->stream_index == videoIndex) {
            if (avcodec_send_packet(pVideoCodecCtx, pVideoPacket) >= 0) {
                if (avcodec_receive_frame(pVideoCodecCtx, pFrameOri) >= 0) {
                    //XXX pFrameOri 这样一帧的数据是大的, 要切割


                    //显示不对
//                    int sliceHeight = sws_scale(sws_ctx, (uint8_t const *const *) pFrameOri->data,
//                                                 pFrameOri->linesize,
//                                                0, height, pFrameYUV->data, pFrameYUV->linesize);

                    //enQueue(pFrameYUV);
                    AVFrame *p = av_frame_alloc();

                    int ret = av_frame_ref(p, pFrameOri);
                    if (ret < 0)
                        break;

                    queue1.push(p);
                }


            }
        }
        av_packet_unref(pVideoPacket);
    }

    while (!queue1.empty()) {

        auto displayFrame = queue1.front();
        queue1.pop();
 int sliceHeight = sws_scale(sws_ctx, (uint8_t const *const *) displayFrame->data, displayFrame->linesize,
                                                0, height, pFrameYUV->data, pFrameYUV->linesize);
        //SDL---------------------------
        //SDL_UpdateTexture(sdlTexture, nullptr, displayFrame->data[0], displayFrame->linesize[0]);
        SDL_UpdateTexture(sdlTexture, nullptr, pFrameYUV->data[0], pFrameYUV->linesize[0]);
        SDL_RenderClear(sdlRenderer);
        SDL_RenderCopy(sdlRenderer, sdlTexture, &sdlRect, &sdlRect);
        //SDL_RenderCopy(sdlRenderer, sdlTexture, nullptr, nullptr);
        SDL_RenderPresent(sdlRenderer);
        //SDL End-----------------------

        SDL_Delay(20);

    }
```



```
pFormatContext->nb_streams : 2  
videoIndex : 0  
pVideoCodecCtx->width,pVideoCodecCtx->height : 640, 352  
pFrameOri->pict_type 1  
pFrameOri->pict_type 3  
pFrameOri->pict_type 3  
pFrameOri->pict_type 2  
pFrameOri->pict_type 3  
pFrameOri->pict_type 3  
pFrameOri->pict_type 2  
pFrameOri->pict_type 3  
pFrameOri->pict_type 3  
 
pFrameOri->pict_type 1  
pFrameOri->pict_type 3  
pFrameOri->pict_type 2  
pFrameOri->pict_type 3  
pFrameOri->pict_type 3  
pFrameOri->pict_type 2  
pFrameOri->pict_type 3  
pFrameOri->pict_type 3  
pFrameOri->pict_type 2  
pFrameOri->pict_type 3  
pFrameOri->pict_type 3  
pFrameOri->pict_type 2  

pFrameOri->pict_type 1  
pFrameOri->pict_type 3  
pFrameOri->pict_type 3  
pFrameOri->pict_type 3  
pFrameOri->pict_type 2  
pFrameOri->pict_type 3  
pFrameOri->pict_type 3  
pFrameOri->pict_type 3  
```
转场动画的时候, 会重新出现关键帧,整个画面全覆盖,这样计算量应该是最小的






