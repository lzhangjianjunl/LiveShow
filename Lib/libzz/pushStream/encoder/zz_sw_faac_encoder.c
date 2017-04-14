

#include "zz_sw_faac_encoder.h"
#include "zz_all.h"

//faac 实例
static zz_faac_context *s_faac_ctx = NULL;
static zz_faac_config *s_faac_config = NULL;

//debug使用
static int32_t audio_count = 0;

//创建基本的audio tag，除类型，数据和时间戳
static zz_flv_audio_tag *zz_sw_encoder_create_flv_audio_tag(zz_faac_config *faac_cfg){
    zz_flv_audio_tag *audio_tag = alloc_zz_flv_audio_tag();
    audio_tag->sound_format = zz_flv_a_codec_id_AAC;
    audio_tag->common_tag.header_size = 2;
    
    if (faac_cfg->sample_rate == 22050) {
        audio_tag->sound_rate = zz_flv_a_sound_rate_22kHZ;
    }else if (faac_cfg->sample_rate == 11025) {
        audio_tag->sound_rate = zz_flv_a_sound_rate_11kHZ;
    }else if (faac_cfg->sample_rate == 5500) {
        audio_tag->sound_rate = zz_flv_a_sound_rate_5_5kHZ;
    }else{
        audio_tag->sound_rate = zz_flv_a_sound_rate_44kHZ;
    }
    
    if (faac_cfg->sample_size == 8) {
        audio_tag->sound_size = zz_flv_a_sound_size_8_bit;
    }else{
        audio_tag->sound_size = zz_flv_a_sound_size_16_bit;
    }
    
    audio_tag->sound_type = faac_cfg->channel_count == 1 ? zz_flv_a_sound_type_mono : zz_flv_a_sound_type_stereo;
    return audio_tag;
}

//编码器开关
extern void zz_sw_encoder_open_faac_encoder(zz_faac_config *faac_config){
    if (zz_sw_faac_encoder_is_valid()) {
        zz_log("[E] zz_sw_encoder_open_faac_encoder when encoder is already inited");
        return;
    }
    
    int32_t faac_cfg_len = sizeof(zz_faac_config);
    if (!s_faac_config) {
        s_faac_config = zz_alloc(faac_cfg_len);
    }
    memcpy(s_faac_config, faac_config, faac_cfg_len);
    
    s_faac_ctx = alloc_zz_faac_context(*faac_config);
}

//关闭编码器并释放资源
extern void zz_sw_encoder_close_faac_encoder(){
    if (!zz_sw_faac_encoder_is_valid()) {
        zz_log("[E] zz_sw_encoder_close_faac_encoder when encoder is not inited");
        return;
    }
    
    free_zz_faac_context(&s_faac_ctx);
    
    if (s_faac_config) {
        zz_free(s_faac_config);
        s_faac_config = NULL;
    }
}

//对pcm数据进行faac软编码，并转成flv_audio_tag
extern zz_flv_audio_tag *zz_sw_encoder_encode_faac_data(int8_t *pcm_data, long len, uint32_t timestamp){
    if (!zz_sw_faac_encoder_is_valid()) {
        zz_log("[E] zz_sw_encoder_encode_faac_data when encoder is not inited");
        return NULL;
    }
    
    zz_encode_pcm_frame_2_aac(s_faac_ctx, pcm_data, len);
    
    int adts_header_size = 7;
    
    //除去ADTS头的7字节
    if (s_faac_ctx->encoded_aac_data->size <= adts_header_size) {
        return NULL;
    }
    
    //除去ADTS头的7字节
    zz_flv_audio_tag *audio_tag = zz_encoder_create_audio_tag((int8_t *)s_faac_ctx->encoded_aac_data->data + adts_header_size, s_faac_ctx->encoded_aac_data->size - adts_header_size, timestamp, &s_faac_ctx->config);
    
    audio_count++;
    
    return audio_tag;
}

//根据faac_config 创建包含audio specific config 的flv tag
extern zz_flv_audio_tag *zz_sw_encoder_create_faac_specific_config_tag(){
    if(!zz_sw_faac_encoder_is_valid()){
        zz_log("[E] zz_sw_encoder_create_faac_specific_config_tag when audio encoder is not inited");
        return NULL;
    }
    
    //创建 audio specfic config record
    zz_flv_audio_tag *aac_tag = zz_sw_encoder_create_flv_audio_tag(&s_faac_ctx->config);
    aac_tag->aac_packet_type = zz_flv_a_aac_package_type_aac_sequence_header;
    
    aac_tag->config_record_data = copy_zz_data(s_faac_ctx->audio_specific_config_data);
    aac_tag->common_tag.timestamp = 0;
    aac_tag->common_tag.data_size = s_faac_ctx->audio_specific_config_data->size + 11 + aac_tag->common_tag.header_size;
    
    return aac_tag;
}

extern uint32_t zz_sw_faac_encoder_max_input_sample_count(){
    if (zz_sw_faac_encoder_is_valid()) {
        return (uint32_t)s_faac_ctx->max_input_sample_count;
    }
    return 0;
}

//编码器是否合法
extern int8_t zz_sw_faac_encoder_is_valid(){
    return s_faac_ctx != NULL;
}

//下面2个函数所有编码器都可以用
//将aac数据转为flv_audio_tag
extern zz_flv_audio_tag *zz_encoder_create_audio_tag(int8_t *aac_data, long len, uint32_t timeStamp, zz_faac_config *faac_cfg){
    zz_flv_audio_tag *audio_tag = zz_sw_encoder_create_flv_audio_tag(faac_cfg);
    audio_tag->aac_packet_type = zz_flv_a_aac_package_type_aac_rzz;
    
    audio_tag->common_tag.timestamp = timeStamp;
    zz_data *frame_data = alloc_zz_data((uint32_t)len);
    memcpy(frame_data->data, aac_data, len);
    frame_data->size = (uint32_t)len;
    audio_tag->frame_data = frame_data;
    audio_tag->common_tag.data_size = audio_tag->frame_data->size + 11 + audio_tag->common_tag.header_size;
    return audio_tag;
}

//创建audio_specific_config_tag
extern zz_flv_audio_tag *zz_encoder_create_audio_specific_config_tag(zz_data *audio_specific_config_data, zz_faac_config *faac_config){
    //创建 audio specfic config record
    zz_flv_audio_tag *audio_tag = zz_sw_encoder_create_flv_audio_tag(faac_config);
    
    audio_tag->config_record_data = copy_zz_data(audio_specific_config_data);
    audio_tag->common_tag.timestamp = 0;
    audio_tag->common_tag.data_size = audio_specific_config_data->size + 11 + audio_tag->common_tag.header_size;
    
    return audio_tag;
}

