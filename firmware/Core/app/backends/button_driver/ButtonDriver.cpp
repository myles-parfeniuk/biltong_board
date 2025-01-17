#include "ButtonDriver.h"

/**
 * @brief Checks if the button on the specified GPIO pin is pressed (active-low).
 * @param pin The biltong_gpio_t struct containing the GPIO port and pin.
 * @return true if the button is pressed, otherwise false
 */
#define BUTTON_ACTIVE(pin) (HAL_GPIO_ReadPin(pin.port, pin.num) == GPIO_PIN_RESET)

EventGroupHandle_t ButtonDriver::evt_grp_btn = NULL;

ButtonDriver::ButtonDriver(Device& d)
    : d(d)
{
}

bool ButtonDriver::init()
{
    evt_grp_btn = xEventGroupCreate();
    ISRDispatch::register_up_button_ISR(up_button_ISR);
    ISRDispatch::register_enter_button_ISR(enter_button_ISR);
    ISRDispatch::register_down_button_ISR(down_button_ISR);
    xTaskCreate(task_button_scan_trampoline, "button_scan", 256, this, 1, &task_button_scan_hdl);
}

void ButtonDriver::up_button_ISR()
{
    BaseType_t higher_priority_task_awoken = pdFALSE;
    xEventGroupSetBitsFromISR(evt_grp_btn, EVT_GRP_BTN_UP_BIT, &higher_priority_task_awoken);
    portYIELD_FROM_ISR(higher_priority_task_awoken);
}

void ButtonDriver::enter_button_ISR()
{
    BaseType_t higher_priority_task_awoken = pdFALSE;
    xEventGroupSetBitsFromISR(evt_grp_btn, EVT_GRP_BTN_ENTER_BIT, &higher_priority_task_awoken);
    portYIELD_FROM_ISR(higher_priority_task_awoken);
}

void ButtonDriver::down_button_ISR()
{
    BaseType_t higher_priority_task_awoken = pdFALSE;
    xEventGroupSetBitsFromISR(evt_grp_btn, EVT_GRP_BTN_DOWN_BIT, &higher_priority_task_awoken);
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
    scan_state_ctx_t scan_ctx_button_up(PIN_SW_UP, EVT_GRP_BTN_UP_BIT);
    scan_state_ctx_t scan_ctx_button_enter(PIN_SW_ENTER, EVT_GRP_BTN_ENTER_BIT);
    scan_state_ctx_t scan_ctx_button_down(PIN_SW_DOWN, EVT_GRP_BTN_DOWN_BIT);

    while (1)
    {
        button_bits = xEventGroupWaitBits(evt_grp_btn, EVT_GRP_BTN_ALL, pdFALSE, pdFALSE, portMAX_DELAY);

        do
        {
            if (scan_button(scan_ctx_button_up, button_bits))
                xEventGroupClearBits(evt_grp_btn, EVT_GRP_BTN_ENTER_BIT);

            if (scan_button(scan_ctx_button_enter, button_bits))
                xEventGroupClearBits(evt_grp_btn, EVT_GRP_BTN_ENTER_BIT);

            if (scan_button(scan_ctx_button_down, button_bits))
                xEventGroupClearBits(evt_grp_btn, EVT_GRP_BTN_DOWN_BIT);

            vTaskDelay(BUTTON_POLL_DELAY_MS);
            button_bits = xEventGroupGetBits(evt_grp_btn);

        } while (button_bits & EVT_GRP_BTN_ALL);
    }
}

bool ButtonDriver::scan_button(scan_state_ctx_t& state_ctx, const EventBits_t current_button_bits)
{
    switch (state_ctx.current_state)
    {
        case ScanState::inactive:
            state_ctx.current_state = handle_inactive_state(state_ctx, current_button_bits);
            break;

        case ScanState::debounce_press:
            state_ctx.current_state = handle_debounce_press_state(state_ctx, current_button_bits);
            break;

        case ScanState::scanning_quick_press:
            state_ctx.current_state = handle_scan_quick_press_state(state_ctx, current_button_bits);
            break;

        case ScanState::scanning_long_press:
            state_ctx.current_state = handle_scan_long_press_state(state_ctx, current_button_bits);
            break;

        case ScanState::scanning_held:
            state_ctx.current_state = handle_scan_held_state(state_ctx, current_button_bits);
            break;

        case ScanState::debounce_release:
            state_ctx.current_state = handle_debounce_release_state(state_ctx, current_button_bits);

            if (state_ctx.current_state == ScanState::inactive)
                return true;

            break;

        default:

            break;
    }

    return false;
}

ButtonDriver::ScanState ButtonDriver::handle_inactive_state(scan_state_ctx_t& state_ctx, const EventBits_t current_button_bits)
{
    if (state_ctx.bit & current_button_bits)
    {
        state_ctx.time_in_state = 0UL;
        return ScanState::debounce_press;
    }

    return ScanState::inactive;
}

