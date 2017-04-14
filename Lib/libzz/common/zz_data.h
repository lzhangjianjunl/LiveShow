

/*
 数据块，可以保存任意二进制数据，并且提供读写方法。
 可以直接使用data_reader/data_writer直接对数据进行读写
 */

#ifndef zz_data_h
#define zz_data_h

#include <stdio.h>

//文件数据
typedef struct zz_data{
    uint32_t size;//有效数据长度
    uint32_t alloc_size;//分配的数据长度
    uint32_t curr_pos;//读取或写入的位置
    uint8_t *data;//实际数据
}zz_data;

//数据操作
//将数据fdata，copy到tdata中，自动扩展tdata大小
extern void memcpy_zz_data(zz_data **tdata, const void *fdata, int size);
//创建 zz_data 传入0表示默认大小10k
extern zz_data * alloc_zz_data(int size);
//copy zz_data
extern zz_data * copy_zz_data(zz_data *data);
//释放zz_data的空间
extern void free_zz_data(zz_data **data);
//给zz_data 扩容/缩小
extern void extend_zz_data(zz_data **old_zz_data, int new_size);
//重置 zz_data，清空数据，一切设为初始状态。
extern void reset_zz_data(zz_data **data);

//数据读取，自动处理大小端
typedef struct zz_data_reader{
    uint8_t (*read_uint8)(zz_data *);
    uint16_t (*read_uint16)(zz_data *);
    uint32_t (*read_uint24)(zz_data *);
    uint32_t (*read_uint32)(zz_data *);
    uint64_t (*read_uint64)(zz_data *);
    double (*read_double)(zz_data *);
    void (*read_string)(zz_data *, char **, int);
    void (*read_bytes)(zz_data *, char **, int);
    void (*skip_bytes)(zz_data *, int);
    int (*remain_count)(zz_data *);
    //debug，纪录某个程序段，一共读取数据的数量。
    void (*start_record_size)();
    size_t (*record_size)();
    void (*end_record_size)();
    void (*start_read)(zz_data *);
}zz_data_reader;

//数据写入，自动处理大小端
typedef struct zz_data_writer{
    void (*write_uint8)(zz_data **, uint8_t);
    void (*write_uint16)(zz_data **, uint16_t);
    void (*write_uint24)(zz_data **, uint32_t);
    void (*write_uint32)(zz_data **, uint32_t);
    void (*write_uint64)(zz_data **, uint64_t);
    void (*write_double)(zz_data **, double);
    void (*write_string)(zz_data **, const char *, uint32_t);
    void (*write_bytes)(zz_data **, const uint8_t *, uint32_t);
    void (*write_empty_bytes)(zz_data **, uint32_t);
    //debug，纪录某个程序段，一共写入数据的数量。
    void (*start_record_size)();
    size_t (*record_size)();
    void (*end_record_size)();
}zz_data_writer;

//全局读取reader
extern zz_data_reader data_reader;
//全局写入writer
extern zz_data_writer data_writer;

//测试本文件
extern void zz_data_test();

#endif /* zz_data_h */
