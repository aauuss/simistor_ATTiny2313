/*  Вывод символов из талбицы ASCII 
 *  реализован криво в целях
 *  экономии места для ATTiny2313.
 *  Для человеческой реализации процедуру 
 *  PrintChar надо исправить.
 */

#ifndef F_CPU
    #define F_CPU 8000000UL
#endif


/*
#include <util/delay.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
*/
#include "HCMS2903_lib.h"


void HCMS29_init(){
  DDRB |= (1 << CE) | (1 << RST) | (1 << D_IN) | (1 << RS) | (1 << CLK);    //ноги от дисплея - выход
  PORTB &= ~((1 << RST) | (1 << D_IN));  
  _delay_us(10);
  PORTB |= (1 << RST) | (1 << CE) | (1 << RS) | (1 << CLK);    
  for (uint8_t i = 0; i < 20; i++){
    dotRegister[i] = 0x00;
  }
}

void SiftOut(uint8_t bitOrder, uint8_t data) { //bitOrder 1-direct, 0-reverse
  PORTB &= ~(1 << CLK);
  for (uint8_t i = 0; i < 8; i++){
    if (bitOrder) {
      (data & (1 << i))?(PORTB |= (1 << D_IN)):(PORTB &= ~(1 << D_IN));
    }    
    else {
      (data & (1 << (7 - i)))?(PORTB |= (1 << D_IN)):(PORTB &= ~(1 << D_IN));
    }
    _delay_us(10);
    PORTB |= (1 << CLK);
    _delay_us(10);
    PORTB &= ~(1 << CLK);
  }
}

void LoadControlRegister(uint8_t data) {
  PORTB |= (1 << RS);
  _delay_us(10);
  PORTB &= ~(1 << CE);
  _delay_us(10);
  SiftOut(1, data);
  PORTB |= (1 << CE);
}

void LoadAllControlRegisters(uint8_t data){
  LoadControlRegister(0x81); //0b10000001
  LoadControlRegister(data);
}

void LoadDotRegister() {
  PORTB &= ~(1 << RS);
  _delay_us(10);
  PORTB &= ~(1 << CE);
  for (uint8_t i = 0; i < 20; i++){
    SiftOut(0, dotRegister[i]);
  }
  PORTB |= (1 << CE);  
}

void SetBrightness(uint8_t brightness) {
  if (brightness > 15) {brightness = 15;}
  LoadAllControlRegisters(0x70 | brightness);  // 0x70 - 0b01110000 последние 4 бита яркость, можно 0x40 - 0b01000000
}

void PrintChar(char str, uint8_t position){
  if (position < 4){
    for (uint8_t i = 0; i < 5; i++){
      if ((str >= 48) && (str <= 57)){          //0..9
        dotRegister[i + position*5] = pgm_read_byte(&Font5x7[(str - 48)*5 + i]);
      } else if (str == 37) {                   //%
        dotRegister[i + position*5] = pgm_read_byte(&Font5x7[50 + i]);
      } else if (str == 38) {                   //% (без нижней точки)
        dotRegister[i + position*5] = pgm_read_byte(&Font5x7[55 + i]);
      } else if (str == 45) {                   //-
        dotRegister[i + position*5] = pgm_read_byte(&Font5x7[60 + i]);
      } else {
        dotRegister[i + position*5] = 0x00;
      }
    }
  }
  LoadDotRegister();
}

void PrintString(char* str, uint8_t position){
  uint8_t ch = 0;
  if (position < 4) {
    for (uint8_t i = position; i < 4; i++){
      PrintChar(str[ch++], i);
    }

  }
}  

