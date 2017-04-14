

#include "zz_rtmp.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "zz_utils.h"

extern const char *zz_rtmp_state_description(zz_rtmp_state rtmp_state){
    switch (rtmp_state) {
        case zz_rtmp_state_idle: {
            return "zz_rtmp_state_idle";
        }
        case zz_rtmp_state_connecting: {
            return "zz_rtmp_state_connecting";
        }
        case zz_rtmp_state_connected: {
            return "zz_rtmp_state_connected";
        }
        case zz_rtmp_state_opened: {
            return "zz_rtmp_state_opened";
        }
        case zz_rtmp_state_closed: {
            return "zz_rtmp_state_closed";
        }
        case zz_rtmp_state_error_write: {
            return "zz_rtmp_state_error_write";
        }
        case zz_rtmp_state_error_open: {
            return "zz_rtmp_state_error_open";
        }
        case zz_rtmp_state_error_net: {
            return "zz_rtmp_state_error_net";
        }
    }
}

extern void zz_init_rtmp_context(zz_rtmp_context *ctx, const char *rtmp_url, zz_rtmp_state_changed_cb state_changed_cb){
    ctx->rtmp = NULL;
    ctx->rtmp_state = zz_rtmp_state_idle;
    
    memset(ctx->rtmp_url, 0, 256);
    size_t url_size = strlen(rtmp_url);
    if (url_size <= 0 || url_size >= 256) {
        zzLog("[error] when init rtmp_context rtmp_url is error");
    }else{
        memcpy(ctx->rtmp_url, rtmp_url, url_size);
    }
    
    ctx->write_error_retry_curr_time = 0;
    ctx->write_error_retry_time_limit = 5;
    ctx->open_error_retry_curr_time = 0;
    ctx->open_error_retry_time_limit = 3;
    ctx->state_changed_cb = state_changed_cb;
}

extern zz_rtmp_context *alloc_zz_rtmp_context(const char *rtmp_url, zz_rtmp_state_changed_cb state_changed_cb){
    zz_rtmp_context *ctx = zz_alloc(sizeof(zz_rtmp_context));
    memset(ctx, 0, sizeof(zz_rtmp_context));
    zz_init_rtmp_context(ctx, rtmp_url, state_changed_cb);
    return ctx;
}

extern void free_zz_rtmp_context(zz_rtmp_context **ctx){
    zz_rtmp_context *inner_ctx = *ctx;
    
    if (inner_ctx) {
        if (inner_ctx->rtmp) {
            zz_rtmp_close(inner_ctx);
        }
        zz_free(inner_ctx);
    }
    *ctx = NULL;
}

static void zz_set_rtmp_state(zz_rtmp_context *ctx, zz_rtmp_state new_state);

static void zz_rtmp_state_changed_inner(zz_rtmp_context *ctx, zz_rtmp_state old_state, zz_rtmp_state new_state){
    switch (new_state) {
        case zz_rtmp_state_idle: {
            ctx->write_error_retry_curr_time = 0;
            ctx->is_header_sent = 0;
            
            //总时间
            ctx->total_duration += ctx->current_time_stamp;
            
            //当前流时间戳
            ctx->current_time_stamp = 0;
            break;
        }
        case zz_rtmp_state_opened: {
            ctx->open_error_retry_curr_time = 0;
            break;
        }
        case zz_rtmp_state_connected:{
            zz_set_rtmp_state(ctx, zz_rtmp_state_opened);
            break;
        }
        case zz_rtmp_state_closed: {
            zz_set_rtmp_state(ctx, zz_rtmp_state_idle);
            break;
        }
        case zz_rtmp_state_error_write: {
            zz_set_rtmp_state(ctx, old_state);
            //写入错误次数过多 重连
            if (ctx->write_error_retry_curr_time >= ctx->write_error_retry_time_limit) {
                zz_rtmp_close(ctx);
                zz_rtmp_open(ctx);
            }else{
                ctx->write_error_retry_curr_time++;
            }
            break;
        }
        case zz_rtmp_state_error_open: {
            zz_set_rtmp_state(ctx, zz_rtmp_state_idle);
            //open错误次数过多，认为网络错误
            if (ctx->open_error_retry_curr_time >= ctx->open_error_retry_curr_time) {
                zz_set_rtmp_state(ctx, zz_rtmp_state_error_net);
            }else{
                ctx->open_error_retry_curr_time++;
            }
            break;
        }
        case zz_rtmp_state_error_net:{
            ctx->open_error_retry_curr_time = 0;
            zz_set_rtmp_state(ctx, zz_rtmp_state_idle);
            break;
        }
        case zz_rtmp_state_connecting:
            break;
    }
}

