#ifndef F_CPU
    #define F_CPU 8000000UL
#endif

#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>
//#include <util/atomic.h>
//#include <math.h>
//#include "font5x7.h"
#include "HCMS2903_lib.h"
#include "EEPROM_lib.h"

#define TEH     PD6

#define ENC_A   PD4
#define ENC_B   PD2
#define ENC_SW  PD3

#define INT1_RISING MCUCR & (1 << ISC10)
#define ENCODER_PRESSED PIND & (1 << ENC_SW)

#define TEH_PWM_PERIOD 1000



uint32_t    millis = 0,
            TEH_PWM_lastTime = 0;    
uint8_t     encPressed = 0;  
uint8_t     op = 0,
            mode = 1,   //1 - работа, 0 - выключен
            sec = 0;

//===================================ПРЕРЫВАНИЯ================================================
ISR(INT0_vect) {
  MCUCR |= (1 << ISC10);
  if (PIND & (1 << ENC_A)) {
    if (op < 95) {op = ENCODER_PRESSED ? op + 5 : op + 1;}
    else if (op < 100) {op = ENCODER_PRESSED ? 100 : op + 1;}
  } else {
    if (op > 5) {op = ENCODER_PRESSED ? op - 5 : op - 1;}
    else if (op > 0) {op = ENCODER_PRESSED ? 0 : op - 1;}
  }
  mode = op > 0 ? 1 : 0;
  _delay_us(100);
}

ISR(INT1_vect) {
  if (INT1_RISING){             //если прерывание сработало по переднему фронту и 
    MCUCR &= ~(1 << ISC10);     //перенастроим на задний фронт
  } else {
    MCUCR |= (1 << ISC10);    
    if (mode) {
      EEPROM_write(0, (op & 0xFF));
      op = 0;  
      mode = 0;
    } else {
      op = EEPROM_read(0);
      mode = 1;
    }       
  }
  _delay_us(100);
}

ISR(TIMER0_COMPA_vect){
  millis++;
  if ((millis & (0x2FF)) == 0x200) { sec ++;}
}


//===================================НАСТРОЙКИ==================================================
void setup(void) { 
  cli();

    //настройка ног
  DDRD &= ~((1 << ENC_A) | (1 << ENC_B) | (1 << ENC_SW)); //ноги от энкодера на вход
  DDRD |= (1 << TEH);  

    //настройка INT0, INT1
  GIMSK |= (1 << INT0) | (1 << INT1); //вкл. внешнее прерывание INT0, INT1
  MCUCR |= (1 << ISC01) | (1 << ISC00) | (1 << ISC11) | (1 << ISC10); //прерывание по переднему фронту
    
    //настройка таймера
  TCCR0A |= (1 << WGM01);
  TCCR0B |= (1 << CS01) | (1 << CS00);
  OCR0A = 0x7F;
  TIMSK |= (1 << OCIE0A);

    //настройка watchdog'а
  WDTCSR |= (1 << WDE) | (1 << WDP2) | (1 << WDP1);
  
    //включаем прерывания
  sei(); 

}

//==================================ПРОЦЕДУРЫ==================================================
void DisplayPrint(uint16_t num){
    char displayString[4];
    char lastSymbol;
    lastSymbol = (sec & 0x01) ? '&' : '%';  //символы не соответствуют таблице ASCII для экономии места
    itoa(num, displayString, 10);
    if (num < 10){   displayString[2] = displayString[0];
                    displayString[3] = lastSymbol;
                    displayString[0] = ' ';
                    displayString[1] = ' ';}
    else if (num < 100) { displayString[2] = displayString[1];
                         displayString[1] = displayString[0];
                         displayString[0] = ' ';
                         displayString[3] = lastSymbol;}
    else { displayString[3] = lastSymbol;}    
    PrintString(displayString, 0);    
}

void SetOut(int _output){
    if (_output <= 0) {
        PORTD &= ~(1 << TEH);
    } 
    else if (_output >= 100) {
        PORTD |= (1 << TEH);
    } 
    else {            
      if (millis < (TEH_PWM_lastTime + (TEH_PWM_PERIOD*_output/100))) {
        PORTD |= (1 << TEH);
      }
      else {
        PORTD &= ~(1 << TEH);
      }
    }
    if (millis > (TEH_PWM_lastTime + TEH_PWM_PERIOD)){
        TEH_PWM_lastTime = millis;
    }
}


//===================================ГЛАВНАЯ ПРОГРАММА==========================================
void main(void) {
    setup();
    _delay_us(500);
    HCMS29_init();
    SetBrightness(0x0A);
    
    while (1) {
        DisplayPrint(op);
        SetOut(op);
        asm volatile ("WDR");
    }
}






