

#include "zz_thread_poll.h"
#include "zz_alloc.h"
#include "zz_utils.h"
#include "zz_array.h"
#include <unistd.h>

typedef struct zz_thread_poll_task{
    zz_thread_func func;
    void *param;
}zz_thread_poll_task;

typedef struct zz_thread_poll_element{
    int index;
    pthread_t *thread;
    pthread_mutex_t *mutex;
    pthread_cond_t *cond;
    zz_thread_poll *poll;
    
    int8_t is_finish;
    
    //传入
    zz_thread_poll_task task;
} zz_thread_poll_element;

static zz_thread_poll_element * alloc_zz_thread_poll_element(zz_thread_poll *poll, int index){
    zz_thread_poll_element *ele = zz_alloc(sizeof(zz_thread_poll_element));
    memset(ele, 0, sizeof(zz_thread_poll_element));
    
    ele->thread = zz_alloc(sizeof(pthread_t));
    
    ele->mutex = zz_alloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(ele->mutex, NULL);
    
    ele->cond = zz_alloc(sizeof(pthread_cond_t));
    pthread_cond_init(ele->cond, NULL);
    
    ele->poll = poll;
    
    ele->index = index;
    
    return ele;
}

static void free_zz_thread_poll_element(zz_thread_poll_element ** ele_p){
    zz_thread_poll_element *ele = *ele_p;
    if (ele) {
        if (ele->thread) {
            zz_free(ele->thread);
        }
        
        if (ele->mutex) {
            zz_free(ele->mutex);
        }
        
        if (ele->cond) {
            zz_free(ele->cond);
        }
        
        zz_free(ele);
    }
    
    *ele_p = NULL;
}

//线程池
typedef struct zz_thread_poll {
    uint32_t limit;//几个线程
    zz_thread_poll_element **elements;
    uint8_t is_running;
    
    //任务列表
    zz_array *task_list;
    uint32_t task_list_limit;
    
    pthread_mutex_t *task_list_mutex;
    
    //结束回调
    zz_thread_finish_cb finish_cb;
}zz_thread_poll;

static void s_inner_thread_poll_running_function(zz_thread_poll_element *th_ele){
    //    zz_log("[d] thread %d is start..\n", th_ele->index);
    while (th_ele && th_ele->poll && th_ele->poll->is_running) {
        pthread_mutex_lock(th_ele->mutex);
        if (th_ele->task.func) {
            //            zz_log("[d] thread %d will run task..\n", th_ele->index);
            th_ele->task.func(th_ele->task.param);
            th_ele->task.func = NULL;
            th_ele->task.param = NULL;
            //            zz_log("[d] thread %d finish run task..\n", th_ele->index);
        }else{
            pthread_mutex_lock(th_ele->poll->task_list_mutex);
            zz_array *task_list = th_ele->poll->task_list;
            if (task_list->count > 0) {
                zz_thread_poll_task *task = zz_array_element_at_index(task_list, 0)->pointer_value;
                zz_array_remove_element_at_index(task_list, 0);
                th_ele->task.func = task->func;
                th_ele->task.param = task->param;
                zz_free(task);
                //                zz_log("[d]continue-- %d task list count=%ld", th_ele->index, task_list->count);
                pthread_mutex_unlock(th_ele->poll->task_list_mutex);
                pthread_mutex_unlock(th_ele->mutex);
                continue;
            }
            pthread_mutex_unlock(th_ele->poll->task_list_mutex);
            
            //            zz_log("[d] thread %d will waiting..\n", th_ele->index);
            pthread_cond_wait(th_ele->cond, th_ele->mutex);
            //            zz_log("[d] thread %d is waked up..\n", th_ele->index);
        }
        pthread_mutex_unlock(th_ele->mutex);
    }
    
    pthread_mutex_lock(th_ele->mutex);
    th_ele->is_finish = 1;
    pthread_mutex_unlock(th_ele->mutex);
    
    //    zz_log("[d] thread %d is finished..\n", th_ele->index);
}

extern zz_thread_poll *alloc_zz_thread_poll(int limit, int stored_task_list_limit){
    zz_thread_poll *th_poll = zz_alloc(sizeof(zz_thread_poll));
    memset(th_poll, 0, sizeof(zz_thread_poll));
    
    th_poll->limit = limit;
    
    th_poll->task_list_limit = stored_task_list_limit;
    
    if (!th_poll->task_list_limit) {
        th_poll->task_list_limit = 100;
    }
    
    //下标
    th_poll->elements = zz_alloc(sizeof(zz_thread_poll_element*) * limit);
    
    //任务列表
    th_poll->task_list = alloc_zz_array(0);
    //列表锁
    th_poll->task_list_mutex = zz_alloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(th_poll->task_list_mutex, NULL);
    
    th_poll->is_running = 1;
    
    for (int i = 0; i < limit; i++) {
        zz_thread_poll_element *element = alloc_zz_thread_poll_element(th_poll, i);
        th_poll->elements[i] = element;
        //线程
        pthread_t *pthread = element->thread;
        //调度策略
        pthread_attr_t pth_attr;
        pthread_attr_init(&pth_attr);
        pthread_attr_setschedpolicy(&pth_attr, SCHED_OTHER);
        pthread_create(pthread, &pth_attr, (void *)s_inner_thread_poll_running_function, element);
    }
    
    return th_poll;
}

