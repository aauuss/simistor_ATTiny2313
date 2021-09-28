/*
 * Cамая лучшая библиотека для HCMS2903
 */


#ifndef F_CPU
    #define F_CPU 8000000UL
#endif

#define CE      PB0
#define RST     PB1
#define D_IN    PB2
#define RS      PB3
#define CLK     PB4

#include <util/delay.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "font5x7.h"

uint8_t dotRegister[20];

void HCMS29_init();
void SiftOut(uint8_t bitOrder, uint8_t data) ;
void LoadControlRegister(uint8_t data);
void LoadAllControlRegisters(uint8_t data);
void LoadDotRegister();
void SetBrightness(uint8_t brightness);
void PrintString(char* str, uint8_t position);
void PrintChar(char str, uint8_t position);


