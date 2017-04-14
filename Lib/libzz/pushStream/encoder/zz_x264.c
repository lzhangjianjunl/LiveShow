

#include "zz_x264.h"
#include "zz_alloc.h"
#include "zz_utils.h"
#include <string.h>

static void zz_create_x264_param(zz_x264_context *zz_ctx, x264_param_t ** param){
    if (!param) {
        zz_log("[E] zz_create_h264_param param error");
        return;
    }
    x264_param_t *x264_param = *param;
    if (!x264_param) {
        x264_param = zz_alloc(sizeof(x264_param_t));
        *param = x264_param;
    }
    
    x264_param_default(x264_param);
    x264_param_default_preset(x264_param, "fast" , "zerolatency" );
    
    //视频属性
    x264_param->i_width = zz_ctx->config.width;
    x264_param->i_height = zz_ctx->config.height;
    x264_param->i_frame_total = 0;//编码总帧数
    x264_param->i_keyint_max = 0;//关键帧最大间隔，不用b帧。否则音视频不对齐。还需要对音频做延时处理。
    
    x264_param->i_log_level = X264_LOG_NONE;
    
    //比特率 帧率
    x264_param->rc.i_rc_method = X264_RC_CQP;//CQP(恒定质量)，CRF(恒定码率)，ABR(平均码率)
    x264_param->rc.i_bitrate = (uint32_t)(zz_ctx->config.bitrate / 1000);//1000kbs
    x264_param->rc.i_vbv_max_bitrate = (uint32_t)((zz_ctx->config.bitrate * 1.2) / 1000);
    x264_param->rc.i_vbv_buffer_size = x264_param->rc.i_vbv_max_bitrate;
    x264_param->i_fps_den = 1;
    x264_param->i_fps_num = zz_ctx->config.fps;
    x264_param->i_timebase_den = x264_param->i_fps_num;
    x264_param->i_timebase_num = x264_param->i_fps_den;
}

static void zz_open_x264_handler(zz_x264_context *zz_ctx, x264_param_t *x264_param){
    x264_t *x264_handler = NULL;
    
    int i = 0;
    for (; i < 7; i++) {
        zz_log("x264_profile_names[%d] = %s", i, x264_profile_names[i]);
    }
    
    x264_param_apply_profile(x264_param, x264_profile_names[1]);
    
    x264_handler = x264_encoder_open(x264_param);
    zz_ctx->x264_handler = x264_handler;
    
    if (!x264_handler) {
        zz_log("[E] x264_handler open error");
        return;
    }
}

//保存本次编码的数据
static void zz_x264_save_encoded_data(zz_x264_context *ctx){
    reset_zz_data(&ctx->encoded_h264_data);
    if (ctx->nal_count > 0) {
        int i = 0;
        for (; i < ctx->nal_count; i++) {
            data_writer.write_bytes(&ctx->encoded_h264_data, ctx->nal[i].p_payload, ctx->nal[i].i_payload);
        }
    }
}

//构造spspps
extern zz_data *zz_create_sps_pps_data(uint8_t *sps_bytes, uint32_t sps_len, uint8_t *pps_bytes, uint32_t pps_len){
    //构造AVCDecoderConfigurationRecord
    //前面6字节固定
    zz_data *avc_decoder_record = alloc_zz_data(0);
    int8_t adr_pre[6] = {0x01, 0x42, 0x00, 0x1e, 0xff, 0xe1};
    data_writer.write_bytes(&avc_decoder_record, (uint8_t *)adr_pre, 6);
    //sps长度
    data_writer.write_uint16(&avc_decoder_record, sps_len);
    data_writer.write_bytes(&avc_decoder_record, (uint8_t *)sps_bytes, sps_len);
    //pps 个数 1
    data_writer.write_uint8(&avc_decoder_record, 1);
    data_writer.write_uint16(&avc_decoder_record, pps_len);
    data_writer.write_bytes(&avc_decoder_record, (uint8_t *)pps_bytes, pps_len);
    return avc_decoder_record;
}

//编码header，sps pps
static void zz_encode_x264_header(zz_x264_context *zz_ctx){
    x264_encoder_headers(zz_ctx->x264_handler, &zz_ctx->nal, &zz_ctx->nal_count);
    
    //保存sps pps data
    uint8_t *sps_bytes = NULL;
    int8_t sps_len = 0;
    uint8_t *pps_bytes = NULL;
    int8_t pps_len = 0;
    int i = 0;
    for (; i < zz_ctx->nal_count; i++) {
        if (zz_ctx->nal[i].i_type == NAL_SPS) {
            sps_bytes = (uint8_t *)zz_ctx->nal[i].p_payload + 4;
            sps_len = zz_ctx->nal[i].i_payload - 4;
        }else if(zz_ctx->nal[i].i_type == NAL_PPS){
            pps_bytes = (uint8_t *)zz_ctx->nal[i].p_payload + 4;
            pps_len = zz_ctx->nal[i].i_payload - 4;
        }
    }
    
    zz_data *avc_decoder_record = zz_create_sps_pps_data(sps_bytes, sps_len, pps_bytes, pps_len);
    memcpy_zz_data(&zz_ctx->sps_pps_data, avc_decoder_record->data, avc_decoder_record->size);
    free_zz_data(&avc_decoder_record);
}

