

#include "zz_dict.h"
#include "zz_utils.h"

extern zz_dict_element *alloc_zz_dict_element(){
    zz_dict_element *ele = (zz_dict_element *)zz_alloc(sizeof(zz_dict_element));
    memset(ele, 0, sizeof(zz_dict_element));
    return ele;
}

extern void free_zz_dict_element(zz_dict_element **ele, int8_t need_repeat){
    zz_dict_element *ele_inner = *ele;
    zz_dict_element *repeat_ele = ele_inner->repeat_next;
    if (need_repeat) {
        while (repeat_ele) {
            zz_dict_element *inner_repeat_ele = repeat_ele;
            repeat_ele = repeat_ele->repeat_next;
            free_zz_dict_element(&inner_repeat_ele, 0);
        }
    }
    switch (ele_inner->type) {
        case zz_DICT_ELEMENT_TYPE_STRING:
            zz_free((void *)ele_inner->string_value);
            break;
        case zz_DICT_ELEMENT_TYPE_RELEASE_POINTER:
            if (ele_inner->free_pointer) {
                ele_inner->free_pointer(ele_inner->pointer_value, ele_inner->extra);
            }else{
                zz_free(ele_inner->pointer_value);
            }
            break;
        default:
            break;
    }
    if (ele_inner->key) {
        zz_free((void *)ele_inner->key);
    }
    zz_free(ele_inner);
    *ele = NULL;
}

extern zz_dict *alloc_zz_dict(){
    zz_dict *dict = (zz_dict *)zz_alloc(sizeof(zz_dict));
    memset(dict, 0, sizeof(zz_dict));
    dict->repeat_seperate_word = '.';
    return dict;
}

extern void free_zz_dict(zz_dict **dict){
    zz_dict *dictp = *dict;
    zz_dict_element *ele = dictp->first;
    while (ele) {
        dictp->first = ele->next;
        free_zz_dict_element(&ele, 1);
        ele = dictp->first;
    }
    zz_free(*dict);
    *dict = NULL;
}

static int is_equal_key(const char *key1, const char *key2){
    return !strcmp(key1, key2);
}

static int8_t parse_realkey_and_index(const char *key, char **real_key, int *index, char repeat_seprate_word){
    char *p_c = strchr(key, repeat_seprate_word);
    if (p_c) {
        size_t real_key_size = p_c - key;
        char *in_real_key = zz_alloc(real_key_size);
        memset(in_real_key, 0, real_key_size);
        memcpy(in_real_key, key, real_key_size);
        *real_key = in_real_key;
        
        size_t count_str_size = strlen(key) - real_key_size - 1;
        char *count_str = zz_alloc(count_str_size + 1);
        memset(count_str, 0, count_str_size + 1);
        memcpy(count_str, key + real_key_size + 1, count_str_size);
        *index = atoi(count_str);
        zz_free(count_str);
        
        return 1;
    }else{
        *real_key = (char *)key;
        
        return 0;
    }
}

static zz_dict_element * zz_dict_find_ele(zz_dict *dict, const char *key){
    zz_dict_element *ele = dict->first;
    if (!ele || !key || !ele->key) {
        return NULL;
    }
    
    char *real_key = NULL;
    int repeat_count = -1;
    int8_t need_free_key = parse_realkey_and_index(key, &real_key, &repeat_count, dict->repeat_seperate_word);
    
    zz_dict_element *repeat_ele = NULL;
    while (ele) {
        if (!is_equal_key(real_key, ele->key)) {
            ele = ele->next;
        }else{
            repeat_ele = ele;
            break;
        }
    }
    
    if (need_free_key) {
        zz_free(real_key);
    }
    
    //查找repeat_next
    if (repeat_count >= 0) {
        int i = 0;
        for (; i < repeat_count; i++) {
            if (!repeat_ele) {
                return NULL;
            }
            repeat_ele = repeat_ele->repeat_next;
        }
    }
    
    return repeat_ele;
}

