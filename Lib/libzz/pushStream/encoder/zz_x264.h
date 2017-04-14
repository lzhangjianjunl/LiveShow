

/*
 h264软编码器：x264封装。
 */

#ifndef zz_x264_h
#define zz_x264_h

#include <stdio.h>
#include "x264.h"
#include "x264_config.h"
#include "zz_data.h"

typedef struct zz_x264_config{
    //宽高
    int width;
    int height;
    
    //帧率，1秒多少帧
    int fps;
    
    //码率
    int bitrate;
    
    //b帧数量
    int b_frame_count;
    
    //X264_CSP_NV12 || X264_CSP_I420
    int input_data_format;
    
}zz_x264_config;

extern zz_x264_config *alloc_zz_x264_config();
extern void free_zz_x264_config(zz_x264_config **);

typedef struct zz_x264_context{
    //配置文件
    zz_x264_config config;
    
    //x264 handler
    x264_t *x264_handler;
    
    //编码过程变量
    //输入
    x264_picture_t *pic_in;
    //输出
    x264_picture_t *pic_out;
    
    //保存编码数据
    x264_nal_t *nal;
    int nal_count;
    
    zz_data *sps_pps_data;
    
    //保存的每一帧的数据
    zz_data *encoded_h264_data;
}zz_x264_context;

extern zz_x264_context *alloc_zz_x264_context(zz_x264_config);
extern void free_zz_x264_context(zz_x264_context **);

//编码一帧数据
extern void zz_encode_yuv_frame_2_x264(zz_x264_context *zz_ctx, int8_t *yuv_frame, int len);

//构造spspps
extern zz_data *zz_create_sps_pps_data(uint8_t *sps_bytes, uint32_t sps_len, uint8_t *pps_bytes, uint32_t pps_len);

#endif /* zz_yuv2h264_h */
