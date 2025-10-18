#!/usr/bin/env python3
import os
import sys

def create_bootable_img(bin_file, img_file, img_size_mb=1.44):
    """
    创建可启动的磁盘镜像
    
    Args:
        bin_file: 输入的bin文件路径
        img_file: 输出的img文件路径  
        img_size_mb: 镜像大小(MB)，默认1.44MB(软盘)
    """
    
    # 检查输入文件是否存在
    if not os.path.exists(bin_file):
        print(f"错误: 找不到文件 {bin_file}")
        return False
    
    # 读取引导程序
    try:
        with open(bin_file, 'rb') as f:
            boot_data = f.read()
    except Exception as e:
        print(f"读取文件错误: {e}")
        return False
    
    # 检查引导程序大小
    if len(boot_data) > 512:
        print("警告: 引导程序超过512字节，可能无法正常启动")
    
    # 计算镜像大小 (1.44MB = 1474560 bytes)
    if img_size_mb == 1.44:
        img_size = 1474560  # 标准软盘大小
    else:
        img_size = img_size_mb * 1024 * 1024
    
    print(f"创建 {img_size_mb}MB 磁盘镜像...")
    
    try:
        # 创建镜像文件并填充0
        with open(img_file, 'wb') as f:
            # 写入全0的镜像
            f.write(b'\x00' * img_size)
        
        # 将引导程序写入镜像开头
        with open(img_file, 'r+b') as f:
            f.write(boot_data)
            
            # 如果是标准软盘大小，添加引导签名
            if img_size_mb == 1.44 and len(boot_data) <= 510:
                f.seek(510)
                f.write(b'\x55\xaa')  # 引导扇区结束标志
        
        print(f"成功创建镜像: {img_file}")
        print(f"镜像大小: {os.path.getsize(img_file)} 字节")
        return True
        
    except Exception as e:
        print(f"创建镜像错误: {e}")
        return False

def main():
    if len(sys.argv) < 2:
        print("用法: python create_img.py <bin文件> [输出img文件] [大小MB]")
        print("示例: python create_img.py boot.bin")
        print("示例: python create_img.py boot.bin myos.img 2")
        return
    
    bin_file = sys.argv[1]
    
    # 设置输出文件名
    if len(sys.argv) > 2:
        img_file = sys.argv[2]
    else:
        img_file = os.path.splitext(bin_file)[0] + '.img'
    
    # 设置镜像大小
    if len(sys.argv) > 3:
        try:
            img_size = float(sys.argv[3])
        except ValueError:
            print("错误: 大小参数必须是数字")
            return
    else:
        img_size = 1.44  # 默认软盘大小
    
    create_bootable_img(bin_file, img_file, img_size)

if __name__ == "__main__":
    main()
