#include "main.h"
#include "stm32g0xx_it.h"

/**
  ******************************************************************************
  * @file    biltong_it.c
  * @brief   Interrupt Service Routines
  ******************************************************************************
  * @attention
  *
  * Author: Eddie Sherban
  *
  * This file is where all the biltong-related interrupts will be defined.
  * Cube's generated files will initialize all necessary interrupts, and this
  * file will hard define all the IRQHandlers, since they are currently weak-defined
  * by the startup assembly code.
  * 
  *     List of EXTI lines used:
  *         GPIO_EXTI2: PD2 -> ZERO CROSS
  *         GPIO_EXTI3: PA3 -> SW_UP
  *         GPIO_EXTI4: PA4 -> SW_ENTER
  *         GPIO_EXTI5: PA5 -> SW_DOWN
  *
  ******************************************************************************
  */

extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c2;
extern DMA_HandleTypeDef hdma_spi1_tx;
extern SPI_HandleTypeDef hspi1;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim6;
extern TIM_HandleTypeDef htim14;
extern TIM_HandleTypeDef htim16;
extern TIM_HandleTypeDef htim17;
extern UART_HandleTypeDef huart3;
extern TIM_HandleTypeDef htim7;



 void EXTI2_3_IRQHandler(void)
 {
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_3);
 }

 