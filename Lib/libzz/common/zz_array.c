

#include "zz_array.h"
#include "zz_alloc.h"
#include <string.h>
#include "zz_utils.h"

#define zz_ARRAY_ALLOC_SIZE 100

extern zz_array_element *copy_zz_array_element(zz_array_element * ele){
    zz_array_element *new_ele = alloc_zz_array_element();
    memcpy(new_ele, ele, sizeof(zz_array_element));
    return new_ele;
}

extern zz_array_element *alloc_zz_array_element(){
    zz_array_element *array_ele = zz_alloc(sizeof(zz_array_element));
    memset(array_ele, 0, sizeof(zz_array_element));
    return array_ele;
}

extern void free_zz_array_element(zz_array_element **array_element){
    zz_array_element *ele = *array_element;
    switch (ele->type) {
        case zz_ARRAY_ELEMENT_TYPE_STRING:{
            if (ele->string_value) {
                zz_free(ele->string_value);
            }
            break;
        }
        case zz_ARRAY_ELEMENT_TYPE_RELEASE_POINTER: {
            if (ele->free_pointer_func) {
                ele->free_pointer_func(ele->pointer_value, ele->free_extra);
            }else{
                zz_free(ele->pointer_value);
            }
            break;
        }
        default:
            break;
    }
    
    zz_free(ele);
    *array_element = NULL;
}

extern zz_array *alloc_zz_array(size_t alloc_size){
    zz_array *array = zz_alloc(sizeof(zz_array));
    memset(array, 0, sizeof(zz_array));
    
    if (!alloc_size) {
        alloc_size = zz_ARRAY_ALLOC_SIZE;
    }
    
    array->array_indexes = zz_alloc(sizeof(zz_array_element *) * alloc_size);
    memset(array->array_indexes, 0, sizeof(zz_array_element *) * alloc_size);
    array->alloc_size = alloc_size;
    array->count = 0;
    
    return array;
}

extern void extend_zz_array(zz_array **array, size_t new_size){
    zz_array *now_array = *array;
    size_t now_size = now_array->alloc_size;
    if (!new_size) {
        now_size += zz_ARRAY_ALLOC_SIZE;
    }else{
        while (new_size >= now_size) {
            now_size += zz_ARRAY_ALLOC_SIZE;
        }
    }
    zz_array *new_array = alloc_zz_array(now_size);
    memcpy(new_array->array_indexes, now_array->array_indexes, now_array->alloc_size * sizeof(zz_array_element *));
    new_array->count = now_array->count;
    
    *array = new_array;
    
    //此时now_array的数组是浅copy到new_array中，所以释放now_array是不允许释放elements
    now_array->count = 0;
    free_zz_array(&now_array);
}

extern int8_t need_extend_zz_array(zz_array *array, int add_count){
    return array->count + add_count > array->alloc_size;
}

extern void free_zz_array(zz_array **array){
    zz_array *zz_array = *array;
    zz_array_element **elements = zz_array->array_indexes;
    size_t i = 0;
    for (; i < zz_array->count; i++) {
        free_zz_array_element(&elements[i]);
    }
    zz_free(zz_array->array_indexes);
    zz_free(zz_array);
    *array = NULL;
}

extern zz_array *move_zz_array(zz_array *array){
    zz_array *new_arr = zz_alloc(sizeof(zz_array));
    memcpy(new_arr, array, sizeof(zz_array));
    
    array->array_indexes = zz_alloc(sizeof(zz_array_element *) * array->alloc_size);
    memset(array->array_indexes, 0, sizeof(zz_array_element *) * array->alloc_size);
    array->count = 0;
    
    return new_arr;
}

