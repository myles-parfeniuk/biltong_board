#include "NSM201210XXR.h"

NSM201210XXR::NSM201210XXR(ADC_HandleTypeDef* hdl_isense_adc):
hdl_isense_adc(hdl_isense_adc)
{
}

bool NSM201210XXR::init()
{
    return true;
}
