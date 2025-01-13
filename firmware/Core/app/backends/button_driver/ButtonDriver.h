#pragma once

//cube mx
#include "gpio.h"
// in-house includes
#include "ButtonDriver_types.h"
#include "Device.h"
#include "pins.h"

class ButtonDriver
{
    public:
        ButtonDriver(Device& d);
        bool init();
        static void task_button_scan_trampoline(void *arg);
        void task_button_scan();
        static void up_button_ISR();

    private:
        static const constexpr EventBits_t EVT_GRP_BTN_UP_BIT = (1UL << 0UL);
        static const constexpr EventBits_t EVT_GRP_BTN_ENTER_BIT = (1UL << 1UL);
        static const constexpr EventBits_t EVT_GRP_BTN_DOWN_BIT = (1UL << 2UL);
        static const constexpr EventBits_t EVT_GRP_BTN_ALL = EVT_GRP_BTN_UP_BIT | EVT_GRP_BTN_ENTER_BIT | EVT_GRP_BTN_DOWN_BIT;
        static const constexpr TickType_t  BUTTON_POLL_DELAY_MS = 50UL / portTICK_PERIOD_MS;
        Device& d;
        TaskHandle_t task_button_scan_hdl = NULL;
        static EventGroupHandle_t evt_grp_btn;
};
