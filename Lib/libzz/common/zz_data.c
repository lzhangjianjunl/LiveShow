

#include "zz_data.h"
#include <string.h>
#include <stdlib.h>
#include "zz_utils.h"

//record size

static int8_t zz_is_started_record_size = 0;
static uint32_t zz_recorded_size = 0;

static void zz_start_record_size(){
    if (zz_is_started_record_size) {
        zzLog("[ERROR] record size is already in use!!");
        return;
    }
    zz_is_started_record_size = 1;
    zz_recorded_size = 0;
}

static void zz_increase_record_size(size_t size){
    if (zz_is_started_record_size) {
        zz_recorded_size += size;
    }
}

static size_t zz_get_recorded_size(){
    return zz_recorded_size;
}

static void zz_end_record_size(){
    zz_is_started_record_size = 0;
    zz_recorded_size = 0;
}

#define zz_DATA_ALLOC_BLOCK 10 * 1024

void memcpy_zz_data(zz_data **tdata, const void *fdata, int size){
    if (!tdata) {
        return;
    }
    zz_data *tdatap = *tdata;
    if (!tdatap) {
        tdatap = alloc_zz_data(0);
        *tdata = tdatap;
    }
    int new_size = tdatap->alloc_size;
    while (new_size < tdatap->size + size) {
        new_size += zz_DATA_ALLOC_BLOCK;
    }
    if (new_size > tdatap->alloc_size) {
        extend_zz_data(tdata, new_size);
        tdatap = *tdata;
    }
    
    memcpy(tdatap->data + tdatap->curr_pos, fdata, size);
    tdatap->curr_pos += size;
    tdatap->size = tdatap->curr_pos;
}

zz_data * alloc_zz_data(int size){
    int zzdatasize = sizeof(zz_data);
    zz_data *zzdata = (zz_data *)zz_alloc(zzdatasize);
    memset(zzdata, 0, zzdatasize);
    if (size == 0) {
        size = zz_DATA_ALLOC_BLOCK;
    }
    zzdata->data = zz_alloc(size);
    zzdata->alloc_size = size;
    return zzdata;
}

extern zz_data * copy_zz_data(zz_data *data){
    if (!data) {
        return NULL;
    }
    zz_data *new_data = alloc_zz_data(data->alloc_size);
    memcpy_zz_data(&new_data, data->data, data->size);
    return new_data;
}

void extend_zz_data(zz_data **old_zz_data, int new_size){
    zz_data * new_zz_data = alloc_zz_data(new_size);
    memcpy_zz_data(&new_zz_data, (*old_zz_data)->data, (*old_zz_data)->size);
    free_zz_data(old_zz_data);
    *old_zz_data = new_zz_data;
}

extern void reset_zz_data(zz_data **data){
    zz_data *zz_data = *data;
    if (zz_data && zz_data->curr_pos > 0 && zz_data->size > 0) {
        zz_data->curr_pos = 0;
        memset(zz_data->data, 0, zz_data->size);
        zz_data->size = 0;
    }
}

void free_zz_data(zz_data **data){
    zz_data *zz_data = *data;
    zz_free(zz_data->data);
    zz_data->size = 0;
    zz_data->alloc_size = 0;
    zz_data->curr_pos = 0;
    zz_data->data = 0;
    zz_free(zz_data);
    *data = NULL;
}

//大小端转换
#define CONVERT_32(i, store, size) \
do{\
store = 0;\
for(int j = 0; j < size; j++){ \
store |=  (((i & (0xff << j * 8)) >> j * 8) & 0xff) << (((int)size - 1 - j) * 8); \
} \
}while(0)

//大小端不影响移位操作
#define CONVERT_64(i, store, size) \
do{ \
/*后面*/ \
uint32_t i1 = (uint32_t)i; \
uint32_t store1 = 0; \
CONVERT_32(i1, store1, 4); \
uint32_t *ipre = (uint32_t *)&store + 1; \
memcpy(ipre, &store1, 4); \
/*前面*/\
uint32_t i2 = i >> 32; \
uint32_t store2 = 0; \
CONVERT_32(i2, store2, 4); \
uint32_t *itail = (uint32_t *)&store; \
memcpy(itail, &store2, 4); \
}while(0)

#define CONVERT(i, store, size) \
do{ \
if(size < 8){ \
CONVERT_32(i, store, size); \
} else { \
CONVERT_64(i, store, size); \
} \
}while(0)

//读取数据
//读取单字节
#define READ_BYTE(type, i) \
size_t type_size = sizeof(type); \
memcpy(&i, zzdata->data + zzdata->curr_pos, type_size); \
zzdata->curr_pos += type_size; \
zz_increase_record_size(type_size);

