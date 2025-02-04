#include "BBEventHandler.h"

void BBEventHandler::init()
{
    evt_grp_bb_evt_handler_hdl = xEventGroupCreateStatic(&evt_grp_bb_evt_handler_buff);
    task_bb_evt_handler_hdl = xTaskCreateStatic(
            task_bb_event_handler, "bbTempRHSmplTsk", BB_EVT_HANDLER_TSK_SZ, NULL, 4, task_bb_evt_handler_stk, &task_bb_evt_handler_tcb);
}

bool BBEventHandler::register_event_cb(const bb_event_t evt, bb_evt_cb_t evt_cb, void* cb_param)
{

    if (evt_cb == nullptr)
    {
        BB_LOGE(TAG, "****register_event_cb()**** invalid event callback");
        return false;
    }

    switch (evt)
    {
        case BB_EVT_SAMPLE_TEMP:
            bb_evts_ctx.sample_temp.cb = evt_cb;
            bb_evts_ctx.sample_temp.param = cb_param;
            return true;
            break;

        case BB_EVT_SAMPLE_RH:
            bb_evts_ctx.sample_rh.cb = evt_cb;
            bb_evts_ctx.sample_rh.param = cb_param;
            return true;
            break;

        case BB_EVT_MAINS_HZ_CALC:
            bb_evts_ctx.mains_mains_hz_calc.cb = evt_cb;
            bb_evts_ctx.mains_mains_hz_calc.param = cb_param;
            return true;
            break;

        case BB_EVT_SAMPLE_ISENSE:
            bb_evts_ctx.sample_isense.cb = evt_cb;
            bb_evts_ctx.sample_isense.param = cb_param;
            return true;
            break;

        case BB_EVT_I_CALC:
            bb_evts_ctx.i_calc.cb = evt_cb;
            bb_evts_ctx.i_calc.param = cb_param;
            return true;
            break;

        default:
            BB_LOGE(TAG, "****register_event_cb()**** failed to find matching evt in registry");
            break;
    }

    return false;
}

void BBEventHandler::send_event_to_handler(bb_event_t evt)
{
    xEventGroupSetBits(evt_grp_bb_evt_handler_hdl, evt);
}

void BBEventHandler::send_event_to_handler_from_ISR(bb_event_t evt)
{
    BaseType_t higher_priority_task_awoken = pdFALSE;
    xEventGroupSetBitsFromISR(evt_grp_bb_evt_handler_hdl, evt, &higher_priority_task_awoken);
    portYIELD_FROM_ISR(higher_priority_task_awoken);
}

bool BBEventHandler::execute_sample_temp_evt_cb()
{
    if (bb_evts_ctx.sample_temp.cb != nullptr)
        bb_evts_ctx.sample_temp.cb(bb_evts_ctx.sample_temp.param);
    else
        return false;

    return true;
}

bool BBEventHandler::execute_sample_rh_evt_cb()
{
    if (bb_evts_ctx.sample_rh.cb != nullptr)
        bb_evts_ctx.sample_rh.cb(bb_evts_ctx.sample_temp.param);
    else
        return false;

    return true;
}

bool BBEventHandler::execute_mains_hz_calc_evt_cb()
{
    if (bb_evts_ctx.mains_mains_hz_calc.cb != nullptr)
        bb_evts_ctx.mains_mains_hz_calc.cb(bb_evts_ctx.mains_mains_hz_calc.param);
    else
        return false;

    return true;
}

bool BBEventHandler::execute_sample_isense_evt_cb()
{
    if (bb_evts_ctx.sample_isense.cb != nullptr)
        bb_evts_ctx.sample_isense.cb(bb_evts_ctx.sample_isense.param);
    else
        return false;

    return true;
}

bool BBEventHandler::execute_icalc_evt_cb()
{
    if (bb_evts_ctx.i_calc.cb != nullptr)
        bb_evts_ctx.i_calc.cb(bb_evts_ctx.i_calc.param);
    else
        return false;

    return true;
}

void BBEventHandler::task_bb_event_handler(void* arg)
{
    EventBits_t evt_bits = 0UL;

    while (1)
    {
        evt_bits = xEventGroupWaitBits(evt_grp_bb_evt_handler_hdl, BB_EVT_ALL, pdTRUE, pdFALSE, portMAX_DELAY);

        if (evt_bits & BB_EVT_MAINS_HZ_CALC)
            if (!execute_mains_hz_calc_evt_cb())
                BB_LOGE(TAG, "****task_bb_event_handler**** failed to handle BB_EVT_MAINS_HZ_CALC");

        if (evt_bits & BB_EVT_SAMPLE_ISENSE)
            if (!execute_sample_isense_evt_cb())
                BB_LOGE(TAG, "****task_bb_event_handler**** failed to handle BB_EVT_SAMPLE_ISENSE");

        if (evt_bits & BB_EVT_I_CALC)
        {
            if (!execute_icalc_evt_cb())
                BB_LOGE(TAG, "****task_bb_event_handler**** failed to handle BB_EVT_I_CALC");
        }

        if (evt_bits & BB_EVT_SAMPLE_TEMP)
            if (!execute_sample_temp_evt_cb())
                BB_LOGE(TAG, "****task_bb_event_handler**** failed to handle BB_EVT_SAMPLE_TEMP");

        if (evt_bits & BB_EVT_SAMPLE_RH)
            if (!execute_sample_rh_evt_cb())
                BB_LOGE(TAG, "****task_bb_event_handler**** failed to handle BB_EVT_SAMPLE_RH");
    }
}