ButtonDriver::ScanState ButtonDriver::handle_debounce_press_state(scan_state_ctx_t& state_ctx, const EventBits_t current_button_bits)
{
    if (state_ctx.time_in_state > BUTTON_DEBOUNCE_DELAY_MS)
    {
        state_ctx.time_in_state = 0UL;
        return ScanState::scanning_quick_press;
    }

    state_ctx.time_in_state += BUTTON_POLL_DELAY_MS;

    return ScanState::debounce_press;
}

ButtonDriver::ScanState ButtonDriver::handle_scan_quick_press_state(scan_state_ctx_t& state_ctx, const EventBits_t current_button_bits)
{
    if (!BUTTON_ACTIVE(state_ctx.pin))
    {
        generate_quick_press_event(state_ctx.pin);
        state_ctx.time_in_state = 0UL;
        return ScanState::debounce_release;
    }
    else if (state_ctx.time_in_state >= BUTTON_QUICK_PRESS_EVENT_TIME)
    {
        state_ctx.time_in_state = 0UL;
        return ScanState::scanning_long_press;
    }

    state_ctx.time_in_state += BUTTON_POLL_DELAY_MS;

    return ScanState::scanning_quick_press;
}

ButtonDriver::ScanState ButtonDriver::handle_scan_long_press_state(scan_state_ctx_t& state_ctx, const EventBits_t current_button_bits)
{
    if (!BUTTON_ACTIVE(state_ctx.pin))
    {
        generate_quick_press_event(state_ctx.pin);
        state_ctx.time_in_state = 0UL;
        return ScanState::debounce_release;
    }
    else if (state_ctx.time_in_state >= BUTTON_LONG_PRESS_EVENT_TIME)
    {
        generate_long_press_event(state_ctx.pin);
        state_ctx.time_in_state = 0UL;
        return ScanState::scanning_held;
    }

    state_ctx.time_in_state += BUTTON_POLL_DELAY_MS;

    return ScanState::scanning_long_press;
}

ButtonDriver::ScanState ButtonDriver::handle_scan_held_state(scan_state_ctx_t& state_ctx, const EventBits_t current_button_bits)
{
    if (!BUTTON_ACTIVE(state_ctx.pin))
    {
        state_ctx.time_in_state = 0UL;
        return ScanState::debounce_release;
    }
    else if ((state_ctx.time_in_state % BUTTON_HELD_EVENT_TIME) == 0UL)
    {
        generate_held_event(state_ctx.pin);
    }

    state_ctx.time_in_state += BUTTON_POLL_DELAY_MS;
    return ScanState::scanning_held;
}

ButtonDriver::ScanState ButtonDriver::handle_debounce_release_state(scan_state_ctx_t& state_ctx, const EventBits_t current_button_bits)
{
    if (state_ctx.time_in_state > BUTTON_DEBOUNCE_DELAY_MS)
    {
        generate_release_event(state_ctx.pin);
        state_ctx.time_in_state = 0UL;

        return ScanState::inactive;
    }

    state_ctx.time_in_state += BUTTON_POLL_DELAY_MS;
    return ScanState::debounce_release;
}

void ButtonDriver::generate_quick_press_event(biltong_gpio_t pin)
{
    if (pin == PIN_SW_UP)
        d.buttons.up.set(ButtonEvent::quick_press);
    else if (pin == PIN_SW_ENTER)
        d.buttons.enter.set(ButtonEvent::quick_press);
    else if (pin == PIN_SW_DOWN)
        d.buttons.down.set(ButtonEvent::quick_press);
}

void ButtonDriver::generate_long_press_event(biltong_gpio_t pin)
{
    if (pin == PIN_SW_UP)
        d.buttons.up.set(ButtonEvent::long_press);
    else if (pin == PIN_SW_ENTER)
        d.buttons.enter.set(ButtonEvent::long_press);
    else if (pin == PIN_SW_DOWN)
        d.buttons.down.set(ButtonEvent::long_press);
}

void ButtonDriver::generate_held_event(biltong_gpio_t pin)
{
    if (pin == PIN_SW_UP)
        d.buttons.up.set(ButtonEvent::held);
    else if (pin == PIN_SW_ENTER)
        d.buttons.enter.set(ButtonEvent::held);
    else if (pin == PIN_SW_DOWN)
        d.buttons.down.set(ButtonEvent::held);
}

void ButtonDriver::generate_release_event(biltong_gpio_t pin)
{
    if (pin == PIN_SW_UP)
        d.buttons.up.set(ButtonEvent::release);
    else if (pin == PIN_SW_ENTER)
        d.buttons.enter.set(ButtonEvent::release);
    else if (pin == PIN_SW_DOWN)
        d.buttons.down.set(ButtonEvent::release);
}