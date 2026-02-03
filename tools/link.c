//这个工具是这个系统的可执行文件编译完后需要它进行链接才能运行
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

// 可执行文件头部结构（16字节）
#pragma pack(push, 1)
struct ExecHeader {
    char     magic[6];      // "XCJUER"
    uint16_t version;       // 版本号 0x0100 = 1.0
    uint32_t code_offset;   // 代码段偏移
    uint32_t code_size;     // 代码段大小
    uint8_t  permissions;   // RWEA权限位
    uint8_t  reserved;      // 保留字节
};
#pragma pack(pop)

// 权限位定义
#define PERM_READ   0x08    // 1000b - 读权限
#define PERM_WRITE  0x04    // 0100b - 写权限  
#define PERM_EXEC   0x02    // 0010b - 执行权限
#define PERM_ADMIN  0x01    // 0001b - 需要管理员权限
#define PERM_ALL    0x0F    // 1111b - 所有权限

// 将权限字符串转换为位掩码
uint8_t parse_permission_string(const char *perm_str) {
    uint8_t perm = 0;
    
    for (int i = 0; perm_str[i] != '\0'; i++) {
        char c = tolower(perm_str[i]);
        switch (c) {
            case 'r': perm |= PERM_READ; break;
            case 'w': perm |= PERM_WRITE; break;
            case 'e': perm |= PERM_EXEC; break;
            case 'a': perm |= PERM_ADMIN; break;
            case '-': break; // 忽略连字符
            default:
                fprintf(stderr, "警告: 未知权限字符 '%c'，忽略\n", c);
                break;
        }
    }
    
    return perm;
}

