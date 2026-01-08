/*int print(const char* words){
    unsigned int i = 0;
    while (words[i] != '\0') {  // 逐个字符检查直到\0
        i++;
    }
    unsigned short* NextPrintMemory;
    NextPrintMemory = (unsigned short*)0xB8000;
    /*NextPrintMemory = (unsigned short*)0xB8000;
    while(*NextPrintMemory != 0){
        NextPrintMemory++;
    }*//*
    for(unsigned a = 0; a < i; a++,NextPrintMemory++) {
    *NextPrintMemory = (unsigned short)words[a] | (0x0F << 8);
    }
    return 0;
}*/
int print(const char* words) {
    /*volatile unsigned short* vga = (volatile unsigned short*)0xB8000;
    
    for (int i = 0; words[i] != '\0' && i < 80*25; i++) {
        // 方法A：使用位或（推荐）
        vga[i] = ((unsigned short)words[i]) | (0x0F << 8);
        
        // 方法B：或这样写
        // vga[i] = ((unsigned short)words[i]) + 0x0F00;
    }
    
    return 0;*/
    volatile unsigned short* screen = (volatile unsigned short*)0xB8000;
    
    // 2. 写几个确定的位置
    screen[0] = 0x0F48;  // 'H' + 白字黑底
    screen[1] = 0x0F65;  // 'e'
    screen[2] = 0x0F6C;  // 'l'
    screen[3] = 0x0F6C;  // 'l'
    screen[4] = 0x0F6F;  // 'o'
}