#include "TempRHDriver.h"

TempRHDriver::TempRHDriver(Device& d, I2C_HandleTypeDef* hdl_i2c_th_A, I2C_HandleTypeDef* hdl_i2c_th_B)
    : d(d)
    , hdl_i2c_th_A(hdl_i2c_th_A)
    , hdl_i2c_th_B(hdl_i2c_th_B)
    , th_A(hdl_i2c_th_A)
    , th_B(hdl_i2c_th_B)
{
}

bool TempRHDriver::init()
{
    BaseType_t op_success = pdFALSE;

    if (!th_A.init())
        return false;

    if (!th_B.init())
        return false;

    evt_grp_temp_rh_hdl = xEventGroupCreateStatic(&evt_grp_temp_rh_buff);

    timer_temp_hdl = xTimerCreateStatic(
            "Temptimer", d.sensors.temperature.sample_rate.get() / portTICK_PERIOD_MS, pdTRUE, this, timer_temp_cb, &timer_temp_buff);

    timer_rh_hdl = xTimerCreateStatic(
            "RHTimer", d.sensors.humidity.sample_rate.get() / portTICK_PERIOD_MS, pdTRUE, static_cast<void*>(this), timer_rh_cb, &timer_rh_buff);

    op_success = xTimerStart(timer_temp_hdl, 0UL);

    if (op_success != pdTRUE)
        return false;

    op_success = xTimerStart(timer_rh_hdl, 0UL);

    if (op_success != pdTRUE)
        return false;

    task_temp_rh_hdl = xTaskCreateStatic(task_temp_rh_trampoline, "bbTempRHSmplTsk",  BB_TEMP_RH_TSK_SZ, static_cast<void*>(this), 4, task_temp_rh_stk, &task_temp_rh_tcb);

    return true;
}

void TempRHDriver::timer_temp_cb(TimerHandle_t timer_hdl)
{
    TempRHDriver* temp_rh_driver = reinterpret_cast<TempRHDriver*>(timer_hdl);
    xEventGroupSetBits(temp_rh_driver->evt_grp_temp_rh_hdl, EVT_GRP_TEMP_RH_SAMPLE_TEMP);
}

void TempRHDriver::timer_rh_cb(TimerHandle_t timer_hdl)
{
    TempRHDriver* temp_rh_driver = reinterpret_cast<TempRHDriver*>(timer_hdl);
    xEventGroupSetBits(temp_rh_driver->evt_grp_temp_rh_hdl, EVT_GRP_TEMP_RH_SAMPLE_RH);
}

void TempRHDriver::task_temp_rh_trampoline(void* arg)
{
    TempRHDriver* temp_rh_driver = static_cast<TempRHDriver*>(arg);
    temp_rh_driver->task_temp_rh();
}

void TempRHDriver::task_temp_rh()
{
    EventBits_t sample_bits = 0UL;
    temp_data_t temp_temp;
    temp_data_t current_temp;
    rh_data_t temp_rh;
    rh_data_t current_rh;

    while (1)
    {
        sample_bits = xEventGroupWaitBits(evt_grp_temp_rh_hdl, EVT_GRP_TEMP_RH_ALL, pdFALSE, pdFALSE, portMAX_DELAY);

        if (sample_bits & EVT_GRP_TEMP_RH_SAMPLE_TEMP)
        {
            temp_temp = {-1L, -1L};

            // read temp A
            if (th_A.get_temp(temp_temp.A))
                current_temp.A = temp_temp.A;
            else
                SerialService::LOG_ln<BB_LL_ERROR>(TAG, "FAIL: Read temp sens A");

            // read temp B
            if (th_B.get_temp(temp_temp.B))
                current_temp.B = temp_temp.B;
            else
                SerialService::LOG_ln<BB_LL_ERROR>(TAG, "FAIL: Read temp sens B");

            // set device model if either temp was successfully read
            if ((temp_temp.A != -1) || (temp_temp.B != -1))
                d.sensors.temperature.celsius.set(current_temp);
        }

        if (sample_bits & EVT_GRP_TEMP_RH_SAMPLE_RH)
        {
            temp_rh = {-1L, -1L};

            // read RH A
            if (th_A.get_humidity(temp_rh.A))
                current_rh.A = temp_rh.A;
            else
                SerialService::LOG_ln<BB_LL_ERROR>(TAG, "FAIL: Read RH sens A");

            // read RH B
            if (th_B.get_humidity(temp_rh.B))
                current_rh.B = temp_rh.B;
            else
                SerialService::LOG_ln<BB_LL_ERROR>(TAG, "FAIL: Read RH sens B");

            // set device model if either RH was successfully read
            if ((temp_rh.A != -1) || (temp_rh.B != -1))
                d.sensors.humidity.relative.set(current_rh);
        }

        xEventGroupClearBits(evt_grp_temp_rh_hdl, EVT_GRP_TEMP_RH_ALL);
    }
}