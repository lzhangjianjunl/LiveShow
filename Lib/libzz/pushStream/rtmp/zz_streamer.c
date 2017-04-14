

#include "zz_streamer.h"
#include "zz_utils.h"
#include "zz_rtmp.h"
#include "zz_encode_flv.h"
#include <unistd.h>
#include <inttypes.h>
#include "zz_thread_poll.h"
#include "zz_array.h"

//open stream 使用的变量
static zz_rtmp_context *s_rtmp_ctx = NULL;
static zz_data *s_output_buf = NULL;
static zz_rtmp_state_changed_cb s_state_changed_cb = NULL;
static char *s_rtmp_url = NULL;

//rtmp
static int8_t zz_streamer_is_rtmp_valid();
static void zz_streamer_send_flv_tag_to_rtmp(zz_flv_common_tag *common_tag);
static void zz_streamer_rtmp_state_changed_callback(zz_rtmp_state old_state, zz_rtmp_state new_state);

//rtmp
static int8_t zz_steamer_open_rtmp_context();
static void zz_streamer_close_rtmp_context();

//video-----

extern void zz_streamer_send_video_data(zz_flv_video_tag *video_tag){
    if (!zz_streamer_is_rtmp_valid()) {
        zz_log("[E] zz_streamer_send_video_data s_rtmp_ctx is NULL");
        return;
    }
    
    //    zz_log("[D] zz_streamer_send_video_data timestamp=%u, compTime=%u", video_tag->common_tag.timestamp, video_tag->h264_composition_time);
    
    zz_streamer_send_flv_tag_to_rtmp(&video_tag->common_tag);
}

extern void zz_streamer_send_video_sps_pps_tag(zz_flv_video_tag *sps_pps_tag){
    if (!zz_streamer_is_rtmp_valid()) {
        zz_log("[E] zz_streamer_send_video_sps_pps_tag when rtmp is not valid");
        return;
    }
    
    if (s_rtmp_ctx->is_header_sent) {
        return;
    }
    
    //发送 video sps pps
    zz_streamer_send_flv_tag_to_rtmp(&sps_pps_tag->common_tag);
}

//audio------

extern void zz_streamer_send_audio_specific_config_tag(zz_flv_audio_tag *asc_tag){
    if (!zz_streamer_is_rtmp_valid()) {
        zz_log("[E] zz_streamer_send_audio_specific_config_tag when rtmp is not valid");
        return;
    }
    
    if (s_rtmp_ctx->is_header_sent) {
        return;
    }
    
    //发送 audio specific config
    zz_streamer_send_flv_tag_to_rtmp(&asc_tag->common_tag);
}

extern void zz_streamer_send_audio_data(zz_flv_audio_tag *audio_tag){
    if (!zz_streamer_is_rtmp_valid()) {
        zz_log("[E] zz_streamer_send_audio_specific_config_tag when rtmp is not valid");
        return;
    }
    
    zz_streamer_send_flv_tag_to_rtmp(&audio_tag->common_tag);
}

//rtmp------

static void zz_streamer_send_flv_tag_to_rtmp(zz_flv_common_tag *common_tag){
    if (!zz_streamer_is_streaming()) {
        return;
    }
    if (common_tag) {
        zz_write_flv_tag(&s_output_buf, common_tag);
        switch (common_tag->tag_type) {
            case zz_flv_tag_type_audio: {
                free_zz_flv_audio_tag(&common_tag->audio_tag);
                break;
            }
            case zz_flv_tag_type_video: {
                free_zz_flv_video_tag(&common_tag->video_tag);
                break;
            }
            case zz_flv_tag_type_script: {
                free_zz_flv_script_tag(&common_tag->script_tag);
                break;
            }
        }
    }
    
    if (s_output_buf->size <= 0) {
        return;
    }
    
    //    int nRet =
    zz_rtmp_write(s_rtmp_ctx, (const char *)s_output_buf->data, s_output_buf->size);
    
    //    zz_log("[d] send flv tag size=%d sended_size=%d", s_output_buf->size, nRet);
    
    reset_zz_data(&s_output_buf);
}

static int8_t zz_streamer_is_rtmp_valid(){
    return s_rtmp_ctx != NULL;
}

extern int8_t zz_streamer_is_streaming(){
    //    return zz_streamer_is_rtmp_valid() && zz_streamer_is_video_valid() && zz_streamer_is_audio_valid() && zz_is_rtmp_opened(s_rtmp_ctx);
    return zz_streamer_is_rtmp_valid() && zz_is_rtmp_opened(s_rtmp_ctx);
}

static void zz_streamer_rtmp_state_changed_callback(zz_rtmp_state old_state, zz_rtmp_state new_state){
    if(new_state == zz_rtmp_state_connected){
        //打开rtmp 先发送 配置tag
        //        zz_streamer_send_video_sps_pps_tag();
        //        zz_streamer_send_audio_specific_config_tag();
    }else if(new_state == zz_rtmp_state_error_open){
        zz_streamer_close_rtmp_context();
    }
    
    if (s_state_changed_cb) {
        s_state_changed_cb(old_state, new_state);
    }
}

static int8_t zz_steamer_open_rtmp_context(){
    if (!s_rtmp_ctx) {
        s_rtmp_ctx = alloc_zz_rtmp_context(s_rtmp_url, zz_streamer_rtmp_state_changed_callback);
    }
    return zz_rtmp_open(s_rtmp_ctx);
}

static void zz_streamer_close_rtmp_context(){
    if (s_rtmp_ctx) {
        zz_rtmp_close(s_rtmp_ctx);
    }
    zz_log("[d] closed rtmp context");
}

//创建 outbuf
static void zz_streamer_create_out_buf(){
    if (s_output_buf) {
        zz_log("[E] zz_streamer_open_encoder s_out_buf is already exist");
        return;
    }
    s_output_buf = alloc_zz_data(0);
}

//释放 outbuf
static void zz_streamer_release_out_buf(){
    if (!s_output_buf) {
        zz_log("[E] zz_streamer_open_encoder s_out_buf is already free");
        return;
    }
    free_zz_data(&s_output_buf);
}

extern int8_t zz_streamer_open(const char *rtmp_url, zz_rtmp_state_changed_cb state_changed_cb){
    if (zz_streamer_is_rtmp_valid()) {
        zz_log("[E] zz_streamer_open_rtmp s_rtmp_ctx is already open");
        return -1;
    }
    
    // 调试 mem leak
    //    zz_uninit_debug_alloc();
    //    zz_init_debug_alloc();
    
    //创建outbuf
    zz_streamer_create_out_buf();
    
    s_state_changed_cb = state_changed_cb;
    
    int32_t rtmp_url_len = (int32_t)strlen(rtmp_url);
    if (!s_rtmp_url) {
        s_rtmp_url = zz_alloc(rtmp_url_len + 1);
    }
    memcpy(s_rtmp_url, rtmp_url, rtmp_url_len);
    
    return zz_steamer_open_rtmp_context();
}

extern void zz_streamer_close(){
    if (!zz_streamer_is_rtmp_valid()) {
        zz_log("[E] zz_streamer_close s_rtmp_ctx is NULL");
        return;
    }
    
    //关闭rtmp
    zz_streamer_close_rtmp_context();
    
    //释放 s_rtmp_ctx;
    free_zz_rtmp_context(&s_rtmp_ctx);
    
    s_state_changed_cb = NULL;
    
    if (s_rtmp_url) {
        zz_free(s_rtmp_url);
        s_rtmp_url = NULL;
    }
    
    //释放outbuf
    zz_streamer_release_out_buf();
    
    //调试mem leak
    //    zz_print_alloc_description();
}
