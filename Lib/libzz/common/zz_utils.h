

/*
 utils log等便利函数
 */

#ifndef zz_utils_h
#define zz_utils_h

#include <stdio.h>
#include <string.h>
#include "zz_alloc.h"

#define zzLog(...)  \
do{ \
printf(__VA_ARGS__); \
printf("\n");\
}while(0)

#define zz_log(...) zzLog(__VA_ARGS__)

//视频编码加速，stride须设置为16的倍数
#define zz_stride(wid) ((wid % 16 != 0) ? ((wid) + 16 - (wid) % 16): (wid))

#endif /* zz_utils_h */
