extern void keyboard_init(unsigned char* idt);
extern void keyboard_handler(void);
extern void hdd_init_all(uint8_t* idt);
extern int hdd_read(uint32_t lba, uint8_t count, uint16_t* buffer);
extern int hdd_read_simple(uint32_t lba, uint16_t* buffer);