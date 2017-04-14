

/*
 aac软编码器：faac封装。
 */

#ifndef zz_faac_h
#define zz_faac_h

#include <stdio.h>
#include "faac.h"
#include "faaccfg.h"
#include "zz_data.h"

typedef struct zz_faac_config {
    //采样率
    int sample_rate;
    
    //单个样本大小
    int sample_size;
    
    //比特率
    int bitrate;
    
    //声道
    int channel_count;
} zz_faac_config;

extern zz_faac_config *alloc_zz_faac_config();
extern void free_zz_faac_config(zz_faac_config **);

typedef struct zz_faac_context {
    zz_faac_config config;
    
    //编码器句柄
    faacEncHandle *faac_handler;
    
    //最大输入样本数
    unsigned long max_input_sample_count;
    unsigned long max_input_byte_count;
    
    //最大输出字节数
    unsigned long max_output_byte_count;
    
    //缓冲区
    int8_t *aac_buffer;
    
    zz_data *audio_specific_config_data;
    
    //保存的每一帧的数据
    zz_data *encoded_aac_data;
} zz_faac_context;

extern zz_faac_context * alloc_zz_faac_context(zz_faac_config);
extern void free_zz_faac_context(zz_faac_context **);

extern void zz_encode_pcm_frame_2_aac(zz_faac_context *ctx, int8_t *pcm_data, long len);

#endif /* zz_pcm2aac_h */
