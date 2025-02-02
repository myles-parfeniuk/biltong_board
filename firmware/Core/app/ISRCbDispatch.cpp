// cube-mx includes
#include "main.h"
// in-house includes
#include "ISRCbDispatch.h"

/**
 ******************************************************************************
 * @file    ISRCbDispatch.c
 * @brief   Dispatches specific ISR callbacks from shared ISRs.
 ******************************************************************************
 * @attention
 *
 * Author: Eddie Sherban & Myles Parfeniuk
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
extern TIM_HandleTypeDef htim15;
extern TIM_HandleTypeDef htim16;
extern TIM_HandleTypeDef htim17;
extern UART_HandleTypeDef huart3;
extern TIM_HandleTypeDef htim7;

/******************************************************************************/
/*           Cortex-M0+ Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
 * @brief This function handles Non maskable interrupt.
 */
void NMI_Handler(void)
{
    /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

    /* USER CODE END NonMaskableInt_IRQn 0 */
    /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
    while (1)
    {
    }
    /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
 * @brief This function handles Hard fault interrupt.
 */
void HardFault_Handler(void)
{
    /* USER CODE BEGIN HardFault_IRQn 0 */

    /* USER CODE END HardFault_IRQn 0 */
    while (1)
    {
        /* USER CODE BEGIN W1_HardFault_IRQn 0 */
        /* USER CODE END W1_HardFault_IRQn 0 */
    }
}

/******************************************************************************/
/* STM32G0xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32g0xx.s).                    */
/******************************************************************************/

/**
 * @brief This function handles EXTI line 2 and line 3 interrupts.
 */
void EXTI2_3_IRQHandler(void)
{
    if (__HAL_GPIO_EXTI_GET_IT(PIN_ZERO_CROSS.num) == SET)
        ISRCbDispatch::execute_zero_cross_ISR_cb();

    if (__HAL_GPIO_EXTI_GET_IT(PIN_SW_UP.num) == SET)
    {
        ISRCbDispatch::execute_up_switch_ISR_cb();
        __HAL_GPIO_EXTI_CLEAR_IT(PIN_SW_DOWN.num);
        __HAL_GPIO_EXTI_CLEAR_IT(PIN_SW_ENTER.num);
    }

    HAL_GPIO_EXTI_IRQHandler(PIN_ZERO_CROSS.num);
    HAL_GPIO_EXTI_IRQHandler(PIN_SW_UP.num);
}

/**
 * @brief This function handles EXTI line 4 to 15 interrupts.
 */
void EXTI4_15_IRQHandler(void)
{
    if (__HAL_GPIO_EXTI_GET_IT(PIN_SW_ENTER.num) == SET)
    {
        ISRCbDispatch::execute_enter_switch_ISR_cb();
        __HAL_GPIO_EXTI_CLEAR_IT(PIN_SW_DOWN.num);
        __HAL_GPIO_EXTI_CLEAR_IT(PIN_SW_UP.num);
    }

    if (__HAL_GPIO_EXTI_GET_IT(PIN_SW_DOWN.num) == SET)
    {
        ISRCbDispatch::execute_down_switch_ISR_cb();
        __HAL_GPIO_EXTI_CLEAR_IT(PIN_SW_UP.num);
        __HAL_GPIO_EXTI_CLEAR_IT(PIN_SW_ENTER.num);
    }

    HAL_GPIO_EXTI_IRQHandler(PIN_SW_ENTER.num);
    HAL_GPIO_EXTI_IRQHandler(PIN_SW_DOWN.num);
}

/**
 * @brief This function handles TIM3 global interrupt.
 */
void TIM3_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim3);
}

/**
 * @brief This function handles TIM6 global interrupt.
 */
void TIM6_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim6);
}

/**
 * @brief This function handles TIM7 global interrupt.
 */
void TIM7_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim7);
}

/**
 * @brief This function handles TIM14 global interrupt.
 */
void TIM14_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim14);
}

/**
 * @brief This function handles TIM15 global interrupt.
 */
void TIM15_IRQHandler(void)
{
    /* USER CODE BEGIN TIM15_IRQn 0 */

    /* USER CODE END TIM15_IRQn 0 */
    if (__HAL_TIM_GET_FLAG(&htim15, TIM_FLAG_CC1))
    {
        ISRCbDispatch::execute_triac_trig_ISR_cb();
    }

    HAL_TIM_IRQHandler(&htim15);
    /* USER CODE BEGIN TIM15_IRQn 1 */

    /* USER CODE END TIM15_IRQn 1 */
}

/**
 * @brief This function handles TIM16 global interrupt.
 */
void TIM16_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim16);
}

/**
 * @brief This function handles TIM17 global interrupt.
 */
void TIM17_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim17);
}

/**
 * @brief This function handles I2C1 event global interrupt / I2C1 wake-up interrupt through EXTI line 23.
 */
void I2C1_IRQHandler(void)
{
    if (hi2c1.Instance->ISR & (I2C_FLAG_BERR | I2C_FLAG_ARLO | I2C_FLAG_OVR))
    {
        HAL_I2C_ER_IRQHandler(&hi2c1);
    }
    else
    {
        HAL_I2C_EV_IRQHandler(&hi2c1);
    }
}

/**
 * @brief This function handles I2C2 global interrupt.
 */
void I2C2_IRQHandler(void)
{
    if (hi2c2.Instance->ISR & (I2C_FLAG_BERR | I2C_FLAG_ARLO | I2C_FLAG_OVR))
    {
        HAL_I2C_ER_IRQHandler(&hi2c2);
    }
    else
    {
        HAL_I2C_EV_IRQHandler(&hi2c2);
    }
}

/**
 * @brief This function handles SPI1 global interrupt.
 */
void SPI1_IRQHandler(void)
{
    HAL_SPI_IRQHandler(&hspi1);
}

/**
 * @brief This function handles USART3 and USART4 interrupts.
 */
void USART3_4_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart3);
}