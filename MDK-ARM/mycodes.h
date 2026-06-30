#ifndef __MYCODES_H__
#define __MYCODES_H__

#include <stdint.h>

void mycodes(void);
void LCD_Init(void);
void LCD_SendCommand(uint8_t cmd);
void LCD_SendData(uint8_t data);
void LCD_SendString(char *str);

void delay_us(uint16_t us);
void Set_Pin_Output(void);
void Set_Pin_Input(void);
uint8_t DHT11_Read(uint8_t *Temp, uint8_t *Hum);

#endif