//编码一帧数据
extern void zz_encode_yuv_frame_2_x264(zz_x264_context *zz_ctx, int8_t *yuv_frame, int len){
    if (len > 0 && yuv_frame) {
        int actual_width = zz_stride(zz_ctx->config.width);
        //数据保存到pic_in中
        if (zz_ctx->config.input_data_format == X264_CSP_NV12) {
            zz_ctx->pic_in->img.plane[0] = (uint8_t *)yuv_frame;
            zz_ctx->pic_in->img.plane[1] = (uint8_t *)yuv_frame + actual_width * zz_ctx->config.height;
        }else if(zz_ctx->config.input_data_format == X264_CSP_BGR || zz_ctx->config.input_data_format == X264_CSP_RGB){
            zz_ctx->pic_in->img.plane[0] = (uint8_t *)yuv_frame;
        }else if(zz_ctx->config.input_data_format == X264_CSP_BGRA){
            zz_ctx->pic_in->img.plane[0] = (uint8_t *)yuv_frame;
        }else{//YUV420
            zz_ctx->pic_in->img.plane[0] = (uint8_t *)yuv_frame;
            zz_ctx->pic_in->img.plane[1] = (uint8_t *)yuv_frame + actual_width * zz_ctx->config.height;
            zz_ctx->pic_in->img.plane[2] = (uint8_t *)yuv_frame + actual_width * zz_ctx->config.height * 5 / 4;
        }
        //编码
        x264_encoder_encode(zz_ctx->x264_handler, &zz_ctx->nal, &zz_ctx->nal_count, zz_ctx->pic_in, zz_ctx->pic_out);
        zz_ctx->pic_in->i_pts++;
    }
    
    //保存本帧数据
    zz_x264_save_encoded_data(zz_ctx);
}

extern zz_x264_config *alloc_zz_x264_config(){
    zz_x264_config *config = zz_alloc(sizeof(zz_x264_config));
    memset(config, 0, sizeof(zz_x264_config));
    return config;
}

extern void free_zz_x264_config(zz_x264_config **config_p){
    zz_x264_config *config = *config_p;
    if (config) {
        zz_free(config);
        *config_p = NULL;
    }
}

extern zz_x264_context *alloc_zz_x264_context(zz_x264_config config){
    zz_x264_context *ctx = zz_alloc(sizeof(zz_x264_context));
    memset(ctx, 0, sizeof(zz_x264_context));
    
    if (!config.input_data_format) {
        config.input_data_format = X264_CSP_I420;
    }
    
    //创建handler do nothing
    memcpy(&ctx->config, &config, sizeof(zz_x264_config));
    x264_param_t *x264_param = NULL;
    zz_create_x264_param(ctx, &x264_param);
    zz_open_x264_handler(ctx, x264_param);
    zz_free(x264_param);
    
    //创建pic_in
    x264_picture_t *pic_in = zz_alloc(sizeof(x264_picture_t));
    x264_picture_init(pic_in);
    
    int alloc_width = zz_stride(config.width);
    
    x264_picture_alloc(pic_in, config.input_data_format, alloc_width, config.height);
    
    pic_in->img.i_csp = config.input_data_format;
    
    if (config.input_data_format == X264_CSP_NV12) {
        pic_in->img.i_stride[0] = alloc_width;
        pic_in->img.i_stride[1] = alloc_width;
        pic_in->img.i_plane = 2;
    }else if(config.input_data_format == X264_CSP_BGR || config.input_data_format == X264_CSP_RGB){
        pic_in->img.i_stride[0] = alloc_width * 3;
        pic_in->img.i_plane = 1;
    }else if(config.input_data_format == X264_CSP_BGRA){
        pic_in->img.i_stride[0] = alloc_width * 4;
        pic_in->img.i_plane = 1;
    }else{//YUV420
        pic_in->img.i_stride[0] = alloc_width;
        pic_in->img.i_stride[1] = alloc_width / 2;
        pic_in->img.i_stride[2] = alloc_width / 2;
        pic_in->img.i_plane = 3;
    }
    
    ctx->pic_in = pic_in;
    
    ctx->pic_out = zz_alloc(sizeof(x264_picture_t));
    x264_picture_init(ctx->pic_out);
    
    //创建zz_data
    ctx->encoded_h264_data = alloc_zz_data(0);
    ctx->sps_pps_data = alloc_zz_data(0);
    
    //获取sps pps
    zz_encode_x264_header(ctx);
    
    return ctx;
}

extern void free_zz_x264_context(zz_x264_context **ctx_p){
    zz_x264_context *ctx = *ctx_p;
    if (ctx) {
        //释放pic_in
        if (ctx->pic_in) {
            x264_picture_clean(ctx->pic_in);
            zz_free(ctx->pic_in);
            ctx->pic_in = NULL;
        }
        
        if (ctx->pic_out) {
            zz_free(ctx->pic_out);
            ctx->pic_out = NULL;
        }
        
        //释放zz_data
        free_zz_data(&ctx->encoded_h264_data);
        
        if (ctx->sps_pps_data) {
            free_zz_data(&ctx->sps_pps_data);
        }
        
        //关闭handler
        if (ctx->x264_handler) {
            x264_encoder_close(ctx->x264_handler);
            ctx->x264_handler = NULL;
        }
        
        zz_free(ctx);
        *ctx_p = NULL;
    }
}

