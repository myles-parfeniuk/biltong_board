#include "TempRHSens.h"

TempRHSens::TempRHSens(I2C_HandleTypeDef* hi2c)
    : hdl_i2c(hi2c)
{
}

bool TempRHSens::init()
{
    if (hdl_i2c == nullptr)
        return false;

    HIDS_Get_Default_Interface(&hids);
    hids.interfaceType = WE_i2c_fifo;
    hids.handle = hdl_i2c;

    vTaskDelay(BOOT_DELAY_MS);

    if (WE_SUCCESS != HIDS_Sensor_Init(&hids))
    {
        SerialService::print_log_ln(TAG, "**** HIDS_MULTIPLEXER_Init error. STOP ****");
        return false;
    }

    SerialService::print_log_ln(TAG, "**** WE_isSensorInterfaceReady(): OK ****");
    return true;
}

bool TempRHSens::get_humidity(int32_t& humidity_raw)
{
    hids_measureCmd_t measured_cmd = HIDS_MEASURE_HPM;

    // returns data in milli values (milli % relative humidity, milli deg C)
    if (WE_SUCCESS == HIDS_Sensor_Measure_Raw(&hids, measured_cmd, NULL, &humidity_raw))
        return true;
    else
        return false;
}

bool TempRHSens::get_temp(int32_t& temperature_raw)
{
    hids_measureCmd_t measured_cmd = HIDS_MEASURE_HPM;

    if (WE_SUCCESS == HIDS_Sensor_Measure_Raw(&hids, measured_cmd, &temperature_raw, NULL))
        return true;
    else
        return false;
}
