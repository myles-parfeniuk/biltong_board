
#ifndef _ISR_cb_DISPATCH_IT_H
#define _ISR_cb_DISPATCH_IT_H

#ifdef __cplusplus
extern "C"
{
#endif
// std library includes
#include <stdint.h>
//in-house includes
#include "pins.h"

    typedef void (*biltong_isr_cb_t)();

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

    class ISRCbDispatch
    {
        public:
            inline static void register_up_switch_ISR_cb(biltong_isr_cb_t isr)
            {
                up_switch_ISR_cb = isr;
            }

            inline static void register_enter_switch_ISR_cb(biltong_isr_cb_t isr)
            {
                enter_switch_ISR_cb = isr;
            }

            inline static void register_down_switch_ISR_cb(biltong_isr_cb_t isr)
            {
                down_switch_ISR_cb = isr;
            }

            inline static void execute_up_switch_ISR_cb()
            {
                if (up_switch_ISR_cb != nullptr)
                    up_switch_ISR_cb();
            }

            inline static void execute_enter_switch_ISR_cb()
            {
                if (enter_switch_ISR_cb != nullptr)
                    enter_switch_ISR_cb();
            }

            inline static void execute_down_switch_ISR_cb()
            {
                if (down_switch_ISR_cb != nullptr)
                    down_switch_ISR_cb();
            }

        private:
            inline static biltong_isr_cb_t dummy = nullptr;
            inline static biltong_isr_cb_t up_switch_ISR_cb = nullptr;
            inline static biltong_isr_cb_t enter_switch_ISR_cb = nullptr;
            inline static biltong_isr_cb_t down_switch_ISR_cb = nullptr;
    };

#ifdef __cplusplus
}
#endif

#endif /* _ISR_cb_DISPATCH_IT_H */