

/*
 libzz是我自己开发时建立的一个方便使用的c语言函数库，包括如下功能：
    数组(存取任意数据)，
    字典(dict，map)，
    数据块(二进制数据读写存储)，
    内存分配（可跟踪），
    文件操作，
    线程池，
    librtmp封装
 包含了libzz中的所有接口，外部使用只需要包含本文件就可以了。
 */

#ifndef zz_all_h
#define zz_all_h

#include <stdio.h>
#include "zz_common.h"
#include "zz_x264.h"
#include "zz_faac.h"
#include "zz_encode_flv.h"
#include "zz_streamer.h"
#include "zz_sw_faac_encoder.h"
#include "zz_sw_x264_encoder.h"
#include "zz_utils.h"

//rtmp状态回调
extern void aw_rtmp_state_changed_cb_in_oc(zz_rtmp_state old_state, zz_rtmp_state new_state);

#endif /* zz_all_h */
