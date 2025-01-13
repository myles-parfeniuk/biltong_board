#include "ButtonDriver.h"

#define ENTER_ACTIVE (HAL_GPIO_ReadPin(PIN_SW_ENTER.port, PIN_SW_ENTER.num) == GPIO_PIN_RESET)

EventGroupHandle_t ButtonDriver::evt_grp_btn = NULL;

ButtonDriver::ButtonDriver(Device& d)
    : d(d)
{
}

bool ButtonDriver::init()
{
    evt_grp_btn = xEventGroupCreate();
    xTaskCreate(task_button_scan_trampoline, "button_scan", 256, this, 1, &task_button_scan_hdl);
}

void ButtonDriver::up_button_ISR()
{
    BaseType_t higher_priority_task_awoken = pdFALSE;
    xEventGroupSetBitsFromISR(evt_grp_btn, EVT_GRP_BTN_ENTER_BIT, &higher_priority_task_awoken);
    portYIELD_FROM_ISR(higher_priority_task_awoken);
}

void ButtonDriver::task_button_scan_trampoline(void* arg)
{
    ButtonDriver* button_driver_ptr = static_cast<ButtonDriver*>(arg);
    button_driver_ptr->task_button_scan();
}

void ButtonDriver::task_button_scan()
{
    uint32_t enter_button_active_ms = 0UL;
    EventBits_t button_bits;

    while (1)
    {
        button_bits = xEventGroupWaitBits(evt_grp_btn, EVT_GRP_BTN_ENTER_BIT, pdTRUE, pdFALSE, portMAX_DELAY);

        if (button_bits & EVT_GRP_BTN_ENTER_BIT)
        {
            vTaskDelay(BUTTON_POLL_DELAY_MS); // debounce press

            while (ENTER_ACTIVE)
            {
                vTaskDelay(BUTTON_POLL_DELAY_MS);
                enter_button_active_ms += 50UL;

                if (enter_button_active_ms == 200UL)
                    d.buttons.enter.set(ButtonEvent::long_press);
                else if (enter_button_active_ms >= 200UL)
                    d.buttons.enter.set(ButtonEvent::held);
            }

            if (enter_button_active_ms < 200UL)
                d.buttons.enter.set(ButtonEvent::quick_press);
            else
                d.buttons.enter.set(ButtonEvent::release);

            enter_button_active_ms = 0UL;
            xEventGroupClearBits(evt_grp_btn, EVT_GRP_BTN_ENTER_BIT);
        }
    }
}