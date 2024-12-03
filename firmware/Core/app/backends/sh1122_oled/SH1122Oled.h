#pragma once
// std library includes
#include <stdio.h>
#include <stdint.h>
#include <cstring>
#include <math.h>
// cube mx inclues
#include "spi.h"
// third party includes
#include "FreeRTOS.h"
#include "task.h"
// in-house includes
#include "pins.h"
#include "SH1122Oled_types.h"

#define SH1122_PIXEL_IN_BOUNDS(x, y) (((x >= 0) && (x < SH1122Oled::WIDTH)) && ((y >= 0) && (y < SH1122Oled::HEIGHT)))

class SH1122Oled
{
    public:
        SH1122Oled(SPI_HandleTypeDef* hspi);
        bool init();

        void clear_buffer();
        bool update_screen();
        bool set_pixel(uint16_t x, uint16_t y, SH1122PixIntens intensity);
        bool draw_line(int16_t x_1, int16_t y_1, int16_t x_2, int16_t y_2, SH1122PixIntens intensity);
        void draw_rectangle_frame(int16_t x_1, int16_t y_1, int16_t width, int16_t height, int16_t thickness, SH1122PixIntens intensity);
        void draw_rectangle(int16_t x_1, int16_t y_1, int16_t width, int16_t height, SH1122PixIntens intensity);

        void reset();
        bool off();
        bool on();
        bool set_oscillator_freq(uint8_t freq_reg_val);
        bool set_multiplex_ratio(uint8_t multiplex_ratio_reg_val);
        bool set_display_offset_mod(uint8_t mod);
        bool set_row_addr(uint8_t row_addr);
        bool set_high_column_address(uint8_t high_column_addr);
        bool set_low_column_address(uint8_t low_column_addr);
        bool set_start_line(uint8_t start_line);
        bool set_vseg_discharge_level(uint8_t discharge_level);
        bool set_dc_dc_control_mod(uint8_t mod);
        bool set_segment_remap(bool remapped);
        bool set_orientation(bool flipped);
        bool set_contrast(uint8_t contrast_reg_val);
        bool set_precharge_period(uint8_t period_reg_val);
        bool set_vcom(uint8_t vcom_reg_val);
        bool set_vseg(uint8_t vseg_reg_val);
        bool set_inverted_intensity(bool inverted);

        static const constexpr uint16_t WIDTH = 256U; ///<Display width
        static const constexpr uint16_t HEIGHT = 64U; ///<Display height

    private:
        /// @brief Represents point on OLED screen, used in drawing functions
        typedef struct sh1122_2d_point_t
        {
                int16_t x;
                int16_t y;
        } sh1122_2d_point_t;

        bool send_cmds(uint8_t* cmds, uint16_t length);
        SPI_HandleTypeDef* hdl_spi = nullptr;

        static const constexpr uint16_t FRAME_BUFFER_LENGTH = WIDTH * HEIGHT / 2; ///< Length of frame buffer being sent over SPI.
        uint8_t frame_buffer[FRAME_BUFFER_LENGTH];                                ///< Frame buffer to contain pixel data being sent over SPI.

        static const constexpr TickType_t RST_DELAY_MS = 100UL / portTICK_PERIOD_MS;

        // commands
        static const constexpr uint8_t CMD_POWER_ON = 0xAF;              ///< Power on command.
        static const constexpr uint8_t CMD_POWER_OFF = 0xAE;             ///< Power off command.
        static const constexpr uint8_t CMD_SET_ROW_ADDR = 0xB0;          ///< Set row address command.
        static const constexpr uint8_t CMD_SCAN_0_TO_N = 0xC0;           ///< Scan from bottom to top command.
        static const constexpr uint8_t CMD_SCAN_N_TO_0 = 0xC8;           ///< Scan from top to bottom command.
        static const constexpr uint8_t CMD_NORM_SEG_MAP = 0xA0;          ///< Regular segment driver output pad assignment command.
        static const constexpr uint8_t CMD_REV_SEG_MAP = 0xA1;           ///< Reversed segment driver output pads assignment command.
        static const constexpr uint8_t CMD_SET_MULTIPLEX_RATION = 0xA8;  ///< Multiplex ratio set command.
        static const constexpr uint8_t CMD_SET_DC_DC_CTRL_MOD = 0xAD;    ///< Set onboard oled DC-DC voltage converter status and switch freq command.
        static const constexpr uint8_t CMD_SET_OSCILLATOR_FREQ = 0xD5;   ///< Set display clock frequency command.
        static const constexpr uint8_t CMD_SET_DISP_START_LINE = 0x40;   ///< Set display starting row address command.
        static const constexpr uint8_t CMD_SET_DISP_CONTRAST = 0x81;     ///< Set display contrast command.
        static const constexpr uint8_t CMD_SET_DISP_OFFSET_MOD = 0xD3;   ///< Set display offset command.
        static const constexpr uint8_t CMD_SET_PRE_CHARGE_PERIOD = 0xD9; ///< Set precharge period command.
        static const constexpr uint8_t CMD_SET_VCOM = 0xDB;              ///< Set common pad output voltage at deselect command.
        static const constexpr uint8_t CMD_SET_VSEG = 0xDC;              ///< Set segment pad output voltage at precharge stage.
        static const constexpr uint8_t CMD_SET_DISCHARGE_LEVEL = 0x30;   ///< Set segment output discharge voltage level command.
        static const constexpr uint8_t CMD_SET_NORMAL_DISPLAY = 0xA6;    ///< Set non inverted pixel intensity command.
        static const constexpr uint8_t CMD_SET_INV_DISPLAY = 0xA7;       ///< Set inverted pixel intensity command.
        static const constexpr uint8_t CMD_SET_HIGH_COLUMN_ADDR = 0x10;  ///< Set high column address command.
        static const constexpr uint8_t CMD_SET_LOW_COLUMN_ADDR = 0x00;   ///< Set low column address command.

        static const constexpr char* TAG = "SH1122Oled";
};