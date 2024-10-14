void kmain(){
    unsigned short *tm_ptr = (unsigned short*)(0xb8000);
    tm_ptr[0] = 0xf41;
    return;
}