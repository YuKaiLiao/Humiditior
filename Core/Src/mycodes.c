#include "main.h"
#include "mycodes.h"
#include "stm32l0xx_hal.h"

void mycodes(void){

//  while(1){
//  HAL_GPIO_WritePin( GPIOA, GPIO_PIN_5, GPIO_PIN_SET);    //control PA5 output High
//  HAL_Delay(2000);                                        // wait 1s
//  HAL_GPIO_WritePin( GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);  //control PA5 output Low
//  HAL_Delay(1000);                                        // wait 1s
//  }
  
  while(1){
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);	//PA5 High
		HAL_Delay(500);// Delay 500ms
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);//PA5 Low
		HAL_Delay(500);// Delay 500ms
	}
}