//读取多字节，系统字节序为小端
#define READ_MUTIBYTES_FOR_LITTLE_ENDIAN(i, type, size) \
do{ \
READ_MUTIBYTES_FOR_BIG_ENDIAN(i, type, size); \
type store = 0; \
CONVERT(i, store, size); \
i = store; \
}while(0);

//读取多字节，系统字节序为大端
#define READ_MUTIBYTES_FOR_BIG_ENDIAN(i, type, isize) \
do{\
memcpy(&i, zzdata->data + zzdata->curr_pos, isize); \
zzdata->curr_pos += isize; \
zz_increase_record_size(isize); \
} while(0);

//读取多字节，自动判断大小端
#define READ_MUTIBYTES(i, type, size) \
do{\
if(is_little_endian()) {\
READ_MUTIBYTES_FOR_LITTLE_ENDIAN(i, type, size); \
}else{\
READ_MUTIBYTES_FOR_BIG_ENDIAN(i, type, size); \
}\
}while(0);

//读取多字节带返回值
#define RETURN_FOR_READ_MUTIBYTES(type) \
type i; \
READ_MUTIBYTES(i, type, sizeof(type)); \
return i;

static uint8_t is_little_endian(){
    union {
        int a;
        uint8_t b;
    }c;
    c.a = 1;
    return c.b == 1;
}

static void start_read(zz_data *zzdata){
    zzdata->curr_pos = 0;
}

static uint8_t read_uint8(zz_data *zzdata){
    uint8_t i;
    READ_BYTE(uint8_t, i);
    return i;
}

static uint16_t read_uint16(zz_data *zzdata){
    RETURN_FOR_READ_MUTIBYTES(uint16_t);
}

static uint32_t read_uint24(zz_data *zzdata){
    uint32_t i;
    READ_MUTIBYTES(i, uint32_t, 3);
    return i;
}

static uint32_t read_uint32(zz_data *zzdata){
    RETURN_FOR_READ_MUTIBYTES(uint32_t);
}

static uint64_t read_uint64(zz_data *zzdata){
    RETURN_FOR_READ_MUTIBYTES(uint64_t);
}

static double read_double(zz_data *zzdata){
    uint64_t i;
    READ_MUTIBYTES(i, uint64_t, sizeof(i));
    double d = 0;
    memcpy(&d, &i, sizeof(i));
    return d;
}

static void read_string(zz_data* zzdata, char **string, int len){
    *string = zz_alloc(len);
    memset(*string, 0, len + 1);
    memcpy(*string, zzdata->data + zzdata->curr_pos, len);
    zzdata->curr_pos += len;
    zz_increase_record_size(len);
}

static void read_bytes(zz_data * zzdata, char **bytes, int len){
    *bytes = zz_alloc(len);
    memset(*bytes, 0, len);
    memcpy(*bytes, zzdata->data + zzdata->curr_pos, len);
    zzdata->curr_pos += len;
    zz_increase_record_size(len);
}

static void skip_bytes(zz_data* zzdata, int count){
    zzdata->curr_pos += count;
    zz_increase_record_size(count);
}

static int remain_count(zz_data *zzdata){
    return zzdata->size - zzdata->curr_pos;
}

//写入
static void write_uint8(zz_data **zzdata, uint8_t v){
    memcpy_zz_data(zzdata, &v, 1);
    zz_increase_record_size(1);
}

static void write_uint16(zz_data **zzdata, uint16_t v){
    uint16_t big = v;
    if (is_little_endian()) {
        CONVERT(v, big, sizeof(v));
    }
    memcpy_zz_data(zzdata, &big, sizeof(uint16_t));
    zz_increase_record_size(2);
}

static void write_uint24(zz_data **zzdata, uint32_t v){
    uint32_t big = v;
    if (is_little_endian()) {
        CONVERT(v, big, 3);
    }
    memcpy_zz_data(zzdata, &big, sizeof(uint8_t) * 3);
    zz_increase_record_size(3);
}

static void write_uint32(zz_data **zzdata, uint32_t v){
    uint32_t big = v;
    if (is_little_endian()) {
        CONVERT(v, big, sizeof(v));
    }
    memcpy_zz_data(zzdata, &big, sizeof(uint32_t));
    zz_increase_record_size(4);
}

static void write_uint64(zz_data **zzdata, uint64_t v){
    uint64_t big = v;
    if (is_little_endian()) {
        CONVERT(v, big, sizeof(v));
    }
    memcpy_zz_data(zzdata, &big, sizeof(uint64_t));
    zz_increase_record_size(8);
}

static void write_double(zz_data **zzdata, double v){
    uint64_t newV = 0;
    memcpy(&newV, &v, sizeof(uint64_t));
    write_uint64(zzdata, newV);
}

static void write_bytes(zz_data **zzdata, const uint8_t *bytes, uint32_t count){
    memcpy_zz_data(zzdata, bytes, (int)count);
    zz_increase_record_size(count);
}

