#include "PROJECT/project.h"
#include "main.h"
#include "tim.h"
#include "stm32f767xx.h"

// Project-Specific Functions Definitions

/**
  * @brief This function changes the duty cycle of a given timer pin.
  * @note 16MHz CLK / PSC (remember PSC + 1) / ARR value (ARR value + 1) = duty cycle freq
  * @note duty cycle = Pulse / ARR value
  * @param dutycycle  specifies the duty cycle, in percentage.
  * @retval None
  */
void adjust_PWM_dutycycle(uint32_t dutycycle)
{
    //change the duty cycle
    //sConfigOC.Pulse = (uint32_t*)dutycycle;
    TIM1->CCR1 = dutycycle;
    int ARR_value = htim1.Init.Period;
    //HAL_TIM_OC_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
}