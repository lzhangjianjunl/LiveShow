

/*
 flv文件编码，此文件描述了一个完整flv文件的编码过程，只要有正确的h264和aac数据，就能够使用此文件函数合成正确的flv文件。
 */

#ifndef zz_encode_flv_h
#define zz_encode_flv_h

#include <stdio.h>
#include "zz_data.h"

//flv tag type
typedef enum zz_flv_tag_type {
    zz_flv_tag_type_audio = 8,
    zz_flv_tag_type_video = 9,
    zz_flv_tag_type_script = 18,
} zz_flv_tag_type;

//共6种(CodecID)，这里只用h264
typedef enum zz_flv_v_codec_id{
    zz_flv_v_codec_id_H263 = 2,
    zz_flv_v_codec_id_H264 = 7,
} zz_flv_v_codec_id;

//共13种(即sound format)，这里只用aac
typedef enum zz_flv_a_codec_id{
    zz_flv_a_codec_id_MP3 = 2,
    zz_flv_a_codec_id_AAC = 10,
} zz_flv_a_codec_id;

//sound size 8bit 16bit
typedef enum zz_flv_a_sound_size{
    zz_flv_a_sound_size_8_bit = 0,
    zz_flv_a_sound_size_16_bit = 1,
} zz_flv_a_sound_size;

//sound rate 5.5 11 22 44 kHz
typedef enum zz_flv_a_sound_rate{
    zz_flv_a_sound_rate_5_5kHZ = 0,
    zz_flv_a_sound_rate_11kHZ = 1,
    zz_flv_a_sound_rate_22kHZ = 2,
    zz_flv_a_sound_rate_44kHZ = 3,
} zz_flv_a_sound_rate;

//sound type mono/stereo
typedef enum zz_flv_a_sound_type{
    zz_flv_a_sound_type_mono = 0,
    zz_flv_a_sound_type_stereo = 1,
} zz_flv_a_sound_type;

//共5种
typedef enum zz_flv_v_frame_type{
    zz_flv_v_frame_type_key = 1,//关键帧
    zz_flv_v_frame_type_inner = 2,//非关键帧
}zz_flv_v_frame_type;

//h264 packet type
typedef enum zz_flv_v_h264_packet_type{
    zz_flv_v_h264_packet_type_seq_header = 0,
    zz_flv_v_h264_packet_type_nalu = 1,
    zz_flv_v_h264_packet_type_end_of_seq = 2,
}zz_flv_v_h264_packet_type;

typedef enum zz_flv_a_aac_packge_type{
    zz_flv_a_aac_package_type_aac_sequence_header = 0,
    zz_flv_a_aac_package_type_aac_rzz = 1,
}zz_flv_a_aac_packge_type;

struct zz_flv_audio_tag;
struct zz_flv_video_tag;
struct zz_flv_script_tag;

// flv tags
typedef struct zz_flv_common_tag{
    zz_flv_tag_type tag_type;//tag类型，1字节
    //数据长度，3字节，
    // total tag size值为 sample_size + [script/audio/video]_tag_header_size + tag_header_size
    // tag header size = 11
    // [script/audio/video]_tag_header_size 会变化
    // tag body size = sample_size + [script/audio/video]_tag_header_size
    uint32_t data_size;//tag 总长度，包含11字节的 common_tag_header_size，包含自己的tag_header_size + tag_body_size;
    uint32_t header_size;//自己的tag header的长度
    uint32_t timestamp;//时间戳，3字节
    uint8_t timestamp_extend;//时间戳扩展位，1字节
    uint32_t stream_id;//总是0，3字节
    
    union{
        struct zz_flv_audio_tag *audio_tag;
        struct zz_flv_video_tag *video_tag;
        struct zz_flv_script_tag *script_tag;
    };
} zz_flv_common_tag;

typedef struct zz_flv_script_tag{
    zz_flv_common_tag common_tag;
    double duration;
    double width;
    double height;
    double video_data_rate;
    double frame_rate;
    double v_frame_rate;
    double a_frame_rate;
    double v_codec_id;
    double a_sample_rate;
    double a_sample_size;
    uint8_t stereo;
    double a_codec_id;
    double file_size;
} zz_flv_script_tag;

extern zz_flv_script_tag *alloc_zz_flv_script_tag();
extern void free_zz_flv_script_tag(zz_flv_script_tag **);

typedef struct zz_flv_audio_tag{
    zz_flv_common_tag common_tag;
    zz_flv_a_codec_id sound_format;//声音格式，4bit(0.5字节)
    zz_flv_a_sound_rate sound_rate;//声音频率，2bit
    zz_flv_a_sound_size sound_size;//声音尺寸, 1bit
    zz_flv_a_sound_type sound_type;//声音类型, 1bit
    zz_flv_a_aac_packge_type aac_packet_type;//aac 包类型，1字节，如果sound format==10(AAC)会有这个字段，否则没有。
    
    //aac sqeuence header，包含了aac转流所需的必备内容，需要作为第一个audio tag发送。
    zz_data *config_record_data;//audio config data
    
    zz_data *frame_data;//audio frame data
} zz_flv_audio_tag;

extern zz_flv_audio_tag *alloc_zz_flv_audio_tag();
extern void free_zz_flv_audio_tag(zz_flv_audio_tag **);

typedef struct zz_flv_video_tag{
    zz_flv_common_tag common_tag;
    zz_flv_v_frame_type frame_type;//帧类型，是否关键帧，4bit(0.5字节)
    zz_flv_v_codec_id codec_id;//编码器id，4bit
    
    //h264才有
    zz_flv_v_h264_packet_type h264_package_type;//h264包类型，1字节
    uint32_t h264_composition_time;//h264 时间调整(即cts，pts = dts + cts), 3字节
    
    //avc sequence header, 包含h264需要的重要信息(sps，pps)，需要在第一个videotag时发送
    zz_data *config_record_data;//video config data
    
    zz_data *frame_data;//video frame data
} zz_flv_video_tag;

extern zz_flv_video_tag *alloc_zz_flv_video_tag();
extern void free_zz_flv_video_tag(zz_flv_video_tag **);

//构造flv的方法
//写入header
extern void zz_write_flv_header(zz_data **flv_data);
//写入flv tag
extern void zz_write_flv_tag(zz_data **flv_data, zz_flv_common_tag *common_tag);

#endif /* zz_encode_flv_h */
