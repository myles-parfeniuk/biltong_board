#include "TempHumiditySensor.h"

TempHumiditySensor::TempHumiditySensor(I2C_HandleTypeDef* hi2c)
    : hdl_i2c(hi2c)
{
}

bool TempHumiditySensor::init()
{
    if (hdl_i2c == nullptr)
        return false;

    HIDS_Get_Default_Interface(&hids);
    hids.interfaceType = WE_i2c_fifo;
    hids.handle = hdl_i2c;

    /* Wait for boot */
    HAL_Delay(50);
    if (WE_SUCCESS != HIDS_Sensor_Init(&hids))
    {
        SerialService::print_log_ln(TAG, "**** HIDS_MULTIPLEXER_Init error. STOP ****");
        return false;
    }

    SerialService::print_log_ln(TAG, "**** WE_isSensorInterfaceReady(): OK ****");
    return true;
}

int32_t TempHumiditySensor::get_humidity()
{
    int32_t humidityRaw = 0;
    hids_measureCmd_t measureCmd = HIDS_MEASURE_HPM;

    // Returns data in milli values (milli % relative humidity, milli deg C)
    if (WE_SUCCESS == HIDS_Sensor_Measure_Raw(&hids, measureCmd, NULL, &humidityRaw))
        SerialService::print_log_ln(TAG, "Humidity: %li", humidityRaw);

    return humidityRaw;
}

int32_t TempHumiditySensor::get_temp()
{
    int32_t temperatureRaw = 0;
    hids_measureCmd_t measureCmd = HIDS_MEASURE_HPM;

    if (WE_SUCCESS == HIDS_Sensor_Measure_Raw(&hids, measureCmd, &temperatureRaw, NULL))
        SerialService::print_log_ln(TAG, "Temperature: %li", temperatureRaw);

    return temperatureRaw;
}