static void write_string(zz_data **zzdata, const char *str, uint32_t len_bytes_count){
    size_t str_len = strlen((const char *)str);
    switch (len_bytes_count) {
        case 1:
            write_uint8(zzdata, (uint8_t) str_len);
            break;
        case 2:
            write_uint16(zzdata, (uint16_t) str_len);
            break;
        case 4:
            write_uint32(zzdata, (uint32_t) str_len);
            break;
        case 8:
            write_uint64(zzdata, (uint64_t) str_len);
            break;
        default:
            break;
    }
    
    write_bytes(zzdata, (const uint8_t *)str, (int)str_len);
}

static void write_empty_bytes(zz_data **zzdata, uint32_t count){
    uint8_t *empty_bytes = (uint8_t *)zz_alloc(count);
    memset(empty_bytes, 0, count);
    write_bytes(zzdata, empty_bytes, count);
}

//初始化 zzdata operation
zz_data_reader data_reader = {
    .start_read = start_read,
    .read_uint8 = read_uint8,
    .read_uint16 = read_uint16,
    .read_uint24 = read_uint24,
    .read_uint32 = read_uint32,
    .read_uint64 = read_uint64,
    .read_double = read_double,
    .read_string = read_string,
    .read_bytes = read_bytes,
    .skip_bytes = skip_bytes,
    .remain_count = remain_count,
    //debug
    .start_record_size = zz_start_record_size,
    .record_size = zz_get_recorded_size,
    .end_record_size = zz_end_record_size,
};

zz_data_writer data_writer = {
    .write_uint8 = write_uint8,
    .write_uint16 = write_uint16,
    .write_uint24 = write_uint24,
    .write_uint32 = write_uint32,
    .write_uint64 = write_uint64,
    .write_double = write_double,
    .write_string = write_string,
    .write_bytes = write_bytes,
    .write_empty_bytes = write_empty_bytes,
    //debug
    .start_record_size = zz_start_record_size,
    .record_size = zz_get_recorded_size,
    .end_record_size = zz_end_record_size,
};

#define StringMark1(mark) #mark
#define StringMark(mark) StrintMark1(mark)

#define TWO 2

#define PRINT(n) zzLog("%s", StringMark(n))

#define TEST(type, funcname, value, printMark) \
wdata = alloc_zz_data(0); \
type funcname##1 = value; \
data_writer.write_##funcname(&wdata, funcname##1); \
data_reader.start_read(wdata); \
type funcname##2 = data_reader.read_##funcname(wdata); \
zzLog(#type".1 = %"printMark", u"#type".2 = %"printMark, funcname##1, funcname##2); \
free_zz_data(&wdata);

static void zz_data_test_convert(){
    uint8_t u8 = 0x12;
    uint8_t u81 = 0;
    CONVERT(u8, u81, 1);
    zzLog("u8=%x, u81=%x", u8, u81);
    
    uint16_t u16 = 0x1234;
    uint16_t u161 = 0;
    CONVERT_32(u16, u161, 2);
    zzLog("u16=%x, u161=%x", u16, u161);
    
    uint32_t u24 = 0x123456;
    uint32_t u241 = 0;
    CONVERT_32(u24, u241, 3);
    zzLog("u24=%x, u241=%x", u24, u241);
    
    uint32_t u32 = 0x12345678;
    uint32_t u321 = 0;
    CONVERT_32(u32, u321, 4);
    zzLog("u32=%x, u321=%x", u32, u321);
    
    uint64_t u64 = 0x12345678aabbccdd;
    uint64_t u641 = 0;
    CONVERT_64(u64, u641, 8);
    zzLog("u64=%llx, u641=%llx", u64, u641);
}

extern void zz_data_test(){
    zzLog("小端 ＝ %d", is_little_endian());
    
    zz_data_test_convert();
    zz_data *
    //    8字节
    TEST(uint8_t, uint8, 0xaa, "x");
    TEST(uint16_t, uint16, 0xaabb, "x");
    TEST(uint32_t, uint24, 0xabcdef, "x");
    TEST(uint32_t, uint32, 0x12345678, "x");
    TEST(uint64_t, uint64, 0x12345678aabbccdd, "llx");
    
    
    double d1 = 0.12345678;
    uint64_t di1 = 0;
    memcpy(&di1, &d1, sizeof(uint64_t));
    wdata = alloc_zz_data(0);
    data_writer.write_double(&wdata, d1);
    data_reader.start_read(wdata);
    double d2 = data_reader.read_double(wdata);
    uint64_t di2 = 0;
    memcpy(&di2, &d2, sizeof(uint64_t));
    zzLog("d.1 = %llx, d.2 = %llx \n", di1, di2);
    free_zz_data(&wdata);
}
