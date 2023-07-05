#include <jni.h>
#include <string>
#include <stdio.h>
#include <android/log.h>

//#include <libavcodec/avcodec.h>
#ifdef __cplusplus
extern "C" {
#endif
#include "ffinclude/libavformat/avformat.h"
#include "ffinclude/libavcodec/avcodec.h"
#include "ffinclude/libavutil/avutil.h"
#include "ffinclude/libavutil/imgutils.h"
#include "ffinclude/libswscale/swscale.h"

#ifdef __cplusplus
}
#endif



#define LOGI(TAG, FORMAT, ...) __android_log_print(ANDROID_LOG_INFO, TAG, FORMAT, ##__VA_ARGS__)
#define LOGE(TAG, FORMAT, ...) __android_log_print(ANDROID_LOG_ERROR, TAG, FORMAT, ##__VA_ARGS__)

#define LOGTAG "FFMPEG-LOG";

extern "C" JNIEXPORT jstring JNICALL
Java_com_zz_example_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    //const char* ffConfig = avcodec_configuration();
    LOGE("retret","bbbbbbbbbbbbbbb");
    const char* ffConfig = av_version_info();
    return env->NewStringUTF(ffConfig);
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_zz_example_MainActivity_startPlayJNI(JNIEnv *env, jobject thiz) {

    jclass cls = env->GetObjectClass(thiz);
    jmethodID callbackMethodID = env->GetMethodID(cls, "ffmpegYuv420Call", "(II[B)V");

    std::string success = "success";
    std::string error = "error";

    avformat_network_init();
    LOGI("FFMPEG-LOG","avformat_network_init");
    const char* url = "rtsp://10.52.8.106:554/stream1";

    AVFormatContext* pFormatCtx = avformat_alloc_context();
    int ret = 0;
    ret = avformat_open_input(&pFormatCtx, url, 0, NULL);
    if (ret != 0) {
        LOGE("FFMPEG-LOG","avformat_open_input error");
    }else{
        LOGE("FFMPEG-LOG","avformat_open_input success");
    }

    ret = avformat_find_stream_info(pFormatCtx, nullptr);
    if (ret < 0) {
        LOGE("FFMPEG-LOG", "fail to get stream information: %d\n", ret);
    }

    int video_stream_index = -1;
    int audio_stream_index = -1;
    fprintf(stdout, "Number of elements in AVFormatContext.streams: %d\n", pFormatCtx->nb_streams);
    for (int i = 0; i < pFormatCtx->nb_streams; ++i) {
        const AVStream* stream = pFormatCtx->streams[i];
        fprintf(stdout, "type of the encoded data: %d\n", stream->codecpar->codec_id);
        if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = i;
            LOGI("FFMPEG-LOG", "dimensions of the video frame in pixels: width: %d, height: %d, pixel format: %d\n",
                    stream->codecpar->width, stream->codecpar->height, stream->codecpar->format);
        }else if (stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audio_stream_index = i;
            LOGI("FFMPEG-LOG", "audio sample format: %d\n", stream->codecpar->format);
        }
    }

    AVCodecContext* pCodecCtx = avcodec_alloc_context3(NULL);
    if (pCodecCtx == NULL){
        LOGE("FFMPEG-LOG", "Could not allocate AVCodecContext\n");
    }
    avcodec_parameters_to_context(pCodecCtx, pFormatCtx->streams[video_stream_index]->codecpar);
    //根据编解码上下文中的编码id查找对应的解码
    const AVCodec* pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if (pCodec == NULL){
        LOGE("FFMPEG-LOG", "找不到解码器");

    }
    //打开解码器
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0){
        LOGE("FFMPEG-LOG", "解码器无法打开");
    }
    //输出视频信息
    LOGI("FFMPEG-LOG", "视频的pix_fmt：%d\n", pCodecCtx->pix_fmt);//AV_PIX_FMT_YUV420P
    LOGI("FFMPEG-LOG", "视频的宽高：%d,%d\n", pCodecCtx->width, pCodecCtx->height);
    LOGI("FFMPEG-LOG", "视频解码器的名称：%s\n", pCodec->name);

    AVPacket* pPacket = av_packet_alloc();// (AVPacket*)av_malloc(sizeof(AVPacket));
    AVFrame* pFrame = av_frame_alloc();

    SwsContext* pSwsCtx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, 192, 108, AV_PIX_FMT_YUV420P, SWS_BICUBIC, nullptr, nullptr, nullptr);
    if (nullptr == pSwsCtx) {
        LOGE("FFMPEG-LOG", "get sws context failed");
    }
    AVFrame* pFrameYuv = av_frame_alloc();
    av_image_alloc(pFrameYuv->data, pFrameYuv->linesize, 192, 108, AV_PIX_FMT_YUV420P, 1);

    while (1) {
        ret = av_read_frame(pFormatCtx, pPacket);
        if (ret < 0) {
            LOGI("FFMPEG-LOG", "error or end of file: %d\n", ret);
            continue;
        }
        if (pPacket->stream_index == audio_stream_index) {
            //LOGI("FFMPEG-LOG", "audio stream, packet size: %d\n", pPacket->size);
            continue;
        }
        if (pPacket->stream_index == video_stream_index) {
            //LOGI("FFMPEG-LOG", "video stream, packet size: %d\n", pPacket->size);
            ret = avcodec_send_packet(pCodecCtx, pPacket);
            av_packet_unref(pPacket);
            if (ret < 0) {
                LOGI("FFMPEG-LOG", "%s", "解码完成");
            }
            ret = avcodec_receive_frame(pCodecCtx, pFrame);
            if (ret != 0) {
                char errbuf[128];
                const char *errbuf_ptr = errbuf;
                av_strerror(ret, errbuf, sizeof(errbuf));
                LOGE("FFMPEG-LOG", "avcodec_receive_frame failed %s", errbuf);
                continue;
            }
            LOGI("FFMPEG-LOG", "avcodec_receive_frame SUCCESS linesize %d,%d,%d", pFrame->linesize[0], pFrame->linesize[1], pFrame->linesize[2]);

            ret = sws_scale(pSwsCtx, pFrame->data, pFrame->linesize, 0, pFrame->height, pFrameYuv->data, pFrameYuv->linesize);
            if (ret < 0) {
                LOGE("FFMPEG-LOG","sws_scale implement failed");
            }
            LOGI("FFMPEG-LOG", "sws_scale SUCCESS linesize %d,%d,%d", pFrameYuv->linesize[0], pFrameYuv->linesize[1], pFrameYuv->linesize[2]);


            jsize wy = 192*108;
            int dataSize = wy+wy/2;
            char data[dataSize];
            LOGE("FFMPEG-LOG", "data[dataSize] %d,%d", dataSize, sizeof(data));
            memset(data, 0, dataSize);


            memcpy(data, pFrameYuv->data[0], wy);
            for(int i=0;i<(wy/4);i++){
                *(data+wy+2*i)=*(pFrameYuv->data[2]+i);
                *(data+wy+1+2*i)=*(pFrameYuv->data[1]+i);
            }

            jbyteArray jdataArray = env->NewByteArray(dataSize);

            //问题2 dataSize 1080*1920会出问题 暂时先设108*192
            env->SetByteArrayRegion(jdataArray, 0, dataSize, (jbyte*)data);
            env->CallVoidMethod(thiz, callbackMethodID, 192, 108, jdataArray);
        }
    }

    return env->NewStringUTF(success.c_str());
}