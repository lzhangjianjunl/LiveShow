

#include "zz_file.h"
#include "zz_utils.h"
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

extern int8_t zz_is_file_exist(const char *file_path){
    FILE *file = fopen(file_path, "r");
    if(file == NULL){
        if (errno == ENOENT) {
            return 0;
        }else{
            zzLog("file open error errno=%d", errno);
            return -1;
        }
    }
    fclose(file);
    return 1;
}

extern size_t zz_file_size(const char *file_path){
    FILE * file = fopen(file_path, "r");
    if (file) {
        fseek(file, 0, SEEK_END);
        size_t file_size = ftell(file);
        fclose(file);
        return file_size;
    }
    return 0;
}

extern int8_t zz_remove_file(const char *file_path){
    if (zz_is_file_exist(file_path)) {
        return (int8_t)remove(file_path) == 0;
    }
    return 1;
}

extern int8_t zz_remove_dir(const char *file_dir){
    DIR *dp = NULL;
    struct dirent *entry = NULL;
    struct stat stat;
    if ((dp = opendir(file_dir)) == NULL) {
        return 0;
    }
    chdir (file_dir);
    while ((entry = readdir(dp)) != NULL)
    {
        lstat(entry->d_name, &stat);
        if (S_ISREG(stat.st_mode))
        {
            remove(entry->d_name);
        }
    }
    
    return 1;
}

extern zz_data *zz_read_data_from_file(const char *file_path){
    FILE * file = fopen(file_path, "r+");
    if (file) {
        size_t file_size = zz_file_size(file_path);
        zz_data *mp4_data = alloc_zz_data((int)file_size);
        size_t read_item_count = fread(mp4_data->data, file_size, 1, file);
        fclose(file);
        if (!read_item_count) {
            free_zz_data(&mp4_data);
            return NULL;
        }
        return mp4_data;
    }
    return NULL;
}

extern int8_t zz_write_data_to_file(const char *file_path, zz_data *data){
    FILE *file = fopen(file_path, "w");
    if (file) {
        size_t file_size = data->size;
        size_t write_item_count = fwrite(data->data, file_size, 1, file);
        fflush(file);
        fclose(file);
        return write_item_count != 0;
    }
    return 0;
}

extern void zz_test_file(const char *dir){
    char *short_name = "/tmp.t";
    size_t file_name_size = strlen(dir) + strlen(short_name) + 1;
    char *file_name = zz_alloc(file_name_size);
    memset(file_name, 0, file_name_size);
    memcpy(file_name, dir, strlen(dir));
    memcpy(file_name + strlen(dir), short_name, strlen(short_name));
    
    //文件是否存在
    zzLog("1 file %s is exist(%d)", file_name, zz_is_file_exist(file_name));
    
    //文件写入
    zz_data *zzdata = alloc_zz_data(0);
    char *datastr = "nihao hahaha 你好";
    memcpy_zz_data(&zzdata, datastr, (int32_t)strlen(datastr) + 1);
    
    if(zz_write_data_to_file(file_name, zzdata)){
        zzLog("文件写入成功");
    }else{
        zzLog("文件写入失败");
    }
    free_zz_data(&zzdata);
    //文件是否存在
    zzLog("2 file %s is exist(%d)", file_name, zz_is_file_exist(file_name));
    zzLog("3 file %s size(%ld)", file_name, zz_file_size(file_name));
    
    //文件读取
    zz_data *readdata = zz_read_data_from_file(file_name);
    if (readdata) {
        zzLog("文件读取成功 %s", readdata->data);
        free_zz_data(&readdata);
    }else{
        zzLog("文件读取失败");
    }
    
    //文件移除
    if(zz_remove_file(file_name)){
        zzLog("文件移除成功");
    }else{
        zzLog("文件移除失败");
    }
    
}
