
#ifndef _ISR_cb_DISPATCH_IT_H
#define _ISR_cb_DISPATCH_IT_H

#ifdef __cplusplus
extern "C"
{
#endif
// std library includes
#include <stdint.h>
// in-house includes
#include "bb_pin_defs.h"

    typedef void (*bb_isr_cb_t)(void* arg);

    void NMI_Handler(void);
    void HardFault_Handler(void);
    void EXTI2_3_IRQHandler(void);
    void EXTI4_15_IRQHandler(void);
    void TIM3_IRQHandler(void);
    void TIM6_IRQHandler(void);
    void TIM7_IRQHandler(void);
    void TIM14_IRQHandler(void);
    void TIM15_IRQHandler(void);
    void TIM16_IRQHandler(void);
    void TIM17_IRQHandler(void);
    void I2C1_IRQHandler(void);
    void I2C2_IRQHandler(void);
    void SPI1_IRQHandler(void);
    void USART3_4_IRQHandler(void);

    class ISRCbDispatch
    {
        private:
            typedef struct bb_isr_cb_ctx
            {
                    bb_isr_cb_t cb;
                    void* param;

                    bb_isr_cb_ctx()
                        : cb(nullptr)
                        , param(nullptr)
                    {
                    }
            } bb_isr_cb_ctx;

            typedef struct bb_isr_cb_registry_t
            {
                    bb_isr_cb_ctx up_switch;
                    bb_isr_cb_ctx enter_switch;
                    bb_isr_cb_ctx down_switch;
                    bb_isr_cb_ctx zero_cross;
            } bb_isr_cb_registry_t;

            inline static bb_isr_cb_registry_t ISR_ctx;

        public:
            inline static void register_up_switch_ISR_cb(bb_isr_cb_t isr, void* cb_param)
            {
                ISR_ctx.up_switch.cb = isr;
                ISR_ctx.up_switch.param = cb_param;
            }

            inline static void register_enter_switch_ISR_cb(bb_isr_cb_t isr, void* cb_param)
            {
                ISR_ctx.enter_switch.cb = isr;
                ISR_ctx.enter_switch.param = cb_param;
            }

            inline static void register_down_switch_ISR_cb(bb_isr_cb_t isr, void* cb_param)
            {
                ISR_ctx.down_switch.cb = isr;
                ISR_ctx.down_switch.param = cb_param;
            }

            inline static void register_zero_cross_ISR_cb(bb_isr_cb_t isr, void* cb_param)
            {
                ISR_ctx.zero_cross.cb = isr;
                ISR_ctx.zero_cross.param = cb_param;
            }

            inline static void execute_up_switch_ISR_cb()
            {
                if (ISR_ctx.up_switch.cb != nullptr)
                    ISR_ctx.up_switch.cb(ISR_ctx.up_switch.param);
            }

            inline static void execute_enter_switch_ISR_cb()
            {
                if (ISR_ctx.enter_switch.cb != nullptr)
                    ISR_ctx.enter_switch.cb(ISR_ctx.enter_switch.param);
            }

            inline static void execute_down_switch_ISR_cb()
            {
                if (ISR_ctx.down_switch.cb != nullptr)
                    ISR_ctx.down_switch.cb(ISR_ctx.down_switch.param);
            }

            inline static void execute_zero_cross_ISR_cb()
            {
                if (ISR_ctx.zero_cross.cb != nullptr)
                    ISR_ctx.zero_cross.cb(ISR_ctx.zero_cross.param);
            }
    }; // namespace ISRCbDispatch

#ifdef __cplusplus
}
#endif

#endif /* _ISR_cb_DISPATCH_IT_H */