static void zz_set_rtmp_state(zz_rtmp_context *ctx, zz_rtmp_state new_state){
    if (ctx->rtmp_state == new_state) {
        return;
    }
    zz_rtmp_state old_state = ctx->rtmp_state;
    ctx->rtmp_state = new_state;
    
    //回调用户接口
    if (ctx->state_changed_cb) {
        ctx->state_changed_cb(old_state, new_state);
    }
    
    //内部处理
    zz_rtmp_state_changed_inner(ctx, old_state, new_state);
}

int8_t zz_is_rtmp_opened(zz_rtmp_context *ctx){
    return ctx && ctx->rtmp != NULL;
}

int zz_rtmp_open(zz_rtmp_context *ctx){
    if (zz_is_rtmp_opened(ctx) || ctx->rtmp_state != zz_rtmp_state_idle) {
        zzLog("[error] static_zz_rtmp is in use");
        return 0;
    }
    if (strlen(ctx->rtmp_url) <= 0 || strlen(ctx->rtmp_url) > 255) {
        zzLog("[error ] zz rtmp setup url = %s\n", ctx->rtmp_url);
        return 0;
    }
    zz_set_rtmp_state(ctx, zz_rtmp_state_connecting);
    int recode = 0;
    ctx->rtmp = RTMP_Alloc();
    RTMP_Init(ctx->rtmp);
    ctx->rtmp->Link.timeout = 1;
    if (!RTMP_SetupURL(ctx->rtmp, ctx->rtmp_url)) {
        zzLog("[error ] zz rtmp setup url = %s\n", ctx->rtmp_url);
        recode = -2;
        goto FAILED;
    }
    
    RTMP_EnableWrite(ctx->rtmp);
    
    RTMP_SetBufferMS(ctx->rtmp, 3 * 1000);
    
    if (!RTMP_Connect(ctx->rtmp, NULL)) {
        recode = -3;
        goto FAILED;
    }
    
    if (!RTMP_ConnectStream(ctx->rtmp, 0)) {
        recode = -4;
        goto FAILED;
    }
    zz_set_rtmp_state(ctx, zz_rtmp_state_connected);
    return 1;
FAILED:
    zz_rtmp_close(ctx);
    zz_set_rtmp_state(ctx, zz_rtmp_state_error_open);
    return !recode;
}

int zz_rtmp_close(zz_rtmp_context *ctx){
    zzLog("zz rtmp closing.......\n");
    if (zz_is_rtmp_opened(ctx)) {
        signal(SIGPIPE, SIG_IGN);
        RTMP_Close(ctx->rtmp);
        RTMP_Free(ctx->rtmp);
        ctx->rtmp = NULL;
        zzLog("zz rtmp closed.......\n");
        zz_set_rtmp_state(ctx, zz_rtmp_state_closed);
    }
    return 1;
}

int zz_rtmp_write(zz_rtmp_context *ctx, const char *buf, int size){
    if (!zz_is_rtmp_opened(ctx)) {
        zzLog("[error] zz rtmp writing but rtmp is not open");
        return 0;
    }
    signal(SIGPIPE, SIG_IGN);
    int write_ret = RTMP_Write(ctx->rtmp, buf, size);
    if (write_ret <= 0) {
        zz_set_rtmp_state(ctx, zz_rtmp_state_error_write);
    }
    return write_ret;
}

uint32_t zz_rtmp_time(){
    return RTMP_GetTime();
}
