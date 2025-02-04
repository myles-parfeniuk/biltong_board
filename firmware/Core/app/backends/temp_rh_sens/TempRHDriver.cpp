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

    if (BBEventHandler::register_event_cb(BB_EVT_SAMPLE_TEMP, sample_temp_evt_cb, this) != true)
        return false;

    if (BBEventHandler::register_event_cb(BB_EVT_SAMPLE_RH, sample_rh_evt_cb, this) != true)
        return false;

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

    return true;
}

void TempRHDriver::timer_temp_cb(TimerHandle_t timer_hdl)
{
    BBEventHandler::send_event_to_handler(BB_EVT_SAMPLE_TEMP);
}

void TempRHDriver::timer_rh_cb(TimerHandle_t timer_hdl)
{
    BBEventHandler::send_event_to_handler(BB_EVT_SAMPLE_RH);
}

void TempRHDriver::sample_temp_evt_cb(void* arg)
{
    TempRHDriver* _driver = static_cast<TempRHDriver*>(arg);
    temp_data_t new_temp;
    temp_data_t current_temp;

    new_temp = {-1L, -1L};

    // read temp A
    if (_driver->th_A.get_temp(new_temp.A))
        current_temp.A = new_temp.A;
    else
        BB_LOGE(TAG, "****sample_temp_evt_cb()**** failed to read temp sense A");

    // read temp B
    if (_driver->th_B.get_temp(new_temp.B))
        current_temp.B = new_temp.B;
    else
        BB_LOGE(TAG, "****sample_temp_evt_cb()**** failed to read temp sense B");

    // set device model if both temps were successfully read
    if ((new_temp.A != -1) && (new_temp.B != -1))
        _driver->d.sensors.temperature.celsius.set(current_temp);
}
void TempRHDriver::sample_rh_evt_cb(void* arg)
{
    TempRHDriver* _driver = static_cast<TempRHDriver*>(arg);
    rh_data_t new_rh;
    rh_data_t current_rh;

    new_rh = {-1L, -1L};

    // read RH A
    if (_driver->th_A.get_humidity(new_rh.A))
        current_rh.A = new_rh.A;
    else
        BB_LOGE(TAG, "****task_temp_rh()**** failed to read RH sense A");

    // read RH B
    if (_driver->th_B.get_humidity(new_rh.B))
        current_rh.B = new_rh.B;
    else
        BB_LOGE(TAG, "****task_temp_rh()**** failed to read RH sense B");

    // set device model if both RH were successfully read
    if ((new_rh.A != -1) && (new_rh.B != -1))
        _driver->d.sensors.humidity.relative.set(current_rh);
}