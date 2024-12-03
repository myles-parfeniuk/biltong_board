#pragma once
// std library includes
#include <stdio.h>
#include <stdint.h>
#include <cstring>
#include <cstdarg>
#include <math.h>
// cube mx inclues
#include "spi.h"
// third party includes
#include "FreeRTOS.h"
#include "task.h"
// in-house includes
#include "pins.h"
#include "SH1122Oled_types.h"
#include "SerialService.h"

#define SH1122_PIXEL_IN_BOUNDS(x, y) (((x >= 0) && (x < SH1122Oled::WIDTH)) && ((y >= 0) && (y < SH1122Oled::HEIGHT)))

class SH1122Oled
{
    public:
        SH1122Oled(SPI_HandleTypeDef* hspi);
        bool init();

        void clear_buffer();
        bool update_screen();
        bool set_pixel(sh1122_pixel_t pixel, SH1122PixIntens intensity);
        bool draw_line(sh1122_pixel_t loc_start, sh1122_pixel_t loc_end, SH1122PixIntens intensity);
        void draw_rectangle_frame(sh1122_pixel_t loc_up_l_corner, int16_t width, int16_t height, int16_t thickness, SH1122PixIntens intensity);
        void draw_rectangle(sh1122_pixel_t loc_up_l_corner, int16_t width, int16_t height, SH1122PixIntens intensity);
        void draw_bitmap(sh1122_pixel_t loc_up_l_corner, const uint8_t* bitmap, SH1122PixIntens bg_intensity = SH1122PixIntens::level_transparent);
        uint16_t draw_glyph(sh1122_pixel_t loc_up_l_corner, SH1122PixIntens intensity, uint16_t encoding);
        uint16_t draw_string(sh1122_pixel_t loc_up_l_corner, SH1122PixIntens intensity, const char* format, ...);

        static void load_font(const uint8_t* font);
        void set_font_direction(SH1122FontDir dir);
        uint16_t font_get_string_width(const char* format, ...);
        uint16_t font_get_string_height(const char* format, ...);
        uint16_t font_get_glyph_width(uint16_t encoding);
        uint16_t font_get_glyph_height(uint16_t encoding);
        uint16_t font_get_string_center_x(const char* str);
        uint16_t font_get_string_center_y(const char* str);

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

        static const constexpr uint16_t WIDTH = 256U;     ///<Display width
        static const constexpr uint16_t HEIGHT = 64U;     ///<Display height
        static const constexpr uint16_t MAX_STR_SZ = 50U; ///<Max possible string size to be sent with draw_string()

    private:
        /// @brief Font information structure, used to contain information about the currently loaded font.
        typedef struct sh1122_oled_font_info_t
        {
                const uint8_t* font; ///<Pointer to first element of font lookup table.
                /* offset 0 */
                uint8_t glyph_cnt;  ///<Total amount of glyphs contained within the font.
                uint8_t bbx_mode;   ///<BBX build mode of font 0: proportional, 1: common height, 2: monospace, 3: multiple of 8
                uint8_t bits_per_0; ///< Glyph RLE (run length encoding) parameter, max bits per background line.
                uint8_t bits_per_1; ///< Glyph RLE (run length encoding) parameter, max bits per foreground line.

                /* offset 4 */
                uint8_t bits_per_char_width;  ///< Glyph RLE (run length encoding) parameter, bits per char width data.
                uint8_t bits_per_char_height; ///<Glyph RLE (run length encoding) parameter, bits per char height data.
                uint8_t bits_per_char_x;      ///<Glyph RLE (run length encoding) parameter, bits per local char x position.
                uint8_t bits_per_char_y;      ///<Glyph RLE (run length encoding) parameter, bits per local char y position.
                uint8_t bits_per_delta_x;     ///<Glyph RLE (run length encoding) parameter, bits per change in x position.

                /* offset 9 */
                int8_t max_char_width;  ///<Max glyph width of any glyphs contained within font.
                int8_t max_char_height; ///<Max glyph height of any glyphs contained within font.
                int8_t x_offset;        ///< x offset
                int8_t y_offset;        ///< y offset

                /* offset 13 */
                int8_t ascent_A;     ///< Ascent of capital A (usually glyph with highest ending y position)
                int8_t descent_g;    ///< Descent of lowercase g (usually glyph with lowest starting y position)
                int8_t ascent_para;  ///< Ascent of '(' glyph.
                int8_t descent_para; ///< Descent of ')' glyph.

                /* offset 17 */
                uint16_t start_pos_upper_A; ///< Starting offset for uppercase lookup table.
                uint16_t start_pos_lower_a; ///< Starting offset for lowercase lookup table.

                uint16_t start_pos_unicode; ///< Starting offset for unicode (16 bit encoded glyphs) lookup table.

        } sh1122_oled_font_info_t;

