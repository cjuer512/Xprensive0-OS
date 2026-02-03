//这是一个帮助把文件写入虚拟磁盘的工具，硬盘中表的内容需要自行配置
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#define CLUSTER_SIZE 64
#define FIRST_GROUP_SIZE 52
#define SUBSEQUENT_GROUP_SIZE 60
#define FILENAME_SIZE 8
#define NEXT_CLUSTER_PTR_SIZE 4

// 函数声明
int write_to_virtual_disk(const char* filename, const char* raw_disk_path, uint32_t start_cluster);
int read_from_virtual_disk(const char* filename, const char* raw_disk_path, uint32_t start_cluster);
void extract_filename(const char* path, char* filename);
uint32_t get_file_size(const char* filename);

int main(int argc, char *argv[])
{
    // 参数数量检查：至少需要3个参数（程序名、指令、文件名）
    if (argc < 3)
    {
        fprintf(stderr, "Usage: %s <read/write> <filename> [raw_disk_path] [start_cluster]\n", argv[0]);
        fprintf(stderr, "For write: %s write source.txt virtual_disk.img 0\n", argv[0]);
        fprintf(stderr, "For read: %s read file_in_disk.txt virtual_disk.img 0\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (strcmp(argv[1], "read") == 0)
    {
        if (argc < 5) {
            fprintf(stderr, "For read: %s read file_in_disk.txt virtual_disk.img start_cluster\n", argv[0]);
            return EXIT_FAILURE;
        }
        
        uint32_t start_cluster = (uint32_t)atoi(argv[4]);
        if (read_from_virtual_disk(argv[2], argv[3], start_cluster) != 0) {
            fprintf(stderr, "Failed to read from virtual disk\n");
            return EXIT_FAILURE;
        }
        
        printf("File read successfully from virtual disk\n");
        return EXIT_SUCCESS;
    }
    else if (strcmp(argv[1], "write") == 0)
    {
        if (argc < 5) {
            fprintf(stderr, "For write: %s write source.txt virtual_disk.img start_cluster\n", argv[0]);
            return EXIT_FAILURE;
        }
        
        uint32_t start_cluster = (uint32_t)atoi(argv[4]);
        if (write_to_virtual_disk(argv[2], argv[3], start_cluster) != 0) {
            fprintf(stderr, "Failed to write to virtual disk\n");
            return EXIT_FAILURE;
        }
        
        printf("File written successfully to virtual disk\n");
        return EXIT_SUCCESS;
    }
    else
    {
        fprintf(stderr, "Unknown command: %s\n", argv[1]);
        fprintf(stderr, "Available commands: read, write\n");
        return EXIT_FAILURE;
    }
}

// 从文件路径中提取文件名（最多8个字符）
void extract_filename(const char* path, char* filename) {
    const char* last_slash = strrchr(path, '/');
    const char* last_backslash = strrchr(path, '\\');
    const char* name_start = path;
    
    if (last_slash != NULL && last_slash > name_start) {
        name_start = last_slash + 1;
    }
    if (last_backslash != NULL && last_backslash > name_start) {
        name_start = last_backslash + 1;
    }
    
    // 拷贝最多8个字符
    strncpy(filename, name_start, FILENAME_SIZE);
    filename[FILENAME_SIZE] = '\0'; // 确保字符串结束
    
    // 去除扩展名（如果有）
    char* dot = strchr(filename, '.');
    if (dot != NULL) {
        *dot = '\0';
    }
}

// 获取文件大小
uint32_t get_file_size(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        return 0;
    }
    
    fseek(file, 0, SEEK_END);
    uint32_t size = (uint32_t)ftell(file);
    fclose(file);
    
    return size;
}

// 写入虚拟硬盘的主要函数
int write_to_virtual_disk(const char* filename, const char* raw_disk_path, uint32_t start_cluster) {
    FILE* source_file = fopen(filename, "rb");
    if (source_file == NULL) {
        perror("Error opening source file for reading");
        return -1;
    }
    
    FILE* disk_file = fopen(raw_disk_path, "r+b");
    if (disk_file == NULL) {
        // 如果文件不存在，创建新文件
        disk_file = fopen(raw_disk_path, "w+b");
        if (disk_file == NULL) {
            perror("Error creating/opening virtual disk file");
            fclose(source_file);
            return -1;
        }
    }
    
    // 获取源文件大小
    fseek(source_file, 0, SEEK_END);
    uint32_t file_size = (uint32_t)ftell(source_file);
    fseek(source_file, 0, SEEK_SET);
    
    if (file_size == 0) {
        fprintf(stderr, "Source file is empty\n");
        fclose(source_file);
        fclose(disk_file);
        return -1;
    }
    
    // 提取文件名（最多8个字符）
    char disk_filename[FILENAME_SIZE + 1];
    extract_filename(filename, disk_filename);
    
    printf("Writing file '%s' (size: %u bytes) to virtual disk at cluster %u\n", 
           disk_filename, file_size, start_cluster);
    
    uint32_t current_cluster = start_cluster;
    uint32_t next_cluster = start_cluster + 1;
    uint32_t bytes_remaining = file_size;
    uint32_t bytes_written_total = 0;
    int is_first_cluster = 1;
    
    unsigned char cluster_buffer[CLUSTER_SIZE];
    
    while (bytes_remaining > 0) {
        // 清空簇缓冲区
        memset(cluster_buffer, 0, CLUSTER_SIZE);
        
        if (is_first_cluster) {
            // 第一簇：写入文件名（8字节）
            memcpy(cluster_buffer, disk_filename, strlen(disk_filename));
            
            // 读取第一组数据（52字节）
            uint32_t bytes_to_read = (bytes_remaining < FIRST_GROUP_SIZE) ? bytes_remaining : FIRST_GROUP_SIZE;
            uint32_t bytes_read = (uint32_t)fread(cluster_buffer + FILENAME_SIZE, 1, bytes_to_read, source_file);
            
            if (bytes_read == 0) {
                fprintf(stderr, "Error reading from source file\n");
                fclose(source_file);
                fclose(disk_file);
                return -1;
            }
            
            bytes_remaining -= bytes_read;
            bytes_written_total += bytes_read;
            
            // 计算下一个簇号（文件未读完时需要继续）
            next_cluster = (bytes_remaining > 0) ? current_cluster + 1 : 0xFFFFFFFF;
            
            // 写入下一个簇指针（最后4字节）
            memcpy(cluster_buffer + CLUSTER_SIZE - NEXT_CLUSTER_PTR_SIZE, &next_cluster, NEXT_CLUSTER_PTR_SIZE);
            
            is_first_cluster = 0;
        } else {
            // 后续簇：直接写入数据（60字节）
            uint32_t bytes_to_read = (bytes_remaining < SUBSEQUENT_GROUP_SIZE) ? bytes_remaining : SUBSEQUENT_GROUP_SIZE;
            uint32_t bytes_read = (uint32_t)fread(cluster_buffer, 1, bytes_to_read, source_file);
            
            if (bytes_read == 0) {
                fprintf(stderr, "Error reading from source file\n");
                fclose(source_file);
                fclose(disk_file);
                return -1;
            }
            
            bytes_remaining -= bytes_read;
            bytes_written_total += bytes_read;
            
            // 计算下一个簇号（文件未读完时需要继续）
            next_cluster = (bytes_remaining > 0) ? current_cluster + 1 : 0xFFFFFFFF;
            
            // 写入下一个簇指针（最后4字节）
            memcpy(cluster_buffer + CLUSTER_SIZE - NEXT_CLUSTER_PTR_SIZE, &next_cluster, NEXT_CLUSTER_PTR_SIZE);
        }
        
        // 定位到当前簇的位置并写入数据
        fseek(disk_file, current_cluster * CLUSTER_SIZE, SEEK_SET);
        uint32_t bytes_written = (uint32_t)fwrite(cluster_buffer, 1, CLUSTER_SIZE, disk_file);
        
        if (bytes_written != CLUSTER_SIZE) {
            fprintf(stderr, "Error writing to virtual disk at cluster %u\n", current_cluster);
            fclose(source_file);
            fclose(disk_file);
            return -1;
        }
        
        printf("Written cluster %u, next cluster: %u, progress: %u/%u bytes (%.1f%%)\n",
               current_cluster, next_cluster, bytes_written_total, file_size,
               (float)bytes_written_total / file_size * 100);
        
        current_cluster++;
    }
    
    fclose(source_file);
    fclose(disk_file);
    return 0;
}

// 从虚拟硬盘读取文件（可选功能）
int read_from_virtual_disk(const char* filename, const char* raw_disk_path, uint32_t start_cluster) {
    FILE* disk_file = fopen(raw_disk_path, "rb");
    if (disk_file == NULL) {
        perror("Error opening virtual disk file for reading");
        return -1;
    }
    
    FILE* output_file = fopen(filename, "wb");
    if (output_file == NULL) {
        perror("Error creating output file");
        fclose(disk_file);
        return -1;
    }
    
    uint32_t current_cluster = start_cluster;
    unsigned char cluster_buffer[CLUSTER_SIZE];
    uint32_t total_bytes_read = 0;
    int is_first_cluster = 1;
    
    printf("Reading file from virtual disk starting at cluster %u\n", start_cluster);
    
    while (current_cluster != 0xFFFFFFFF) {
        // 定位到当前簇并读取数据
        fseek(disk_file, current_cluster * CLUSTER_SIZE, SEEK_SET);
        uint32_t bytes_read = (uint32_t)fread(cluster_buffer, 1, CLUSTER_SIZE, disk_file);
        
        if (bytes_read != CLUSTER_SIZE) {
            fprintf(stderr, "Error reading from virtual disk at cluster %u\n", current_cluster);
            fclose(disk_file);
            fclose(output_file);
            return -1;
        }
        
        if (is_first_cluster) {
            // 第一簇：跳过文件名（8字节），读取52字节数据
            uint32_t data_bytes = CLUSTER_SIZE - FILENAME_SIZE - NEXT_CLUSTER_PTR_SIZE;
            fwrite(cluster_buffer + FILENAME_SIZE, 1, data_bytes, output_file);
            total_bytes_read += data_bytes;
            is_first_cluster = 0;
        } else {
            // 后续簇：读取60字节数据
            uint32_t data_bytes = CLUSTER_SIZE - NEXT_CLUSTER_PTR_SIZE;
            fwrite(cluster_buffer, 1, data_bytes, output_file);
            total_bytes_read += data_bytes;
        }
        
        // 读取下一个簇指针
        memcpy(&current_cluster, cluster_buffer + CLUSTER_SIZE - NEXT_CLUSTER_PTR_SIZE, NEXT_CLUSTER_PTR_SIZE);
        
        printf("Read cluster %u, next cluster: %u, total bytes read: %u\n",
               current_cluster, (current_cluster == 0xFFFFFFFF) ? 0 : current_cluster, total_bytes_read);
        
        if (current_cluster == 0xFFFFFFFF) {
            break; // 文件结束
        }
    }
    
    fclose(disk_file);
    fclose(output_file);
    printf("File extraction completed. Total bytes: %u\n", total_bytes_read);
    return 0;
}