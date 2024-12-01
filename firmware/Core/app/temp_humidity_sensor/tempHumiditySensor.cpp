#include "tempHumiditySensor.h"
#include <cstring>

/* Sensor interface configuration */
WE_sensorInterface_t tempHumiditySensor::hids;

tempHumiditySensor::tempHumiditySensor()
{
}

/* Debug output functions */
static void debugPrint(char _out[])
{
	HAL_UART_Transmit(&huart3, (uint8_t *)_out, strlen(_out), 10);
}

static void debugPrintln(char _out[])
{
	HAL_UART_Transmit(&huart3, (uint8_t *)_out, strlen(_out), 10);
	char* newline = "\r\n";
	HAL_UART_Transmit(&huart3, (uint8_t *)newline, strlen(newline), 10);
}

bool tempHumiditySensor::init()
{
	HIDS_Get_Default_Interface(&hids);
	hids.interfaceType = WE_i2c_fifo;
	hids.handle = &hi2c1;
	/* Wait for boot */
	HAL_Delay(50);
	if (WE_SUCCESS != HIDS_Sensor_Init(&hids))
	{
		debugPrintln("**** HIDS_MULTIPLEXER_Init error. STOP ****");
		HAL_Delay(5);
		while (1)
			;
	}

	debugPrintln("**** WE_isSensorInterfaceReady(): OK ****");
	return true;
}

int32_t tempHumiditySensor::getHumidity()
{
	int32_t humidityRaw = 0;
	hids_measureCmd_t measureCmd = HIDS_MEASURE_HPM;

	// Returns data in milli values (milli % relative humidity, milli deg C)
	if (WE_SUCCESS == HIDS_Sensor_Measure_Raw(&hids, measureCmd, NULL, &humidityRaw))
	{
		char bufferHumidity[20];
		sprintf(bufferHumidity, "humidity: %li", humidityRaw);
		debugPrint(bufferHumidity);
		debugPrint(",");
	}

	return humidityRaw;
}


int32_t tempHumiditySensor::getTemp()
{
	int32_t temperatureRaw = 0;
	hids_measureCmd_t measureCmd = HIDS_MEASURE_HPM;
	if (WE_SUCCESS == HIDS_Sensor_Measure_Raw(&hids, measureCmd, &temperatureRaw, NULL))
	{
		char bufferTemperature[20];
		sprintf(bufferTemperature, "temperature: %li", temperatureRaw);
		debugPrint(bufferTemperature);
		debugPrintln("");
	}

	return temperatureRaw;
}
