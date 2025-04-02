#pragma once
#include <stdint.h>
#include <stdarg.h>
void init_serial();
void serial_writeb(uint8_t port, uint8_t data);
void serial_writereg(uint8_t port, uint8_t reg, uint8_t data);
uint8_t serial_readreg(uint8_t port, uint8_t reg);
uint8_t serial_readb(uint8_t port);
void debug_puts(char *string);
void printf(char *string, ...);
void vprintf(char *string, va_list vars);