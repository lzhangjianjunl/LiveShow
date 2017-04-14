

/*
 提供了librtmp的连接、断开、发送数据等操作，并提供完整的事件处理及回调，包括断线重连等机制
 */

#ifndef zz_rtmp_h
#define zz_rtmp_h

#include "rtmp.h"
#include "zz_alloc.h"
#include <string.h>

typedef enum zz_rtmp_state{
    zz_rtmp_state_idle,//默认情况
    zz_rtmp_state_connecting,//连接中
    zz_rtmp_state_connected,//连接成功
    zz_rtmp_state_opened,//已打开，可以streaming了
    zz_rtmp_state_closed,//关闭，发送后回到idle状态
    zz_rtmp_state_error_write,//写入失败，发送完毕回到open状态
    zz_rtmp_state_error_open,//打开失败，发送后回到idle
    zz_rtmp_state_error_net,//多次连接失败，网络错误
}zz_rtmp_state;

extern const char *zz_rtmp_state_description(zz_rtmp_state rtmp_state);

typedef void (*zz_rtmp_state_changed_cb) (zz_rtmp_state old_state, zz_rtmp_state new_state);

typedef struct zz_rtmp_context{
    char rtmp_url[256];
    RTMP *rtmp;
    
    //写入错误断线重连
    int8_t write_error_retry_time_limit;//写入错误重试次数，超过这个次数就要重连。
    int8_t write_error_retry_curr_time;//当前重试错误次数。
    
    //open错误重连
    int8_t open_error_retry_time_limit;//连接重试次数，超过这个次数。
    int8_t open_error_retry_curr_time;//当前连接重试的次数，表示连接错误。
    
    int8_t is_header_sent;//是不是发送过flv的header了
    
    //当前mp4文件开始时间戳
    double current_time_stamp;
    double last_time_stamp;
    
    //总时间
    double total_duration;
    
    //状态变化回调，注意，不要在状态回调中做释放zz_rtmp_context的操作。
    //如果非要释放，请延迟一帧。
    zz_rtmp_state_changed_cb state_changed_cb;
    //当前状态
    zz_rtmp_state rtmp_state;
} zz_rtmp_context;

extern void zz_init_rtmp_context(zz_rtmp_context *ctx, const char *rtmp_url, zz_rtmp_state_changed_cb state_changed_cb);
extern zz_rtmp_context *alloc_zz_rtmp_context(const char *rtmp_url, zz_rtmp_state_changed_cb state_changed_cb);
extern void free_zz_rtmp_context(zz_rtmp_context **ctx);

//rtmp是否打开
extern int8_t zz_is_rtmp_opened(zz_rtmp_context *ctx);

//打开rtmp
extern int zz_rtmp_open(zz_rtmp_context *ctx);

//写入数据
extern int zz_rtmp_write(zz_rtmp_context *ctx, const char *buf, int size);

//关闭rtmp
extern int zz_rtmp_close(zz_rtmp_context *ctx);

//获取当前时间
extern uint32_t zz_rtmp_time();

#endif /* zz_rtmp_h */
