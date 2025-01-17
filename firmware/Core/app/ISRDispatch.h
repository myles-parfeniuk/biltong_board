
#ifndef _ISR_DISPATCH_IT_H
#define _ISR_DISPATCH_IT_H

#ifdef __cplusplus
extern "C"
{
#endif
// std library includes
#include <stdint.h>
//in-house includes
#include "pins.h"

    typedef void (*biltong_isr_t)();

    void NMI_Handler(void);
    void HardFault_Handler(void);
    void EXTI2_3_IRQHandler(void);
    void EXTI4_15_IRQHandler(void);
    void TIM3_IRQHandler(void);
    void TIM6_IRQHandler(void);
    void TIM7_IRQHandler(void);
    void TIM14_IRQHandler(void);
    void TIM16_IRQHandler(void);
    void TIM17_IRQHandler(void);
    void I2C1_IRQHandler(void);
    void I2C2_IRQHandler(void);
    void SPI1_IRQHandler(void);
    void USART3_4_IRQHandler(void);

    class ISRDispatch
    {
        public:
            inline static void register_up_button_ISR(biltong_isr_t isr)
            {
                up_button_ISR = isr;
            }

            inline static void register_enter_button_ISR(biltong_isr_t isr)
            {
                enter_button_ISR = isr;
            }

            inline static void register_down_button_ISR(biltong_isr_t isr)
            {
                down_button_ISR = isr;
            }

            inline static void execute_up_button_ISR()
            {
                if (up_button_ISR != nullptr)
                    up_button_ISR();
            }

            inline static void execute_enter_button_ISR()
            {
                if (enter_button_ISR != nullptr)
                    enter_button_ISR();
            }

            inline static void execute_down_button_ISR()
            {
                if (down_button_ISR != nullptr)
                    down_button_ISR();
            }

        private:
            inline static biltong_isr_t up_button_ISR = nullptr;
            inline static biltong_isr_t enter_button_ISR = nullptr;
            inline static biltong_isr_t down_button_ISR = nullptr;
    };

#ifdef __cplusplus
}
#endif

#endif /* _ISR_DISPATCH_IT_H */