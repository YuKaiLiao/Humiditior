#include "mycodes.h"
#include "main.h"
#include <stdio.h> // sprintf need it

// claim STM32 hardware handle(claim in main.c)
extern I2C_HandleTypeDef hi2c1; 
extern TIM_HandleTypeDef htim2;

// PCF8574 default I2C address is 0x27, left moveon bit is 0x4E for HAL using
#define LCD_ADDR (0x27 << 1)

// send command or data
void LCD_SendCommand(uint8_t cmd) {
    uint8_t data_u, data_l;
    uint8_t data_t[4];
    
    data_u = (cmd & 0xf0);       // upper 4 byte
    data_l = ((cmd << 4) & 0xf0); // lower 4 byte
    
    // I2C must obey LCD's order (En must be rise then pull down, create one pulse)
    // then add 0x08 (background light up eternity) and 0x00 (RS=0, mean command)
    data_t[0] = data_u | 0x0C;  // En=1, RS=0
    data_t[1] = data_u | 0x08;  // En=0, RS=0
    data_t[2] = data_l | 0x0C;  // En=1, RS=0
    data_t[3] = data_l | 0x08;  // En=0, RS=0
    
    HAL_I2C_Master_Transmit(&hi2c1, LCD_ADDR, data_t, 4, 100);
}

void LCD_SendData(uint8_t data) {
    uint8_t data_u, data_l;
    uint8_t data_t[4];
    
    data_u = (data & 0xf0);
    data_l = ((data << 4) & 0xf0);
    
    // add 0x08 (background light up eternity) and 0x00 (RS=1, mean data)
    data_t[0] = data_u | 0x0D;  // En=1, RS=1
    data_t[1] = data_u | 0x09;  // En=0, RS=1
    data_t[2] = data_l | 0x0D;  // En=1, RS=1
    data_t[3] = data_l | 0x09;  // En=0, RS=1
    
    HAL_I2C_Master_Transmit(&hi2c1, LCD_ADDR, data_t, 4, 100);
}

// LCD init
void LCD_Init(void) {
    HAL_Delay(50); // wait for screen state
    LCD_SendCommand(0x30);
    HAL_Delay(5);
    LCD_SendCommand(0x30);
    HAL_Delay(1);
    LCD_SendCommand(0x32); // transform into 4-bit mode
    HAL_Delay(10);

    LCD_SendCommand(0x28); // 2 line, 5x8 aray
    HAL_Delay(1);
    LCD_SendCommand(0x0C); // turn on showing, turn off cursor
    HAL_Delay(1);
    LCD_SendCommand(0x01); // clear screen
    HAL_Delay(2);
}

// send string
void LCD_SendString(char *str) {
    while (*str) {
        LCD_SendData(*str++);
    }
}

void delay_us(uint16_t us)
{
    __HAL_TIM_SET_COUNTER(&htim2, 0);

    while (__HAL_TIM_GET_COUNTER(&htim2) < us);
}

void Set_Pin_Output(void) {
    __HAL_RCC_GPIOB_CLK_ENABLE(); // ?? ???? GPIOB ???
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_0;             // ?? ?? PB0
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;   
    GPIO_InitStruct.Pull = GPIO_PULLUP;          
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH; 
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);        // ?? GPIOB
}

void Set_Pin_Input(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_0;             // ?? ?? PB0
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;    
    GPIO_InitStruct.Pull = GPIO_PULLUP;          
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);        // ?? GPIOB
}

// ?? DHT11 ?????
uint8_t DHT11_Read(uint8_t *Temp, uint8_t *Hum) {
  
  
    uint8_t i, j, data[5] = {0};
    uint32_t retry = 0;
    uint8_t error_code = 0; // ????????,??????? LCD

    // 1. STM32 ??????
    Set_Pin_Output();
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET); 
    delay_us(20000); // ?? 18ms
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);   
    delay_us(30);   // ? 1ms ????
    
    // 2. ??????,??????
    Set_Pin_Input();

//while (1)
//{
//    LCD_SendCommand(0xC0);

//    if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0))
//        LCD_SendString("HIGH ");
//    else
//        LCD_SendString("LOW  ");