// 打印权限详情
void print_permission_details(uint8_t perm) {
    printf("  权限位: 0x%02X (", perm);
    
    // 二进制显示
    for (int i = 7; i >= 0; i--) {
        if (i == 3) printf(" ");
        printf("%c", (perm & (1 << i)) ? '1' : '0');
    }
    printf("b)\n");
    
    printf("  权限说明: [");
    printf("%c", (perm & PERM_READ)  ? 'R' : '-');
    printf("%c", (perm & PERM_WRITE) ? 'W' : '-');
    printf("%c", (perm & PERM_EXEC)  ? 'E' : '-');
    printf("%c", (perm & PERM_ADMIN) ? 'A' : '-');
    printf("]\n");
    
    // 权限解释
    printf("  具体含义:\n");
    printf("    - 可读取: %s\n", (perm & PERM_READ) ? "是" : "否");
    printf("    - 可写入: %s\n", (perm & PERM_WRITE) ? "是" : "否");
    printf("    - 可执行: %s\n", (perm & PERM_EXEC) ? "是" : "否");
    printf("    - 需要管理员: %s\n", (perm & PERM_ADMIN) ? "是" : "否");
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("用法: %s <输入文件> <输出文件> <权限>\n", argv[0]);
        printf("\n权限格式:\n");
        printf("  1. 字符串格式: 包含字母 r,w,e,a 的组合\n");
        printf("     例如: \"rwe\", \"re\", \"rwa\", \"ea\"\n");
        printf("     特殊值: \"all\" = rwea, \"none\" = 无权限\n");
        printf("  2. 十六进制: 0x01-0x0F\n");
        printf("     例如: 0x0F = rwea, 0x07 = rwe, 0x05 = rwa\n");
        printf("  3. 十进制: 1-15\n");
        printf("\n示例:\n");
        printf("  %s kernel.bin kernel.xcj rwe\n", argv[0]);
        printf("  %s kernel.bin kernel.xcj 0x0F\n", argv[0]);
        printf("  %s kernel.bin kernel.xcj all\n", argv[0]);
        return 1;
    }
    
    // 1. 读取原始二进制文件
    FILE *in = fopen(argv[1], "rb");
    if (!in) {
        perror("打开输入文件失败");
        return 1;
    }
    
    fseek(in, 0, SEEK_END);
    long file_size = ftell(in);
    fseek(in, 0, SEEK_SET);
    
    if (file_size == 0) {
        fprintf(stderr, "错误: 输入文件为空\n");
        fclose(in);
        return 1;
    }
    
    unsigned char *data = malloc(file_size);
    if (!data) {
        perror("内存分配失败");
        fclose(in);
        return 1;
    }
    
    size_t bytes_read = fread(data, 1, file_size, in);
    fclose(in);
    
    if (bytes_read != file_size) {
        fprintf(stderr, "错误: 读取文件不完整\n");
        free(data);
        return 1;
    }
    
    // 2. 解析权限参数
    char *perm_str = argv[3];
    uint8_t permissions;
    
    // 检查特殊字符串
    if (strcmp(perm_str, "all") == 0 || strcmp(perm_str, "ALL") == 0) {
        permissions = PERM_ALL;
    } else if (strcmp(perm_str, "none") == 0 || strcmp(perm_str, "NONE") == 0) {
        permissions = 0;
    } 
    // 检查是否以0x开头的十六进制
    else if (strncmp(perm_str, "0x", 2) == 0) {
        permissions = (uint8_t)strtol(perm_str, NULL, 16);
        // 确保权限位在有效范围内
        if (permissions > PERM_ALL) {
            fprintf(stderr, "警告: 权限值 0x%02X 超出范围，将截断为 0x%02X\n", 
                    permissions, permissions & PERM_ALL);
            permissions &= PERM_ALL;
        }
    }
    // 检查是否全数字（十进制）
    else if (isdigit(perm_str[0])) {
        int dec_value = atoi(perm_str);
        if (dec_value < 0 || dec_value > PERM_ALL) {
            fprintf(stderr, "错误: 权限值 %d 超出范围 (0-15)\n", dec_value);
            free(data);
            return 1;
        }
        permissions = (uint8_t)dec_value;
    }
    // 否则当作字符串解析
    else {
        permissions = parse_permission_string(perm_str);
    }
    
    // 3. 创建头部
    struct ExecHeader header;
    
    // 设置魔数
    memcpy(header.magic, "XCJUER", 6);
    
    // 版本号：主版本1，次版本0
    header.version = 0x0100;
    
    // 代码偏移：头部之后立即开始
    header.code_offset = sizeof(struct ExecHeader);
    
    // 代码大小：整个文件内容
    header.code_size = (uint32_t)file_size;
    
    // 权限
    header.permissions = permissions;
    
    // 保留字节
    header.reserved = 0;
    
    // 4. 创建输出文件
    FILE *out = fopen(argv[2], "wb");
    if (!out) {
        perror("创建输出文件失败");
        free(data);
        return 1;
    }
    
    // 写入头部
    if (fwrite(&header, sizeof(header), 1, out) != 1) {
        perror("写入头部失败");
        fclose(out);
        free(data);
        return 1;
    }
    
    // 写入原始二进制数据
    if (fwrite(data, 1, file_size, out) != file_size) {
        perror("写入数据失败");
        fclose(out);
        free(data);
        return 1;
    }
    
    // 5. 清理
    fclose(out);
    free(data);
    
    // 6. 输出结果
    printf("成功创建可执行文件: %s\n", argv[2]);
    printf("头部信息:\n");
    printf("  魔数: %.6s\n", header.magic);
    printf("  版本: %d.%d\n", 
           (header.version >> 8) & 0xFF,
           header.version & 0xFF);
    printf("  代码偏移: 0x%08X (%u 字节)\n", 
           header.code_offset, header.code_offset);
    printf("  代码大小: 0x%08X (%u 字节)\n", 
           header.code_size, header.code_size);
    
    print_permission_details(header.permissions);
    printf("  文件总大小: %lu 字节\n", sizeof(header) + file_size);
    
    return 0;
}