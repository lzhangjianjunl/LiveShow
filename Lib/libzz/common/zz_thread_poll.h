

/*
 封装了便于使用的pthread线程池，使用时直接向线程池丢任务即可。
 */

#ifndef zz_thread_h
#define zz_thread_h

#include <stdio.h>
#include <pthread/pthread.h>

typedef struct zz_thread_poll zz_thread_poll;
typedef void (*zz_thread_func)(void *);
typedef void (*zz_thread_finish_cb)();

//创建线程池
extern zz_thread_poll *alloc_zz_thread_poll(int limit, int stored_task_list_limit);
//释放线程池
extern void free_zz_thread_poll(zz_thread_poll **poll_p, zz_thread_finish_cb finish_cb);
//添加任务
extern void zz_add_task_to_thread_poll(zz_thread_poll *poll, zz_thread_func func, void *param);

//累积的未发送的thread poll数量
extern int zz_stored_task_count_in_thread_poll(zz_thread_poll *poll);

extern void test_thread();

#endif /* zz_thread_h */