extern void zz_array_insert_element(zz_array **array, zz_array_element *ele, int index){
    if (!array || !*array || !ele) {
        zzLog("[error] in zz_array_insert_element array or ele is NULL");
        return;
    }
    zz_array *zz_array = *array;
    if (need_extend_zz_array(zz_array, 1)) {
        extend_zz_array(array, zz_array->alloc_size + 1);
        zz_array = *array;
    }
    if (index >= zz_array->count) {
        zz_array->array_indexes[zz_array->count] = ele;
    }else {
        if(index < 0){
            index = 0;
        }
        memmove(zz_array->array_indexes + index + 1, zz_array->array_indexes + index, sizeof(zz_array_element *) * (zz_array->count - index));
        zz_array->array_indexes[index] = ele;
    }
    zz_array->count++;
}

extern void zz_array_add_element(zz_array **array, zz_array_element *ele){
    if (!array || !*array || !ele) {
        zzLog("[error] in zz_array_add_element array or ele is NULL");
        return;
    }
    zz_array_insert_element(array, ele, (int32_t)(*array)->count);
}

extern zz_array_element *zz_array_element_at_index(zz_array *array, int index){
    if (!array || index < 0 || index >= array->count) {
        zzLog("[error] in zz_array_element_at_index array is NULL or index out of range");
        return NULL;
    }
    return array->array_indexes[index];
}

extern void zz_array_remove_element(zz_array *array, zz_array_element *element){
    zz_array_element_at_index(array, zz_array_index_of_element(array, element));
}

extern void zz_array_remove_element_at_index(zz_array *array, int index){
    if (!array || index < 0 || index >= array->count) {
        zzLog("[error] in zz_array_remove_element_at_index array is NULL or index out of range");
        return;
    }
    zz_array_element *will_removed_ele = array->array_indexes[index];
    array->array_indexes[index] = NULL;
    if (index < array->count - 1) {
        memmove(array->array_indexes + index, array->array_indexes + index + 1, (array->count - 1 - index) * sizeof(zz_array_element *));
    }
    
    array->count--;
    
    free_zz_array_element(&will_removed_ele);
}

extern int32_t zz_array_index_of_element(zz_array *array, zz_array_element *ele){
    if (!array || !ele) {
        zzLog("[error] in zz_array_index_of_element array or element is NULL");
        return -1;
    }
    int i = 0;
    for (; i < (int)array->array_indexes; i++) {
        if (ele == array->array_indexes[i]) {
            return i;
        }
    }
    return -1;
}

extern zz_array_element *zz_array_add_int(zz_array **array, int32_t i){
    if (!array) {
        return NULL;
    }
    zz_array_element * ele = alloc_zz_array_element();
    ele->type = zz_ARRAY_ELEMENT_TYPE_INT;
    ele->int_value = i;
    
    zz_array_add_element(array, ele);
    
    return ele;
}

extern zz_array_element *zz_array_add_double(zz_array **array, double d){
    if (!array) {
        return NULL;
    }
    zz_array_element * ele = alloc_zz_array_element();
    ele->type = zz_ARRAY_ELEMENT_TYPE_DOUBLE;
    ele->double_value = d;
    
    zz_array_add_element(array, ele);
    
    return ele;
}

extern zz_array_element *zz_array_add_pointer(zz_array **array, void *pointer){
    if (!array) {
        return NULL;
    }
    zz_array_element * ele = alloc_zz_array_element();
    ele->type = zz_ARRAY_ELEMENT_TYPE_POINTER;
    ele->pointer_value = pointer;
    
    zz_array_add_element(array, ele);
    
    return ele;
}

extern zz_array_element *zz_array_add_string(zz_array **array, const char *str){
    if (!array) {
        return NULL;
    }
    zz_array_element * ele = alloc_zz_array_element();
    ele->type = zz_ARRAY_ELEMENT_TYPE_STRING;
    
    int32_t str_len = (int32_t)strlen(str);
    char *real_str = zz_alloc(str_len + 1);
    memset(real_str, 0, str_len + 1);
    memcpy(real_str, str, str_len);
    
    ele->string_value = real_str;
    
    zz_array_add_element(array, ele);
    
    return ele;
}