static zz_dict_element *zz_dict_add_new_ele(zz_dict *dict, const char *key, int allow_repeat){
    zz_dict_element *ele = zz_dict_find_ele(dict, key);
    
    //检查key，不允许带"."
    char *p_c = strchr(key, dict->repeat_seperate_word);
    if (p_c) {
        zzLog("in zz_dict_add_new_ele dont contains '%c' in key(%s)", dict->repeat_seperate_word, key);
        return NULL;
    }
    
    if (ele) {
        if (allow_repeat) {
            zz_dict_element *repeat_ele = ele;
            while (repeat_ele->repeat_next) {
                repeat_ele = repeat_ele->repeat_next;
            }
            zz_dict_element *new_repeat_ele = alloc_zz_dict_element();
            repeat_ele->repeat_next = new_repeat_ele;
            new_repeat_ele->repeat_pre = repeat_ele;
            ele = new_repeat_ele;
        }else{
            ele = NULL;
        }
    }else{
        if (!dict->first) {
            dict->first = alloc_zz_dict_element();
            dict->last = dict->first;
            ele = dict->first;
        }else{
            ele = alloc_zz_dict_element();
            dict->last->next = ele;
            ele->pre = dict->last;
            dict->last = ele;
        }
    }
    
    if (ele) {
        int ele_key_len = (int)strlen(key) + 1;
        char *ele_key = zz_alloc(ele_key_len);
        memset(ele_key, 0, ele_key_len);
        memcpy(ele_key, key, ele_key_len - 1);
        
        ele->key = ele_key;
    }
    
    return ele;
}

int8_t zz_dict_remove_object(zz_dict *dict, const char *key){
    zz_dict_element *first = dict->first;
    if (!first || !key || !first->key) {
        return 0;
    }
    
    char *real_key = NULL;
    int repeat_count = -1;
    int8_t need_free_key = parse_realkey_and_index(key, &real_key, &repeat_count, dict->repeat_seperate_word);
    
    if (real_key) {
        zz_dict_element *ele = zz_dict_find_ele(dict, key);
        if (need_free_key) {
            zz_free(real_key);
        }
        if (ele) {
            if (repeat_count >= 0) {
                //移除的是repeat的element
                if (repeat_count == 0) {
                    //移除repeat结点中的根结点
                    zz_dict_element *repeat_next = ele->repeat_next;
                    if (repeat_next) {
                        if (ele->pre) {
                            repeat_next->pre = ele->pre;
                            ele->pre->next = repeat_next;
                        }
                        if (ele->next) {
                            repeat_next->next = ele->next;
                            ele->next->pre = repeat_next;
                        }
                        if (ele == dict->first) {
                            dict->first = repeat_next;
                        }else if(ele == dict->last){
                            dict->last = repeat_next;
                        }
                    }else{
                        if (ele == dict->first) {
                            dict->first = ele->next;
                        }else if(ele == dict->last){
                            dict->last = ele->pre;
                        }else{
                            ele->pre->next = ele->next;
                            ele->next->pre = ele->pre;
                            ele->pre = NULL;
                            ele->next = NULL;
                        }
                    }
                }else{
                    //移除repeat结点中的非根结点
                    ele->repeat_pre->repeat_next = ele->repeat_next;
                    ele->repeat_next->repeat_pre = ele->repeat_pre;
                    ele->pre = NULL;
                    ele->next = NULL;
                }
                free_zz_dict_element(&ele, 0);
            }else{
                //移除的是主干上的结点
                if (ele == dict->first) {
                    dict->first = ele->next;
                }else if(ele == dict->last){
                    dict->last = ele->pre;
                }else{
                    ele->pre->next = ele->next;
                    ele->next->pre = ele->pre;
                    ele->pre = NULL;
                    ele->next = NULL;
                }
                free_zz_dict_element(&ele, 1);
            }
            
            //检查dict
            if(dict->first){
                dict->first->pre = NULL;
                if (dict->first == dict->last) {
                    dict->last = NULL;
                }
            }
            
            if (dict->last) {
                dict->last->next = NULL;
            }
            
            return 1;
        }
    }
    
    return 0;
}

