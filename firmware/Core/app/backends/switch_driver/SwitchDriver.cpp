#include "SwitchDriver.h"

#define ENTER_ACTIVE HAL_GPIO_ReadPin(PIN_SW_ENTER.port, PIN_SW_ENTER.num) == GPIO_PIN_RESET

/**
 * @brief Checks if the switch on the specified GPIO pin is pressed (active-low).
 * @param pin The biltong_gpio_t struct containing the GPIO port and pin.
 * @return true if the switch is pressed, otherwise false
 */
#define SWITCH_ACTIVE(pin) (HAL_GPIO_ReadPin(pin.port, pin.num) == GPIO_PIN_RESET)

SwitchDriver::SwitchDriver(Device& d)
    : d(d)
{
}

bool SwitchDriver::init()
{
    evt_grp_sw_hdl = xEventGroupCreateStatic(&evt_grp_sw_buff);
    ISRCbDispatch::register_up_switch_ISR_cb(up_switch_ISR_cb, this);
    ISRCbDispatch::register_enter_switch_ISR_cb(enter_switch_ISR_cb, this);
    ISRCbDispatch::register_down_switch_ISR_cb(down_switch_ISR_cb, this);

    task_switch_scan_hdl = xTaskCreateStatic(task_switch_scan_trampoline, "bbSwitchScanTsk",  BB_SW_SCAN_TSK_SZ, static_cast<void*>(this), 4, task_switch_scan_stk, &task_switch_scan_tcb);

    return true; 
}

void SwitchDriver::down_switch_ISR_cb(void* arg)
{
    SwitchDriver* _sw = static_cast<SwitchDriver*>(arg);

    BaseType_t higher_priority_task_awoken = pdFALSE;
    xEventGroupSetBitsFromISR(_sw->evt_grp_sw_hdl, EVT_GRP_SW_DOWN_BIT, &higher_priority_task_awoken);
    portYIELD_FROM_ISR(higher_priority_task_awoken);
}

void SwitchDriver::enter_switch_ISR_cb(void* arg)
{
    SwitchDriver* sw = static_cast<SwitchDriver*>(arg);

    BaseType_t higher_priority_task_awoken = pdFALSE;
    xEventGroupSetBitsFromISR(sw->evt_grp_sw_hdl, EVT_GRP_SW_ENTER_BIT, &higher_priority_task_awoken);
    portYIELD_FROM_ISR(higher_priority_task_awoken);
}

void SwitchDriver::up_switch_ISR_cb(void* arg)
{
    SwitchDriver* sw = static_cast<SwitchDriver*>(arg);

    BaseType_t higher_priority_task_awoken = pdFALSE;
    xEventGroupSetBitsFromISR(sw->evt_grp_sw_hdl, EVT_GRP_SW_UP_BIT, &higher_priority_task_awoken);
    portYIELD_FROM_ISR(higher_priority_task_awoken);
}

void SwitchDriver::task_switch_scan_trampoline(void* arg)
{
    SwitchDriver* _switch_driver = static_cast<SwitchDriver*>(arg);
    _switch_driver->task_switch_scan();
}

void SwitchDriver::task_switch_scan()
{
    EventBits_t switch_bits;
    scan_state_ctx_t scan_ctx_switch_up(PIN_SW_UP, EVT_GRP_SW_UP_BIT);
    scan_state_ctx_t scan_ctx_switch_enter(PIN_SW_ENTER, EVT_GRP_SW_ENTER_BIT);
    scan_state_ctx_t scan_ctx_switch_down(PIN_SW_DOWN, EVT_GRP_SW_DOWN_BIT);

    while (1)
    {
        switch_bits = xEventGroupWaitBits(evt_grp_sw_hdl, EVT_GRP_SW_ALL, pdFALSE, pdFALSE, portMAX_DELAY);

        do
        {
            if (scan_switch(scan_ctx_switch_up, switch_bits))
                xEventGroupClearBits(evt_grp_sw_hdl, EVT_GRP_SW_UP_BIT);

            if (scan_switch(scan_ctx_switch_enter, switch_bits))
                xEventGroupClearBits(evt_grp_sw_hdl, EVT_GRP_SW_ENTER_BIT);

            if (scan_switch(scan_ctx_switch_down, switch_bits))
                xEventGroupClearBits(evt_grp_sw_hdl, EVT_GRP_SW_DOWN_BIT);

            vTaskDelay(SWITCH_POLL_DELAY_MS);
            switch_bits = xEventGroupGetBits(evt_grp_sw_hdl);

        } while (switch_bits & EVT_GRP_SW_ALL);
    }
}

bool SwitchDriver::scan_switch(scan_state_ctx_t& state_ctx, const EventBits_t current_switch_bits)
{
    switch (state_ctx.current_state)
    {
        case ScanState::inactive:
            state_ctx.current_state = handle_inactive_state(state_ctx, current_switch_bits);
            break;

        case ScanState::debounce_press:
            state_ctx.current_state = handle_debounce_press_state(state_ctx, current_switch_bits);
            break;

        case ScanState::scanning_quick_press:
            state_ctx.current_state = handle_scan_quick_press_state(state_ctx, current_switch_bits);
            break;

        case ScanState::scanning_long_press:
            state_ctx.current_state = handle_scan_long_press_state(state_ctx, current_switch_bits);
            break;

        case ScanState::scanning_held:
            state_ctx.current_state = handle_scan_held_state(state_ctx, current_switch_bits);
            break;

        case ScanState::debounce_release:
            state_ctx.current_state = handle_debounce_release_state(state_ctx, current_switch_bits);

            if (state_ctx.current_state == ScanState::inactive)
                return true;

            break;

        default:

            break;
    }

    return false;
}

