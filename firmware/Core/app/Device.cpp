#include "Device.h"

Device::Device()
    : buttons{
        DataWatch<ButtonEvent, 4>(ButtonEvent::uninit), 
        DataWatch<ButtonEvent, 4>(ButtonEvent::uninit),
        DataWatch<ButtonEvent, 4>(ButtonEvent::uninit)
    }
{
}