int8_t zz_dict_set_str(zz_dict *dict, const char *key, const char *str, int allow_repeat){
    zz_dict_element *new_ele = zz_dict_add_new_ele(dict, key, allow_repeat);
    
    if (new_ele) {
        int str_len = (int)strlen(str) + 1;
        char *new_str = zz_alloc(str_len);
        memcpy(new_str, str, str_len);
        new_ele->string_value = new_str;
        new_ele->type = zz_DICT_ELEMENT_TYPE_STRING;
        return 1;
    }
    return 0;
}

const char *zz_dict_get_str(zz_dict *dict, const char *key){
    zz_dict_element *ele = zz_dict_find_ele(dict, key);
    if (ele) {
        return ele->string_value;
    }
    return 0;
}

int8_t zz_dict_set_int(zz_dict *dict, const char *key, int32_t value, int allow_repeat){
    zz_dict_element *new_ele = zz_dict_add_new_ele(dict, key, allow_repeat);
    if (new_ele) {
        new_ele->int_value = value;
        new_ele->type = zz_DICT_ELEMENT_TYPE_INT;
        return 1;
    }
    return 0;
}

int32_t zz_dict_get_int(zz_dict *dict, const char *key){
    zz_dict_element *ele = zz_dict_find_ele(dict, key);
    if (ele) {
        return ele->int_value;
    }
    return 0;
}

int8_t zz_dict_set_double(zz_dict *dict, const char *key, double value, int allow_repeat){
    zz_dict_element *new_ele = zz_dict_add_new_ele(dict, key, allow_repeat);
    if (new_ele) {
        new_ele->double_value = value;
        new_ele->type = zz_DICT_ELEMENT_TYPE_DOUBLE;
        return 1;
    }
    return 0;
}

double zz_dict_get_double(zz_dict *dict, const char *key){
    zz_dict_element *ele = zz_dict_find_ele(dict, key);
    if (ele) {
        return ele->double_value;
    }
    return 0;
}

int8_t zz_dict_set_pointer(zz_dict *dict, const char *key, void * value, int allow_repeat){
    zz_dict_element *new_ele = zz_dict_add_new_ele(dict, key, allow_repeat);
    if (new_ele) {
        new_ele->pointer_value = value;
        new_ele->type = zz_DICT_ELEMENT_TYPE_POINTER;
        return 1;
    }
    return 0;
}

void * zz_dict_get_pointer(zz_dict *dict, const char *key){
    zz_dict_element *ele = zz_dict_find_ele(dict, key);
    if (ele) {
        return ele->pointer_value;
    }
    return 0;
}

int8_t zz_dict_set_release_pointer(zz_dict *dict, const char *key, void * value, void(*release_func)(void *, int), int extra, int allow_repeat){
    zz_dict_element *new_ele = zz_dict_add_new_ele(dict, key, allow_repeat);
    if (new_ele) {
        new_ele->pointer_value = value;
        new_ele->free_pointer = release_func;
        new_ele->extra = extra;
        new_ele->type = zz_DICT_ELEMENT_TYPE_RELEASE_POINTER;
        return 1;
    }
    return 0;
}

void * zz_dict_get_release_pointer(zz_dict *dict, const char *key){
    zz_dict_element *ele = zz_dict_find_ele(dict, key);
    if (ele) {
        return ele->pointer_value;
    }
    return 0;
}

const char *zz_dict_description(zz_dict *dict){
    return "";
}

//=============test=================

