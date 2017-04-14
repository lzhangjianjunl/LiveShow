

#ifndef zz_array_h
#define zz_array_h

/*
 任意类型数组，可以存储任意类型的数据。包含了对数组数据的增删改查及排序
 */

#include <stdio.h>

typedef enum zz_array_element_type{
    zz_ARRAY_ELEMENT_TYPE_INT,
    zz_ARRAY_ELEMENT_TYPE_DOUBLE,
    zz_ARRAY_ELEMENT_TYPE_STRING,
    zz_ARRAY_ELEMENT_TYPE_POINTER,
    zz_ARRAY_ELEMENT_TYPE_RELEASE_POINTER,
} zz_array_element_type;

typedef struct zz_array_element{
    zz_array_element_type type;
    union{
        int32_t int_value;
        double double_value;
        void * pointer_value;
        char * string_value;
    };
    void (*free_pointer_func)(void *, int);
    int free_extra;
}zz_array_element;

extern zz_array_element *copy_zz_array_element(zz_array_element *);
extern zz_array_element *alloc_zz_array_element();
extern void free_zz_array_element(zz_array_element **);

typedef struct zz_array{
    zz_array_element **array_indexes;
    size_t alloc_size;
    size_t count;
}zz_array;

extern zz_array *alloc_zz_array(size_t);
extern void free_zz_array(zz_array **);
extern zz_array *move_zz_array(zz_array *array);

//基础功能
//插入元素
extern void zz_array_insert_element(zz_array **array, zz_array_element *ele, int index);
//尾部插入元素
extern void zz_array_add_element(zz_array **array, zz_array_element *ele);
//查找元素
extern zz_array_element *zz_array_element_at_index(zz_array *array, int index);
//移除元素
extern void zz_array_remove_element(zz_array *array, zz_array_element *element);
//根据下标移除元素
extern void zz_array_remove_element_at_index(zz_array *array, int index);
//获取指定元素的下标
extern int32_t zz_array_index_of_element(zz_array *array, zz_array_element *ele);
//元素交换
extern void zz_array_swap_element(zz_array *, int, int);

//具体功能函数
//加入一个int数据
extern zz_array_element *zz_array_add_int(zz_array **, int32_t);
//加入一个double数据
extern zz_array_element *zz_array_add_double(zz_array **, double);
//加入一个无需array管理释放的指针数据
extern zz_array_element *zz_array_add_pointer(zz_array **, void *);
//加入一个字符串数据，会copy一份新的
extern zz_array_element *zz_array_add_string(zz_array **, const char *);
//加入一个需要array管理释放的指针，但是要传入指定类型的释放该指针的方法，如果不传入释放方法，默认使用zz_free释放
extern zz_array_element *zz_array_add_release_pointer(zz_array **, void *, void (*)(void *, int), int);

//数组排序，实现了冒泡和快速排序
//比较函数结果
typedef enum zz_array_sort_compare_result {
    zz_array_sort_compare_result_great = -1,
    zz_array_sort_compare_result_less,
    zz_array_sort_compare_result_equal,
} zz_array_sort_compare_result;

//升序or降序排列
typedef enum zz_array_sort_policy{
    zz_array_sort_policy_ascending,
    zz_array_sort_policy_descending
}zz_array_sort_policy;

//排序数组元素比较函数
typedef zz_array_sort_compare_result(*zz_array_sort_compare_func) (zz_array_element* obj1, zz_array_element *obj2);

//排序函数指针
typedef void(*zz_array_sort_func)(zz_array *, zz_array_sort_policy, zz_array_sort_compare_func);

//冒泡排序
extern void zz_array_sort_bubble(zz_array *, zz_array_sort_policy, zz_array_sort_compare_func);

//快速排序
extern void zz_array_sort_quick(zz_array *, zz_array_sort_policy, zz_array_sort_compare_func);

//测试
extern void test_zz_array();

#endif /* zz_array_h */
