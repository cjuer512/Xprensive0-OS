#include "driver.h"
#include "driverp.h"
#include "stdint.h"

int cmpstr_inifilesystem(char* start, char* targetword, int id) {
    for(int i = 0; i < id; i++) {
        if(start[i] != targetword[i]) {
            return 0;
        }
    }
    return 1;
}

int len_infilesystem(const char *str) {
    int count = 0;
    while (str[count] != '\0') {
        count++;
    }
    return count;
}

int FindInitalCjuerfilesystem() {
    // 假设 0x13000 处已经有数据
    char* position = (char*)0x20000;
    
    // 修正：文件系统标识应该是11字节的 "\xEB\x76\x90" "CJUER   "
    // 但实际字符串 "CJUER   " 只有8字节，加上3字节的魔数 = 11字节
    for(int i = 0; i < 100; i++) {
        int flag = cmpstr_inifilesystem(position, "xb1cjuer",8);
        if(flag == 1) {
            return i;  // 返回找到的位置索引
        }
    }
    return -1;  // 未找到
}

int locatefirsttable(char* buffer) {
    // 修正：确保正确获取表位置
    // buffer[11] 是一个 char，需要转换为 int
    if (buffer[11] < 0) {
        return 256 + (int)buffer[11];  // 处理有符号 char
    }
    return (int)buffer[11];
}

uint32_t getlocatefile(int table, uint32_t file_id, int mode, char* buffer) {
    if (mode == 1 && buffer != NULL) {
        // 转换为uint32_t指针，便于按4字节单位操作
        uint32_t* data = (uint32_t*)buffer;
        
        // 计算总条目数（每8字节一个条目 = 2个uint32_t）
        int entry_count = 512 / 8;
        
        for (int i = 0; i < entry_count; i++) {
            // 计算当前条目的起始位置
            uint32_t* entry_start = &data[i * 2];  // 每个条目占2个uint32_t
            
            // 获取文件ID（条目的第一个uint32_t）
            uint32_t current_id = entry_start[0];
            
            // 比较文件ID
            if (current_id == file_id) {
                // 返回簇号（条目的第二个uint32_t）
                uint32_t cluster_number = entry_start[1];
                return cluster_number;
            }
        }
    }
    return -1;  // 未找到或参数无效
}