extern zz_array_element *zz_array_add_release_pointer(zz_array **array, void *pointer, void (*free_func)(void *, int), int free_extra){
    if (!array) {
        return NULL;
    }
    zz_array_element * ele = alloc_zz_array_element();
    ele->type = zz_ARRAY_ELEMENT_TYPE_RELEASE_POINTER;
    ele->pointer_value = pointer;
    ele->free_extra = free_extra;
    ele->free_pointer_func = free_func;
    
    zz_array_add_element(array, ele);
    
    return ele;
}

extern void zz_array_swap_element(zz_array *array, int index1, int index2){
    zz_array_element *ele1 = zz_array_element_at_index(array, index1);
    if (ele1) {
        zz_array_element *ele2 = zz_array_element_at_index(array, index2);
        if (ele2) {
            zz_array_element *tmp = ele1;
            array->array_indexes[index1] = ele2;
            array->array_indexes[index2] = tmp;
        }
    }
}

extern void zz_array_sort_bubble(zz_array *array, zz_array_sort_policy sort_policy, zz_array_sort_compare_func compare_func){
    int i = 0;
    for (; i < array->count; i++) {
        int j = i + 1;
        for (; j < array->count; j++) {
            switch(compare_func(zz_array_element_at_index(array, i), zz_array_element_at_index(array, j))) {
                case zz_array_sort_compare_result_great: {
                    if (sort_policy == zz_array_sort_policy_ascending) {
                        zz_array_swap_element(array, i, j);
                    }
                    break;
                }
                case zz_array_sort_compare_result_less: {
                    if (sort_policy == zz_array_sort_policy_descending) {
                        zz_array_swap_element(array, i, j);
                    }
                    break;
                }
                case zz_array_sort_compare_result_equal: {
                    // do nothing
                    break;
                }
            }
        }
    }
}

static void zz_array_sort_quick_inner(zz_array *array, int start, int end, zz_array_sort_policy sort_policy, zz_array_sort_compare_func compare_func){
    int len = end - start + 1;
    if (len <= 0) {
        return;
    }
    zz_array_element *key = zz_array_element_at_index(array, start);
    
    int i = start, j = end;
    while (j > i) {
        do{
            zz_array_element *end_ele = zz_array_element_at_index(array, j);
            zz_array_sort_compare_result compare_result = compare_func(key, end_ele);
            if(compare_result == zz_array_sort_compare_result_great && sort_policy == zz_array_sort_policy_ascending) {
                zz_array_swap_element(array, i, j);
                ++i;
                break;
            }else if(compare_result == zz_array_sort_compare_result_less && sort_policy == zz_array_sort_policy_descending) {
                zz_array_swap_element(array, i, j);
                ++i;
                break;
            }
        }while(--j > i);
        
        if (j <= i) {
            break;
        }
        
        do{
            zz_array_element *start_ele = zz_array_element_at_index(array, i);
            zz_array_sort_compare_result compare_result = compare_func(start_ele, key);
            if(compare_result == zz_array_sort_compare_result_great && sort_policy == zz_array_sort_policy_ascending) {
                zz_array_swap_element(array, i, j);
                --j;
                break;
            }else if(compare_result == zz_array_sort_compare_result_less&&sort_policy == zz_array_sort_policy_descending) {
                zz_array_swap_element(array, i, j);
                --j;
                break;
            }
        }while (++i < j);
    }
    
    zz_array_sort_quick_inner(array, start, i - 1, sort_policy, compare_func);
    zz_array_sort_quick_inner(array, i + 1, end, sort_policy, compare_func);
}

extern void zz_array_sort_quick(zz_array *array, zz_array_sort_policy sort_policy, zz_array_sort_compare_func compare_func){
    zz_array_sort_quick_inner(array, 0, (int32_t)array->count - 1, sort_policy, compare_func);
}

#include "zz_dict.h"

