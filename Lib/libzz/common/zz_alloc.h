

/*
 对 malloc 和 free 进行封装，可以跟踪内存分配情况，方便调试
 */

#ifndef zz_alloc_h
#define zz_alloc_h

#include <stdio.h>

#if defined(__FILE__) && defined(__LINE__)
#define zz_alloc(size) zz_alloc_detail((size), __FILE__, __LINE__)
#else
#define zz_alloc(size) zz_alloc_detail((size), "", 0)
#endif

//可以监视内存的分配和释放，便于调试内存泄漏

//自定义 alloc
extern void * zz_alloc_detail(size_t size, const char *file_name, uint32_t line);

//自定义 free
extern void zz_free(void *);

//开始debug alloc，调用此函数开始才记录分配和释放数量
extern void zz_init_debug_alloc();

//停止debug alloc，调用此函数后，不再记录分配释放。所有记录清0
extern void zz_uninit_debug_alloc();

//返回总共alloc的size
extern size_t zz_total_alloc_size();

//返回总共free的size
extern size_t zz_total_free_size();

//打印内存alloc/free/leak状况
extern void zz_print_alloc_description();

#endif /* zz_alloc_h */