static void zz_free_thread_poll_inner(zz_thread_poll *poll){
    if (poll) {
        if (poll->elements) {
            int i = 0;
            for (; i < poll->limit; i++) {
                zz_thread_poll_element *ele = poll->elements[i];
                if (!ele->is_finish) {
                    pthread_cond_signal(ele->cond);
                    pthread_join(*ele->thread, NULL);
                }
                free_zz_thread_poll_element(&ele);
            }
            
            zz_free(poll->elements);
            poll->elements = NULL;
        }
        
        if (poll->task_list) {
            free_zz_array(&poll->task_list);
        }
        
        if (poll->task_list_mutex) {
            zz_free(poll->task_list_mutex);
            poll->task_list_mutex = NULL;
        }
        
        zz_thread_finish_cb finish_cb = poll->finish_cb;
        
        zz_free(poll);
        
        if (finish_cb) {
            finish_cb();
        }
    }
    
    //    zz_log("[d]thread_poll is finished step 2\n");
}

extern void free_zz_thread_poll(zz_thread_poll **poll_p, zz_thread_finish_cb finish_cb){
    zz_thread_poll *poll = *poll_p;
    if (poll) {
        poll->is_running = 0;
        poll->finish_cb = finish_cb;
        pthread_t finish_th;
        pthread_create(&finish_th, NULL, (void *)zz_free_thread_poll_inner, poll);
    }else{
        if (finish_cb) {
            finish_cb();
        }
    }
    //    zz_log("[d]thread_poll is finished step 1\n");
}

extern void zz_add_task_to_thread_poll(zz_thread_poll *poll, zz_thread_func func, void *param){
    if (poll->is_running) {
        zz_thread_poll_element *avaliable_th = NULL;
        int i = 0;
        for (; i < poll->limit; i++) {
            zz_thread_poll_element *th_ele = poll->elements[i];
            if (!th_ele->task.func) {
                avaliable_th = th_ele;
                break;
            }
        }
        if (avaliable_th) {
            avaliable_th->task.func = func;
            avaliable_th->task.param = param;
            pthread_cond_signal(avaliable_th->cond);
        }else{
            zz_thread_poll_task *task = zz_alloc(sizeof(zz_thread_poll_task));
            task->func = func;
            task->param = param;
            pthread_mutex_lock(poll->task_list_mutex);
            if (poll->task_list->count >= poll->task_list_limit) {
                zz_thread_poll_task *remove_task = zz_array_element_at_index(poll->task_list, 0)->pointer_value;
                zz_array_remove_element_at_index(poll->task_list, 0);
                zz_free(remove_task);
                zz_log("[w] ---- thread poll stored tasks too much, so remove the first one task");
                zz_log("[w] ---- you can set task_list_limit with a larger value, current task_list_limit=%d\n", poll->task_list_limit);
            }
            zz_array_add_pointer(&poll->task_list, task);
            pthread_mutex_unlock(poll->task_list_mutex);
        }
    }
}

extern int zz_stored_task_count_in_thread_poll(zz_thread_poll *poll){
    if (poll && poll->task_list) {
        return (int)poll->task_list->count;
    }
    return 0;
}

static void test_thread_func(int *x){
    zz_log("[d] test_thread_func p = %p, x = %d\n", x, *x);
}

static void test_thread_finish_cb(){
    zz_log("[d] thread is finished !!!!\n");
    
    //    zz_print_alloc_description();
}

extern void test_thread(){
    //    zz_uninit_debug_alloc();
    //    zz_init_debug_alloc();
    
    zz_thread_poll *th_poll = alloc_zz_thread_poll(5, 0);
    
    usleep(3*1000000);
    
    int idxes[] = {1,2,3,4,5,6,7,8,9,10};
    
    int i = 0;
    for (; i < 10; i++) {
        zz_add_task_to_thread_poll(th_poll, (zz_thread_func)test_thread_func, idxes + i);
    }
    
    zz_log("[d] start sleep 10 sec...\n");
    usleep(10 * 1000000);
    zz_log("[d] finish sleep 10 sec...\n");
    free_zz_thread_poll(&th_poll, test_thread_finish_cb);
    zz_log("[d] main thread finished\n");
}