SwitchDriver::ScanState SwitchDriver::handle_inactive_state(scan_state_ctx_t& state_ctx, const EventBits_t current_switch_bits)
{
    if (state_ctx.bit & current_switch_bits)
    {
        state_ctx.time_in_state = 0UL;
        return ScanState::debounce_press;
    }

    return ScanState::inactive;
}

SwitchDriver::ScanState SwitchDriver::handle_debounce_press_state(scan_state_ctx_t& state_ctx, const EventBits_t current_switch_bits)
{
    state_ctx.time_in_state += SWITCH_POLL_DELAY_MS;

    if (state_ctx.time_in_state > SWITCH_DEBOUNCE_DELAY_MS)
    {
        state_ctx.time_in_state = 0UL;
        return ScanState::scanning_quick_press;
    }

    return ScanState::debounce_press;
}

SwitchDriver::ScanState SwitchDriver::handle_scan_quick_press_state(scan_state_ctx_t& state_ctx, const EventBits_t current_switch_bits)
{
    state_ctx.time_in_state += SWITCH_POLL_DELAY_MS;

    if (!SWITCH_ACTIVE(state_ctx.pin))
    {
        generate_quick_press_event(state_ctx.bit);
        state_ctx.time_in_state = 0UL;
        return ScanState::debounce_release;
    }
    else if (state_ctx.time_in_state >= SWITCH_QUICK_PRESS_EVENT_TIME)
    {
        state_ctx.time_in_state = 0UL;
        return ScanState::scanning_long_press;
    }

    return ScanState::scanning_quick_press;
}

SwitchDriver::ScanState SwitchDriver::handle_scan_long_press_state(scan_state_ctx_t& state_ctx, const EventBits_t current_switch_bits)
{
    state_ctx.time_in_state += SWITCH_POLL_DELAY_MS;

    if (!SWITCH_ACTIVE(state_ctx.pin))
    {
        generate_quick_press_event(state_ctx.bit);
        state_ctx.time_in_state = 0UL;
        return ScanState::debounce_release;
    }
    else if (state_ctx.time_in_state >= SWITCH_LONG_PRESS_EVENT_TIME)
    {
        generate_long_press_event(state_ctx.bit);
        state_ctx.time_in_state = 0UL;
        return ScanState::scanning_held;
    }

    return ScanState::scanning_long_press;
}

SwitchDriver::ScanState SwitchDriver::handle_scan_held_state(scan_state_ctx_t& state_ctx, const EventBits_t current_switch_bits)
{
    state_ctx.time_in_state += SWITCH_POLL_DELAY_MS;

    if (!SWITCH_ACTIVE(state_ctx.pin))
    {
        state_ctx.time_in_state = 0UL;
        return ScanState::debounce_release;
    }
    else if ((state_ctx.time_in_state % SWITCH_HELD_EVENT_TIME) == 0UL)
    {
        generate_held_event(state_ctx.bit);
    }

    return ScanState::scanning_held;
}

SwitchDriver::ScanState SwitchDriver::handle_debounce_release_state(scan_state_ctx_t& state_ctx, const EventBits_t current_switch_bits)
{
    state_ctx.time_in_state += SWITCH_POLL_DELAY_MS;

    if (state_ctx.time_in_state > SWITCH_DEBOUNCE_DELAY_MS)
    {
        generate_release_event(state_ctx.bit);
        state_ctx.time_in_state = 0UL;

        return ScanState::inactive;
    }

    return ScanState::debounce_release;
}

void SwitchDriver::generate_quick_press_event(EventBits_t sw_bit)
{
    switch (sw_bit)
    {
        case EVT_GRP_SW_UP_BIT:
            d.switches.up.set(SwitchEvent::quick_press);
            break;

        case EVT_GRP_SW_ENTER_BIT:
            d.switches.enter.set(SwitchEvent::quick_press);
            break;

        case EVT_GRP_SW_DOWN_BIT:
            d.switches.down.set(SwitchEvent::quick_press);

            break;

        default:

            break;
    }
}

void SwitchDriver::generate_long_press_event(EventBits_t sw_bit)
{

    switch (sw_bit)
    {
        case EVT_GRP_SW_UP_BIT:
            d.switches.up.set(SwitchEvent::long_press);
            break;

        case EVT_GRP_SW_ENTER_BIT:
            d.switches.enter.set(SwitchEvent::long_press);
            break;

        case EVT_GRP_SW_DOWN_BIT:
            d.switches.down.set(SwitchEvent::long_press);

            break;

        default:

            break;
    }
}

void SwitchDriver::generate_held_event(EventBits_t sw_bit)
{
    switch (sw_bit)
    {
        case EVT_GRP_SW_UP_BIT:
            d.switches.up.set(SwitchEvent::held);
            break;

        case EVT_GRP_SW_ENTER_BIT:
            d.switches.enter.set(SwitchEvent::held);
            break;

        case EVT_GRP_SW_DOWN_BIT:
            d.switches.down.set(SwitchEvent::held);

            break;

        default:

            break;
    }
}

void SwitchDriver::generate_release_event(EventBits_t sw_bit)
{

    switch (sw_bit)
    {
        case EVT_GRP_SW_UP_BIT:
            d.switches.up.set(SwitchEvent::release);
            break;

        case EVT_GRP_SW_ENTER_BIT:
            d.switches.enter.set(SwitchEvent::release);
            break;

        case EVT_GRP_SW_DOWN_BIT:
            d.switches.down.set(SwitchEvent::release);

            break;

        default:

            break;
    }
}
