

#ifndef zz_sw_x264_encoder_h
#define zz_sw_x264_encoder_h

/*
 使用x264进行软编码：h264软编码器
 */

#include "zz_x264.h"
#include "zz_encode_flv.h"

//将采集到的video yuv数据，编码为flv video tag
extern zz_flv_video_tag * zz_sw_encoder_encode_x264_data(int8_t *yuv_data, long len, uint32_t timeStamp);

//根据flv/h264/aac协议创建video/audio首帧tag
extern zz_flv_video_tag *zz_sw_encoder_create_x264_sps_pps_tag();

//开关编码器
extern void zz_sw_encoder_open_x264_encoder(zz_x264_config *x264_config);
extern void zz_sw_encoder_close_x264_encoder();

extern int8_t zz_sw_x264_encoder_is_valid();

//创建 flv tag 跟编码无关。
//将h264数据转为flv_video_tag
extern zz_flv_video_tag *zz_encoder_create_video_tag(int8_t *h264_data, long len, uint32_t timeStamp, long composition_time, int8_t is_key_frame);
//创建sps_pps_tag
extern zz_flv_video_tag *zz_encoder_create_sps_pps_tag(zz_data *sps_pps_data);

#endif /* zz_sw_video_encoder_h */