//    HAL_Delay(200);
//}

    // ?? DHT11 ???? (???? 40us ???,? 100 ??????)
    retry = 0;
    while (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == GPIO_PIN_SET) {
        delay_us(1);
        if (++retry > 2000) { error_code = 1; goto OVER; } // ????
    }
    
    // ?? DHT11 ???? (????? 80us)
    retry = 0;
    while (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == GPIO_PIN_RESET) {
        delay_us(1);
        if (++retry > 100) { error_code = 2; goto OVER; } // ????
    }
    
    // ?? DHT11 ???? (??????? 80us,???????)
    retry = 0;
    while (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == GPIO_PIN_SET) {
        delay_us(1);
        if (++retry > 100) { error_code = 3; goto OVER; } // ????
    }

    // 3. ???? 40 bits ??
    for (i = 0; i < 5; i++) {
        for (j = 0; j < 8; j++) {
            // ?? bit ???? 50us ????
            retry = 0;
            while (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == GPIO_PIN_RESET) {
                delay_us(1);
                if (++retry > 100) { error_code = 4; goto OVER; }
            }
            
            // ?????,?? 26-28us ?? '0',?? 70us ?? '1'
            delay_us(40); // ?? 40us ????????
            
            data[i] <<= 1;
            if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == GPIO_PIN_SET) {
                data[i] |= 1; // 40us ??????,??? bit ? 1
            }
            
            // ???? bit ??????
            retry = 0;
            while (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == GPIO_PIN_SET) {
                delay_us(1);
                if (++retry > 100) { error_code = 5; goto OVER; }
            }
        }
    }
    
OVER:
    // 4. ?????????????
    LCD_SendCommand(0xC0); // ?????????
    
    if (error_code != 0) {
        // ??????????????????
        if (error_code == 1) LCD_SendString("Err: No Response");
        if (error_code == 2) LCD_SendString("Err: Low Timeout");
        if (error_code == 3) LCD_SendString("Err: High Timeout");
        if (error_code == 4) LCD_SendString("Err: Data Low Timeout");
        if (error_code > 4) LCD_SendString("Err: Data High Timeout");
        return 0;
    }

    // ?????
    if (data[0] + data[1] + data[2] + data[3] == data[4]) {
        *Hum = data[0];  
        *Temp = data[2]; 
        LCD_SendString("Read Success! ");
        return 1; 
    } else {
        LCD_SendString("CheckSum Error");
        return 0;
    }
}

void DHT11_Lock_Test(void) {
    uint32_t timeout = 0;
    
    // 1. ?????:???????? LED(??? PC13)????
    // (?????????? LED ??,?????????)
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET); 

    // 2. ??????
    Set_Pin_Output();
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET); 
    delay_us(25000); // ?? 25ms
    
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
  
  LCD_SendCommand(0xC0);

//if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0))
//    LCD_SendString("After SET HIGH ");
//else
//    LCD_SendString("After SET LOW  ");
    // ????????? delay_us,?????????
//    for(volatile int delay = 0; delay < 200; delay++); 
    delay_us(25000); // ?? 25ms

    // 3. ??????
    Set_Pin_Input();

while (1)
{
    if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0))
        LCD_SendString("H");
    else
        LCD_SendString("L");

    HAL_Delay(500);
}

    // 4. ????:???????????????????
    // ???? 100,000 ??????(????????)
    while (1)
    {
      if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == GPIO_PIN_RESET)
        {
            LCD_SendCommand(0xC0);
            LCD_SendString("LOW DETECTED");
            while(1);
        }
        else{
            LCD_SendString("No DETECTED");
          while(1){}
        }
    }
}

// entry
void mycodes(void) {
    uint8_t temperature = 0;
    uint8_t humidity = 0;
    char lcd_buffer[16];
    
    HAL_TIM_Base_Start(&htim2);

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);

    LCD_Init(); // 1. init
    
    LCD_SendCommand(0x80); // 2. set the cursor first line title
//    LCD_SendString("System Ready!");
    HAL_Delay(2000);
  
    while(1) {
      
//      DHT11_Lock_Test();
        // ? 2 ????? DHT11
      uint32_t t1 = __HAL_TIM_GET_COUNTER(&htim2);
      
      delay_us(1000);
      uint32_t t2 = __HAL_TIM_GET_COUNTER(&htim2);

      sprintf(lcd_buffer, "%lu", t2 - t1);
      LCD_SendString(lcd_buffer);
      
        if (DHT11_Read(&temperature, &humidity)) {
            // ??????????
            LCD_SendCommand(0x01); 
            HAL_Delay(2);
            
            // ?????,????????
            LCD_SendCommand(0x80);
            sprintf(lcd_buffer, "Temp: %d C", temperature);
            LCD_SendString(lcd_buffer);
            
            // ?????,????????
            LCD_SendCommand(0xC0);
            sprintf(lcd_buffer, "Hum:  %d %%", humidity);
            LCD_SendString(lcd_buffer);
        } else {
            // ??????,??????
            LCD_SendCommand(0xC0);
            //LCD_SendString("Read Error... ");
        }
        
        HAL_Delay(2000); // ?? DHT11 ???????? 2 ???????
    }
}