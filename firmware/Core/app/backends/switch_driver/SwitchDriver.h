#pragma once

// cube mx
#include "gpio.h"
// in-house includes
#include "Device_types.h"
#include "Device.h"
#include "pins.h"
#include "ISRCbDispatch.h"
#define ENTER_ACTIVE HAL_GPIO_ReadPin(PIN_SW_ENTER.port, PIN_SW_ENTER.num) == GPIO_PIN_RESET
class SwitchDriver
{
    public:
        SwitchDriver(Device& d);
        bool init();
        static void task_switch_scan_trampoline(void* arg);
        void task_switch_scan();

    private:
        enum class ScanState
        {
            inactive,
            debounce_press,
            scanning_quick_press,
            scanning_long_press,
            scanning_held,
            debounce_release
        };

        typedef struct scan_state_ctx_t
        {
                ScanState current_state;
                TickType_t time_in_state;
                biltong_gpio_t pin;
                EventBits_t bit;

                scan_state_ctx_t(biltong_gpio_t pin, EventBits_t bit)
                    : current_state(ScanState::inactive)
                    , time_in_state(0UL)
                    , pin(pin)
                    , bit(bit)
                {
                }
        } scan_state_ctx_t;

        static void up_switch_ISR_cb();
        static void enter_switch_ISR_cb();
        static void down_switch_ISR_cb();

        bool scan_switch(scan_state_ctx_t& state_ctx, const EventBits_t current_switch_bits);

        ScanState handle_inactive_state(scan_state_ctx_t& state_ctx, const EventBits_t current_switch_bits);
        ScanState handle_debounce_press_state(scan_state_ctx_t& state_ctx, const EventBits_t current_switch_bits);
        ScanState handle_scan_quick_press_state(scan_state_ctx_t& state_ctx, const EventBits_t current_switch_bits);
        ScanState handle_scan_long_press_state(scan_state_ctx_t& state_ctx, const EventBits_t current_switch_bits);
        ScanState handle_scan_held_state(scan_state_ctx_t& state_ctx, const EventBits_t current_switch_bits);
        ScanState handle_debounce_release_state(scan_state_ctx_t& state_ctx, const EventBits_t current_switch_bits);

        void generate_quick_press_event(EventBits_t sw_bit);
        void generate_long_press_event(EventBits_t sw_bit);
        void generate_held_event(EventBits_t sw_bit);
        void generate_release_event(EventBits_t sw_bit);

        static const constexpr EventBits_t EVT_GRP_BTN_UP_BIT = (1UL << 0UL);
        static const constexpr EventBits_t EVT_GRP_BTN_ENTER_BIT = (1UL << 1UL);
        static const constexpr EventBits_t EVT_GRP_BTN_DOWN_BIT = (1UL << 2UL);
        static const constexpr EventBits_t EVT_GRP_BTN_ALL = EVT_GRP_BTN_UP_BIT | EVT_GRP_BTN_ENTER_BIT | EVT_GRP_BTN_DOWN_BIT;
        static const constexpr TickType_t SWITCH_POLL_DELAY_MS = 25UL / portTICK_PERIOD_MS;
        static const constexpr TickType_t SWITCH_DEBOUNCE_DELAY_MS = 25UL / portTICK_PERIOD_MS;
        static const constexpr TickType_t SWITCH_QUICK_PRESS_EVENT_TIME = 100UL / portTICK_PERIOD_MS;
        static const constexpr TickType_t SWITCH_LONG_PRESS_EVENT_TIME = 200UL / portTICK_PERIOD_MS;
        static const constexpr TickType_t SWITCH_HELD_EVENT_TIME = 100UL / portTICK_PERIOD_MS;

        Device& d;
        TaskHandle_t task_switch_scan_hdl = NULL;
        inline static EventGroupHandle_t evt_grp_btn_hdl = NULL;
        inline static StaticEventGroup_t evt_grp_btn_buff;
};
