#pragma once

// std library
#include <stdint.h>
// third-party
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
// in-house
#include "bb_task_defs.h"
#include "SerialService.h"

enum bb_event_t
{
    BB_EVT_SAMPLE_TEMP = (1UL << 0UL),
    BB_EVT_SAMPLE_RH = (1UL << 1UL),
    BB_EVT_MAINS_HZ_CALC = (1UL << 2UL),
    BB_EVT_SAMPLE_ISENSE = (1UL << 3UL),
    BB_EVT_I_CALC = (1UL << 4UL),
    BB_EVT_ALL = (1UL << 5UL) - 1UL
};

typedef void (*bb_evt_cb_t)(void* arg);

class BBEventHandler
{
    public:
        static void init();
        static bool register_event_cb(const bb_event_t evt, bb_evt_cb_t evt_cb, void* cb_param);
        static void send_event_to_handler(bb_event_t evt);
        static void send_event_to_handler_from_ISR(bb_event_t evt);

    private:
        typedef struct bb_evt_cb_ctx
        {
                bb_evt_cb_t cb;
                void* param;

                bb_evt_cb_ctx()
                    : cb(nullptr)
                    , param(nullptr)
                {
                }
        } bb_evt_cb_ctx;

        typedef struct bb_evt_cb_registry_t
        {
                bb_evt_cb_ctx sample_temp;
                bb_evt_cb_ctx sample_rh;
                bb_evt_cb_ctx mains_mains_hz_calc;
                bb_evt_cb_ctx sample_isense;
                bb_evt_cb_ctx i_calc;
        } bb_evt_cb_registry_t;

        static bool execute_sample_temp_evt_cb();
        static bool execute_sample_rh_evt_cb();
        static bool execute_mains_hz_calc_evt_cb();
        static bool execute_sample_isense_evt_cb();
        static bool execute_icalc_evt_cb();

        static void task_bb_event_handler(void* arg);

        inline static bb_evt_cb_registry_t bb_evts_ctx;
        inline static TaskHandle_t task_bb_evt_handler_hdl = NULL;
        inline static StaticTask_t task_bb_evt_handler_tcb;
        inline static StackType_t task_bb_evt_handler_stk[BB_EVT_HANDLER_TSK_SZ] = {0UL};
        inline static EventGroupHandle_t evt_grp_bb_evt_handler_hdl = NULL;
        inline static StaticEventGroup_t evt_grp_bb_evt_handler_buff;

        inline static const constexpr char* TAG = "BBEventHandler";
};