static zz_array_sort_compare_result test_zz_array_sort_compare_func(zz_array_element *element1, zz_array_element *element2){
    if (element1->int_value > element2->int_value) {
        return zz_array_sort_compare_result_great;
    }else if(element1->int_value == element2->int_value){
        return zz_array_sort_compare_result_equal;
    }else{
        return zz_array_sort_compare_result_less;
    }
}

static void test_zz_array_sort(zz_array_sort_func sort_func){
    zz_array *array = alloc_zz_array(0);
    int i = 0;
    int int_arr[10] = {5,6,2,7,8,3,4,1,9,0};
    for (; i < 10; i++) {
        int value = int_arr[i];
        zz_array_add_int(&array, value);
    }
    
    zzLog("排序前");
    for (i = 0; i < 10; i++) {
        zzLog("%d", zz_array_element_at_index(array, i)->int_value);
    }
    sort_func(array, zz_array_sort_policy_ascending, test_zz_array_sort_compare_func);
    
    zzLog("排序后 升序");
    for (i = 0; i < 10; i++) {
        zzLog("%d", zz_array_element_at_index(array, i)->int_value);
    }
    sort_func(array, zz_array_sort_policy_descending, test_zz_array_sort_compare_func);
    
    zzLog("排序后 降序");
    for (i = 0; i < 10; i++) {
        zzLog("%d", zz_array_element_at_index(array, i)->int_value);
    }
    
    free_zz_array(&array);
}

static void test_release_zz_dict(zz_dict *dict, int extra){
    free_zz_dict(&dict);
}

static void test_move_array(){
    zz_array *array = alloc_zz_array(1);
    const char *s = "test string";
    zz_array_add_string(&array, s);
    
    //test release pointer
    zz_dict *dict = alloc_zz_dict();
    int i = 0;
    for (; i < 10; i++) {
        zz_dict_set_int(dict, "x", i, 1);
    }
    zz_array_add_release_pointer(&array, dict, (void (*)(void *, int))test_release_zz_dict, 0);
    
    zz_array *new_arr = move_zz_array(array);
    
    free_zz_array(&array);
    free_zz_array(&new_arr);
}

extern void test_zz_array(){
    zz_uninit_debug_alloc();
    zz_init_debug_alloc();
    // test insert
    zz_array *array = alloc_zz_array(1);
    
    int i = 0;
    for (; i < 10; i++) {
        zz_array_add_int(&array, i);
    }
    
    i = 0;
    for (; i < array->count; i++) {
        zzLog(" - %d", zz_array_element_at_index(array, i)->int_value);
    }
    
    i = 0;
    for (; i < 5; i++) {
        zz_array_remove_element_at_index(array, i);
    }
    
    i = 0;
    for (; i < array->count; i++) {
        zzLog(" -- %d", zz_array_element_at_index(array, i)->int_value);
    }
    
    zzLog("test add and remove end");
    
    //test string
    const char *s = "test string";
    zz_array_add_string(&array, s);
    
    //test release pointer
    zz_dict *dict = alloc_zz_dict();
    i = 0;
    for (; i < 10; i++) {
        zz_dict_set_int(dict, "x", i, 1);
    }
    zz_array_add_release_pointer(&array, dict, (void (*)(void *, int))test_release_zz_dict, 0);
    
    zz_array_add_double(&array, 1.236);
    
    zzLog(" test 5(%s)", zz_array_element_at_index(array, 5)->string_value);
    
    zz_dict *get_dict = (zz_dict *)zz_array_element_at_index(array, 6)->pointer_value;
    i = 0;
    for (; i < 10; i++) {
        char xx[5];
        memset(xx, 0, 5);
        sprintf(xx, "x.%d", i);
        zzLog(" test 6-%d(%d)", i, zz_dict_get_int(get_dict, xx));
    }
    zzLog(" test 7(%f)", zz_array_element_at_index(array, 7)->double_value);
    
    free_zz_array(&array);
    
    //    test_zz_array_sort(zz_array_sort_bubble);
    test_zz_array_sort(zz_array_sort_quick);
    
    test_move_array();
    
    zz_print_alloc_description();
}