void zz_dict_test_int(){
    zz_dict *dict = alloc_zz_dict();
    //test int
    zz_dict_set_int(dict, "a", 1, 0);
    int a = zz_dict_get_int(dict, "a");
    zzLog("a=%d", a);
    
    //repeat int
    zz_dict_set_int(dict, "a", 2, 1);
    zz_dict_set_int(dict, "a", 3, 1);
    zz_dict_set_int(dict, "a", 4, 1);
    zz_dict_set_int(dict, "a", 5, 1);
    int a1 = zz_dict_get_int(dict, "a.1");
    int a2 = zz_dict_get_int(dict, "a.2");
    zzLog("a=%d, a1=%d, a2=%d", a, a1, a2);
    
    //test double
    zz_dict_set_double(dict, "a", 6.6, 1);
    zz_dict_set_double(dict, "a", 7.7, 1);
    
    zz_dict_set_double(dict, "b", 8.8, 1);
    zz_dict_set_double(dict, "b", 9.9, 1);
    zz_dict_set_double(dict, "b", 10.10, 1);
    zz_dict_set_double(dict, "b", 11.11, 1);
    zz_dict_set_double(dict, "b", 12.12, 1);
    zz_dict_set_double(dict, "b", 13.13, 1);
    double d1 = zz_dict_get_double(dict, "a.3");
    double d2 = zz_dict_get_double(dict, "a.4");
    double d3 = zz_dict_get_double(dict, "b.0");
    double d4 = zz_dict_get_double(dict, "b.1");
    zzLog("d1=%f, d2=%f, d3=%f, d4=%f", d1, d2, d3, d4);
    
    zz_dict_remove_object(dict, "a.0");
    zz_dict_remove_object(dict, "a.1");
    
    a1 = zz_dict_get_int(dict, "a.1");
    a2 = zz_dict_get_int(dict, "a.2");
    zzLog("---a1=%d, a2=%d", a1, a2);
    
    zz_dict_remove_object(dict, "b.0");
    zz_dict_remove_object(dict, "b.1");
    zz_dict_remove_object(dict, "b.100");
    
    d1 = zz_dict_get_double(dict, "a.3");
    d2 = zz_dict_get_double(dict, "a.4");
    d3 = zz_dict_get_double(dict, "b.0");
    d4 = zz_dict_get_double(dict, "b.1");
    zzLog("d1=%f, d2=%f, d3=%f, d4=%f", d1, d2, d3, d4);//6.6, 7.7, 9.9, 11.11
    
    zz_dict_remove_object(dict, "a");
    zz_dict_remove_object(dict, "b");
    
    
    free_zz_dict(&dict);
    
    zzLog("zz_dict_test int total alloc size = %ld, total free size = %ld", zz_total_alloc_size(), zz_total_free_size());
    zz_uninit_debug_alloc();
}

static void zz_dict_test_release_pointer(zz_dict ** entries, int count){
    int i = 0;
    for (; i < count; i++) {
        zz_dict * dict = entries[i];
        free_zz_dict(&dict);
    }
    zz_free(entries);
}

static void zz_dict_test_pointer(){
    zz_uninit_debug_alloc();
    zz_init_debug_alloc();
    
    zz_dict *dict = alloc_zz_dict();
    
    int count = 100;
    
    zz_dict **entries = zz_alloc(sizeof(zz_dict *) * count);
    
    int i = 0;
    for (; i < count; i++) {
        zz_dict *dict_in = alloc_zz_dict();
        zz_dict_set_int(dict_in, "xx", i, 1);
        entries[i] = dict_in;
    }
    
    zz_dict_set_release_pointer(dict, "a", entries, (void (*)(void *, int))zz_dict_test_release_pointer, count, 1);
    
    free_zz_dict(&dict);
    
    zzLog("zz_dict_test_pointer int total alloc size = %ld, total free size = %ld", zz_total_alloc_size(), zz_total_free_size());
    
    zz_uninit_debug_alloc();
}

void zz_dict_test(){
    zz_dict_test_int();
    zz_dict_test_pointer();
}