        /// @brief Glyph decode information structure, used to contain information about glyphs being decoded and drawn.
        typedef struct sh1122_oled_font_decode_t
        {
                const uint8_t* decode_ptr; ///< Pointer to the glyph data being decoded
                uint8_t bit_pos;           ///< Current bit position in decoding/drawing process
                int8_t glyph_width;        ///< Glyph width
                int8_t glyph_height;       ///< Glyph height
                int8_t x;                  ///< Current x position to be drawn at
                int8_t y;                  ///< Current y position to be drawn at
                uint16_t target_x;         ///< Target x position of the glyph
                uint16_t target_y;         ///< Target y position of the glyph
                int8_t glyph_x_offset;     ///< Glyph x offset used for string width calculations only
                uint8_t fg_intensity;      ///< Foreground intensity to draw
        } sh1122_oled_font_decode_t;

        bool send_cmds(uint8_t* cmds, uint16_t length);

        void bitmap_read_byte(const uint8_t** data_ptr, int16_t& r_val_lim, SH1122PixIntens& intensity);
        void bitmap_read_word(const uint8_t** data_ptr, int16_t& r_val_lim, SH1122PixIntens& intensity);
        void bitmap_decode_pixel_block(const uint8_t** data_ptr, int16_t& r_val_lim, SH1122PixIntens& intensity);

        static uint8_t font_lookup_table_read_char(const uint8_t* font, uint8_t offset);
        static uint16_t font_lookup_table_read_word(const uint8_t* font, uint8_t offset);
        const uint8_t* font_get_glyph_data(uint16_t encoding);
        uint16_t font_get_glyph_width(sh1122_oled_font_decode_t* decode, uint16_t encoding);
        void font_setup_glyph_decode(sh1122_oled_font_decode_t* decode, const uint8_t* glyph_data);
        int8_t font_decode_and_draw_glyph(sh1122_oled_font_decode_t* decode, const uint8_t* glyph_data);
        uint8_t font_decode_get_unsigned_bits(sh1122_oled_font_decode_t* decode, uint8_t cnt);
        int8_t font_decode_get_signed_bits(sh1122_oled_font_decode_t* decode, uint8_t cnt);
        uint16_t font_apply_direction_y(uint16_t dy, int8_t x, int8_t y, SH1122FontDir dir);
        uint16_t font_apply_direction_x(uint16_t dx, int8_t x, int8_t y, SH1122FontDir dir);
        void font_draw_lines(sh1122_oled_font_decode_t* decode, uint8_t len, uint8_t is_foreground);
        void font_draw_line(sh1122_oled_font_decode_t* decode, sh1122_pixel_t loc_start, uint16_t length, SH1122PixIntens intensity);
        uint16_t get_ascii_next(uint8_t b);

        SPI_HandleTypeDef* hdl_spi = nullptr;

        inline static sh1122_oled_font_info_t font_info;
        inline static SH1122FontDir font_dir = SH1122FontDir::left_to_right;

        static const constexpr uint16_t FRAME_BUFFER_LENGTH = WIDTH * HEIGHT / 2; ///< Length of frame buffer being sent over SPI.
        uint8_t frame_buffer[FRAME_BUFFER_LENGTH];                                ///< Frame buffer to contain pixel data being sent over SPI.

        static const constexpr TickType_t RST_DELAY_MS = 100UL / portTICK_PERIOD_MS;

        // bitmap decoding
        static const constexpr uint8_t BITMAP_DECODE_WORD_FLG_BIT = BIT7;    ///< Indicates word length pixel block.
        static const constexpr uint16_t BITMAP_DECODE_R_VAL_LOW_BIT_POS = 5; ///< Shift for lower repeated value bits.
        static const constexpr uint16_t BITMAP_DECODE_R_VAL_LOW_MASK =
                (BIT7 | BIT6 | BIT5); ///< Mask for isolating the 3 lsbs of repeated value count with word length pixel block.
        static const constexpr uint16_t BITMAP_DECODE_R_VAL_B_MASK =
                (BIT6 | BIT5); ///< Mask for isolating repeated value count with byte length pixel block.
        static const constexpr uint8_t BITMAP_DECODE_PIXEL_INTENSITY_MASK =
                (BIT4 | BIT3 | BIT2 | BIT1 | BIT0); ///< Mask for isolating grayscale intensity value.

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