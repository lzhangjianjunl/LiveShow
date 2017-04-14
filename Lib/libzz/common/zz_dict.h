

/*
 字典，支持字典的基本操作
 */

#ifndef zz_dict_h
#define zz_dict_h

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//字典元素类型
typedef enum zz_dict_element_type{
    zz_DICT_ELEMENT_TYPE_INT,
    zz_DICT_ELEMENT_TYPE_DOUBLE,
    zz_DICT_ELEMENT_TYPE_STRING,
    zz_DICT_ELEMENT_TYPE_POINTER,
    zz_DICT_ELEMENT_TYPE_RELEASE_POINTER,
} zz_dict_element_type;

//字典元素
typedef struct zz_dict_element{
    const char *key;
    zz_dict_element_type type;
    union{
        int32_t int_value;
        double double_value;
        void * pointer_value;
        char * string_value;
    };
    void (*free_pointer)(void *, int);
    struct zz_dict_element *next;
    struct zz_dict_element *pre;
    struct zz_dict_element *repeat_next;//key值重复
    struct zz_dict_element *repeat_pre;//key值重复
    int extra;
}zz_dict_element;

extern zz_dict_element *alloc_zz_dict_element();
extern void free_zz_dict_element(zz_dict_element **, int8_t need_repeat);

//字典
typedef struct zz_dict{
    zz_dict_element *first;
    zz_dict_element *last;
    char repeat_seperate_word;//重复key的value，取值分隔符，默认为‘.’，设置为非字母和数字的不常见字符比较合适
}zz_dict;

extern zz_dict *alloc_zz_dict();
extern void free_zz_dict(zz_dict **);

//下列函数中的allow_repeat，表示如果dict中已经存在相同的key如何处理。
//allow_repeat为false，则函数调用失败。
//allow_repeat为true，则会给key绑定2个值。
//可以通过 zz_dict_get_xxx(dict, "key.0")，zz_dict_get_xxx(dict, "key.1")，的方式来区分不同的值。
//也可以通过 zz_dict_remove_object(dict, "key.0"), zz_dict_remove_object(dict, "key.1") 来移除指定的值。
//"key.0"，"key.1" 中的 "." 事实上是 zz_dict中的 repeat_seprate_word，默认为"."，可以根据需要修改此值。
//若改成"@"，同一个key对应多个value的访问方式可以这样写："key@0"，"key@1"

//字典存取int
extern int8_t zz_dict_set_int(zz_dict *dict, const char *key, int32_t value, int allow_repeat);
extern int32_t zz_dict_get_int(zz_dict *dict, const char *key);

//字典存取字符串
extern int8_t zz_dict_set_str(zz_dict *dict, const char *key, const char *v, int allow_repeat);
extern const char *zz_dict_get_str(zz_dict *dict, const char *key);

//字典存取double
extern int8_t zz_dict_set_double(zz_dict *dict, const char *key, double value, int allow_repeat);
extern double zz_dict_get_double(zz_dict *dict, const char *key);

//字典存取无需dict管理的指针
extern int8_t zz_dict_set_pointer(zz_dict *dict, const char *key, void * value, int allow_repeat);
extern void * zz_dict_get_pointer(zz_dict *dict, const char *key);

//字典存取需要dict释放的指针，需要传入释放该指针的release_func，否则会用zz_free释放。
extern int8_t zz_dict_set_release_pointer(zz_dict *dict, const char *key, void * value, void(*release_func)(void *, int), int extra, int allow_repeat);
extern void * zz_dict_get_release_pointer(zz_dict *dict, const char *key);

//字典中移除某个key
extern int8_t zz_dict_remove_object(zz_dict *dict, const char *key);

extern const char *zz_dict_description(zz_dict *dict);

extern void zz_dict_test();

#endif /* zz_dict_h */
