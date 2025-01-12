#pragma once
// cube-mx includes
#include "gpio.h"

typedef struct biltong_board_gpio_t
{
        GPIO_TypeDef* port;
        uint16_t num;
};

// switch pins
static const constexpr biltong_board_gpio_t PIN_SW_UP = {GPIOA, GPIO_PIN_3};
static const constexpr biltong_board_gpio_t PIN_SW_ENTER = {GPIOA, GPIO_PIN_4};
static const constexpr biltong_board_gpio_t PIN_SW_DOWN = {GPIOA, GPIO_PIN_5};

// display pins
static const constexpr biltong_board_gpio_t PIN_DISP_SCL = {GPIOA, GPIO_PIN_1};
static const constexpr biltong_board_gpio_t PIN_DISP_MOSI = {GPIOA, GPIO_PIN_2};
static const constexpr biltong_board_gpio_t PIN_DISP_CS = {GPIOB, GPIO_PIN_7};
static const constexpr biltong_board_gpio_t PIN_DISP_DC = {GPIOB, GPIO_PIN_8};
static const constexpr biltong_board_gpio_t PIN_DISP_RST = {GPIOB, GPIO_PIN_9};

// uart pins
static const constexpr biltong_board_gpio_t PIN_UART_TX = {GPIOB, GPIO_PIN_10};
static const constexpr biltong_board_gpio_t PIN_UART_RX = {GPIOB, GPIO_PIN_11};

// temp/humidity sensor pins
static const constexpr biltong_board_gpio_t PIN_TH_B_SCL = {GPIOA, GPIO_PIN_9};
static const constexpr biltong_board_gpio_t PIN_TH_B_SDA = {GPIOA, GPIO_PIN_10};
static const constexpr biltong_board_gpio_t PIN_TH_A_SCL = {GPIOA, GPIO_PIN_11};
static const constexpr biltong_board_gpio_t PIN_TH_A_SDA = {GPIOA, GPIO_PIN_12};

// fan pins
static const constexpr biltong_board_gpio_t PIN_FAN_B_PWM = {GPIOA, GPIO_PIN_6};
static const constexpr biltong_board_gpio_t PIN_FAN_B_TACH = {GPIOA, GPIO_PIN_7};
static const constexpr biltong_board_gpio_t PIN_FAN_A_PWM = {GPIOB, GPIO_PIN_1};
static const constexpr biltong_board_gpio_t PIN_FAN_A_TACH = {GPIOB, GPIO_PIN_0};

// heat lamp control & feedback pins
static const constexpr biltong_board_gpio_t PIN_TRIAC_TRIG = {GPIOB, GPIO_PIN_3};
static const constexpr biltong_board_gpio_t PIN_HEAT_LAMP_EN = {GPIOB, GPIO_PIN_4};
static const constexpr biltong_board_gpio_t PIN_BUZZ = {GPIOB, GPIO_PIN_6};
static const constexpr biltong_board_gpio_t PIN_I_SENSE = {GPIOB, GPIO_PIN_12};
static const constexpr biltong_board_gpio_t PIN_ZERO_CROSS = {GPIOD, GPIO_PIN_2};

static const constexpr uint16_t BIT0 = (1U << 0);
static const constexpr uint16_t BIT1 = (1U << 1);
static const constexpr uint16_t BIT2 = (1U << 2);
static const constexpr uint16_t BIT3 = (1U << 3);
static const constexpr uint16_t BIT4 = (1U << 4);
static const constexpr uint16_t BIT5 = (1U << 5);
static const constexpr uint16_t BIT6 = (1U << 6);
static const constexpr uint16_t BIT7 = (1U << 7);
static const constexpr uint16_t BIT8 = (1U << 8);
static const constexpr uint16_t BIT9 = (1U << 9);
static const constexpr uint16_t BIT10 = (1U << 10);
static const constexpr uint16_t BIT11 = (1U << 11);
static const constexpr uint16_t BIT12 = (1U << 12);
static const constexpr uint16_t BIT13 = (1U << 13);
static const constexpr uint16_t BIT14 = (1U << 14);
static const constexpr uint16_t BIT15 = (1U << 15);
