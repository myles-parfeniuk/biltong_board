#include "SH1122Oled.h"

/**
 * @brief SH1122Oled constructor.
 *
 * @param hspi Pointer to stm32CUBE HAL SPI handle to use with transmissions.
 */
SH1122Oled::SH1122Oled(SPI_HandleTypeDef* hspi)
    : hdl_spi(hspi)
{
}

/**
 * @brief Initializes display SH1122.
 *
 * @return True if initialize success.
 */
bool SH1122Oled::init()
{
    if (hdl_spi == nullptr)
        return false;

    // ensure all non-spi peripheral controlled pins are in high state
    HAL_GPIO_WritePin(PIN_DISP_RST.port, PIN_DISP_RST.num, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PIN_DISP_CS.port, PIN_DISP_CS.num, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PIN_DISP_DC.port, PIN_DISP_DC.num, GPIO_PIN_SET);

    font_info.font = nullptr;

    reset();

    // turn oled off for initialization routine
    if (!off())
        return false;

    if (!set_oscillator_freq(0x50))
        return false;

    if (!set_multiplex_ratio(HEIGHT - 1))
        return false;

    if (!set_display_offset_mod(0x00))
        return false;

    if (!set_row_addr(0x00))
        return false;

    if (!set_high_column_address(0x00))
        return false;

    if (!set_low_column_address(0x00))
        return false;

    if (!set_start_line(0x00))
        return false;

    if (!set_vseg_discharge_level(0x00))
        return false;

    if (!set_dc_dc_control_mod(0x80))
        return false;

    if (!set_segment_remap(false))
        return false;

    if (!set_orientation(false))
        return false;

    if (!set_contrast(0x90))
        return false;

    if (!set_precharge_period(0x28))
        return false;

    if (!set_vcom(0x30))
        return false;

    if (!set_vseg(0x1E))
        return false;

    if (!set_inverted_intensity(false))
        return false;

    // turn oled back on for use
    if (!on())
        return false;

    // clear screen of any artifacts
    clear_buffer();
    if (!update_screen())
        return false;

    return true;
}

/**
 * @brief Clears the buffer containing the pixel data sent to SH1122.
 *
 * @return void, nothing to return
 */
void SH1122Oled::clear_buffer()
{
    memset(frame_buffer, 0U, FRAME_BUFFER_LENGTH);
}

/**
 * @brief Updates OLED display with current frame buffer.
 *
 * Sends frame buffer to SH1122 over SPI, should be called after performing draw operations.
 *
 * @return True if SPI transmission succeeded.
 */
bool SH1122Oled::update_screen()
{
    HAL_StatusTypeDef op_success = HAL_ERROR;

    HAL_GPIO_WritePin(PIN_DISP_DC.port, PIN_DISP_DC.num, GPIO_PIN_SET); // bring oled into data mode

    HAL_GPIO_WritePin(PIN_DISP_CS.port, PIN_DISP_CS.num, GPIO_PIN_RESET); // bring chip select low


    op_success = HAL_SPI_Transmit(hdl_spi, frame_buffer, FRAME_BUFFER_LENGTH, 100UL);


    HAL_GPIO_WritePin(PIN_DISP_CS.port, PIN_DISP_CS.num, GPIO_PIN_SET); // bring chip select high

    return (op_success == HAL_OK);
}

/**
 * @brief Sets respective pixel to specified grayscale intensity.
 *
 * @param pixel Pixel to be set
 * @param intensity Grayscale intensity of the drawn pixel.
 * @return True if valid pixel bounds.
 */
bool SH1122Oled::set_pixel(sh1122_pixel_t pixel, SH1122PixIntens intensity)
{
    int16_t x_it = 0;
    int16_t y_it = 0;
    int16_t high_byte = 0;
    if (SH1122_PIXEL_IN_BOUNDS(pixel.x, pixel.y))
    {
        if (intensity != SH1122PixIntens::level_transparent)
        {
            if (pixel.x != 0)
            {
                x_it = pixel.x / 2;
                high_byte = pixel.x % 2;
            }

            if (pixel.y != 0)
                y_it = (pixel.y * WIDTH) / 2;

            uint8_t* pixel_raw_ptr = (frame_buffer + x_it + y_it);

            if (high_byte == 1)
                *pixel_raw_ptr = ((uint8_t) intensity & 0x0F) | (*pixel_raw_ptr & 0xF0);
            else
                *pixel_raw_ptr = (((uint8_t) intensity << 4) & 0xF0) | (*pixel_raw_ptr & 0x0F);
        }

        return true;
    }

    return false;
}

/**
 * @brief Draws a line between two points.
 *
 * @param loc_start Line starting pixel.
 * @param loc_end Line ending pixel.
 * @param intensity Grayscale intensity of the drawn line.
 * @return void, nothing to return (out of bounds pixels will be ignored)
 */
bool SH1122Oled::draw_line(sh1122_pixel_t loc_start, sh1122_pixel_t loc_end, SH1122PixIntens intensity)
{
    const int16_t delta_x = abs(loc_end.x - loc_start.x);
    const int16_t delta_y = abs(loc_end.y - loc_start.y);
    const int16_t sign_x = loc_start.x < loc_end.x ? 1 : -1;
    const int16_t sign_y = loc_start.y < loc_end.y ? 1 : -1;
    int16_t error = delta_x - delta_y;

    set_pixel(loc_end, intensity);

    while (loc_start.x != loc_end.x || loc_start.y != loc_end.y)
    {
        set_pixel(loc_start, intensity);

        const int16_t error_2 = error * 2;

        if (error_2 > -delta_y)
        {
            error -= delta_y;
            loc_start.x += sign_x;
        }
        if (error_2 < delta_x)
        {
            error += delta_x;
            loc_start.y += sign_y;
        }
    }

    return true;
}

/**
 * @brief Draws rectangular frame at the specified location.
 *
 * @param loc_up_l_corner Frame start location/anchor (upper left corner of frame)
 * @param width Frame width.
 * @param height Frame height.
 * @param thickness Frame thickness (drawn towards center of rectangle)
 * @param intensity Grayscale intensity of the drawn frame.
 * @return void, nothing to return (out of bounds pixels will be ignored)
 */
void SH1122Oled::draw_rectangle_frame(sh1122_pixel_t loc_up_l_corner, int16_t width, int16_t height, int16_t thickness, SH1122PixIntens intensity)
{
    sh1122_pixel_t line_start_loc;
    sh1122_pixel_t line_end_loc;

    for (int i = 0; i < thickness; i++)
    {
        line_start_loc.x = loc_up_l_corner.x + i;
        line_start_loc.y = loc_up_l_corner.y + thickness;
        line_end_loc.x = loc_up_l_corner.x + i;
        line_end_loc.y = (loc_up_l_corner.y + height - 1) - thickness;
        draw_line(line_start_loc, line_end_loc, intensity);
    }

    for (int i = 0; i < thickness; i++)
    {
        line_start_loc.x = (loc_up_l_corner.x + width - 1) - i;
        line_start_loc.y = loc_up_l_corner.y + thickness;
        line_end_loc.x = (loc_up_l_corner.x + width - 1) - i;
        line_end_loc.y = (loc_up_l_corner.y + height - 1) - thickness;
        draw_line(line_start_loc, line_end_loc, intensity);
    }

    for (int i = 0; i < thickness; i++)
    {
        line_start_loc.x = loc_up_l_corner.x;
        line_start_loc.y = loc_up_l_corner.y + i;
        line_end_loc.x = (loc_up_l_corner.x + width - 1);
        line_end_loc.y = loc_up_l_corner.y + i;
        draw_line(line_start_loc, line_end_loc, intensity);
    }

    for (int i = 0; i < thickness; i++)
    {
        line_start_loc.x = loc_up_l_corner.x;
        line_start_loc.y = (loc_up_l_corner.y + height - 1) - i;
        line_end_loc.x = (loc_up_l_corner.x + width - 1);
        line_end_loc.y = (loc_up_l_corner.y + height - 1) - i;
        draw_line(line_start_loc, line_end_loc, intensity);
    }
}

/**
 * @brief Draws a filled rectangle at the specified location.
 *
 * @param loc_up_l_corner Rectangle start location/anchor (upper left corner of Rectangle)
 * @param width Rectangle width.
 * @param height Rectangle height.
 * @param intensity Grayscale intensity of the drawn rectangle.
 * @return void, nothing to return (out of bounds pixels will be ignored)
 */
void SH1122Oled::draw_rectangle(sh1122_pixel_t loc_up_l_corner, int16_t width, int16_t height, SH1122PixIntens intensity)
{
    sh1122_pixel_t loc_local;

    for (int16_t j = 0; j < height; j++)
    {
        for (int16_t i = 0; i < width; i++)
        {
            loc_local.x = loc_up_l_corner.x + i;
            loc_local.y = loc_up_l_corner.y + j;
            set_pixel(loc_local, intensity);
        }
    }
}

/**
 * @brief Draws a sh1122 custom run-length-encoded bitmap created with sh1122_encode_bitmap.py.
 *
 * @param loc_up_l_corner Bitmap start location/anchor (upper left corner of bitmap)
 * @param bitmap Pointer to first element of bitmap to draw.
 * @param bg_intensity Background intensity (optional, default transparent), fills transparent pixels with bg_intensity if used
 *
 * @return void, nothing to return
 */
void SH1122Oled::draw_bitmap(sh1122_pixel_t loc_up_l_corner, const uint8_t* bitmap, SH1122PixIntens bg_intensity)
{
    const int16_t bitmap_col_sz = *(bitmap + 2);
    const int16_t bitmap_row_sz = *(bitmap + 3);
    const uint8_t* data_ptr = bitmap + 4;
    int16_t repeated_value_lim = 0;
    SH1122PixIntens intensity = SH1122PixIntens::level_transparent;
    int16_t repeated_value_count = 0;
    sh1122_pixel_t loc_local;

    bitmap_decode_pixel_block(&data_ptr, repeated_value_lim, intensity);

    for (int row = 0; row < bitmap_row_sz; row++)
    {
        for (int col = 0; col < bitmap_col_sz; col++)
        {
            if (intensity == SH1122PixIntens::level_transparent)
                intensity = bg_intensity;

            loc_local.x = loc_up_l_corner.x + col;
            loc_local.y = loc_up_l_corner.y + row;
            set_pixel(loc_local, intensity);
            repeated_value_count++;

            if (repeated_value_count >= repeated_value_lim)
            {
                bitmap_decode_pixel_block(&data_ptr, repeated_value_lim, intensity);
                repeated_value_count = 0;
            }
        }
    }
}

/**
 * @brief Loads a font for drawing strings and glyphs.
 *
 * @param font A pointer to the first element of the respective font lookup table, font tables are located in fonts directory.
 * @return void, nothing to return
 */
void SH1122Oled::load_font(const uint8_t* font)
{
    font_info.font = font;

    font_info.glyph_cnt = font_lookup_table_read_char(font, 0);
    font_info.bbx_mode = font_lookup_table_read_char(font, 1);
    font_info.bits_per_0 = font_lookup_table_read_char(font, 2);
    font_info.bits_per_1 = font_lookup_table_read_char(font, 3);

    font_info.bits_per_char_width = font_lookup_table_read_char(font, 4);
    font_info.bits_per_char_height = font_lookup_table_read_char(font, 5);
    font_info.bits_per_char_x = font_lookup_table_read_char(font, 6);
    font_info.bits_per_char_y = font_lookup_table_read_char(font, 7);
    font_info.bits_per_delta_x = font_lookup_table_read_char(font, 8);

    font_info.max_char_width = font_lookup_table_read_char(font, 9);
    font_info.max_char_height = font_lookup_table_read_char(font, 10);
    font_info.x_offset = font_lookup_table_read_char(font, 11);
    font_info.y_offset = font_lookup_table_read_char(font, 12);

    font_info.ascent_A = font_lookup_table_read_char(font, 13);  // capital a usually the highest pixels of any characters
    font_info.descent_g = font_lookup_table_read_char(font, 14); // lower case usually has the lowest pixels of any characters
    font_info.ascent_para = font_lookup_table_read_char(font, 15);
    font_info.descent_para = font_lookup_table_read_char(font, 16);

    font_info.start_pos_upper_A = font_lookup_table_read_word(font, 17);
    font_info.start_pos_lower_a = font_lookup_table_read_word(font, 19);
    font_info.start_pos_unicode = font_lookup_table_read_word(font, 21);
}

/**
 * @brief Sets the draw direction for strings and glyphs, default is left to right.
 *
 * @param dir The direction strings and glyphs should be drawn in, see FontDirection definition.
 * @return void, nothing to return
 */
void SH1122Oled::set_font_direction(SH1122FontDir dir)
{
    font_dir = dir;
}

/**
 * @brief Returns the width of specified string using the currently loaded font.
 *
 * @param format The string for which width is desired, supports variable arguments (ie printf style formatting)
 * @return The width of the specified string.
 */
uint16_t SH1122Oled::font_get_string_width(const char* format, ...)
{
    uint16_t encoding;
    uint16_t width;
    uint16_t dx;
    int8_t initial_x_offset = -64;
    sh1122_oled_font_decode_t decode;
    int16_t chars_written = 0U;
    uint8_t* str = nullptr;
    char str_to_send[MAX_STR_SZ] = {};
    va_list args;

    // cannot get string width without font info
    if (font_info.font == nullptr)
    {
        SerialService::LOG_ln<BB_LL_ERROR>(TAG, "No font loaded.");
        return 0;
    }

    va_start(args, format);
    chars_written = vsnprintf(str_to_send, MAX_STR_SZ, format, args);
    va_end(args);

    if (chars_written >= MAX_STR_SZ)
        return 0;

    str = reinterpret_cast<uint8_t*>(str_to_send);
    width = 0;
    dx = 0;

    while (1)
    {
        encoding = get_ascii_next(*str); // get next character

        if (encoding == 0x0ffff)
            break;
        if (encoding != 0x0fffe)
        {
            dx = font_get_glyph_width(&decode, encoding); // get the glyph width
            if (initial_x_offset == -64)
                initial_x_offset = decode.glyph_x_offset;

            width += dx; // increment width counter
        }
        str++; // increment string pointer to next glyph
    }

    // if glyph_width is greater than 0, apply the respective glyph x offset.
    if (decode.glyph_width != 0)
    {
        width -= dx;
        width += decode.glyph_width;
        width += decode.glyph_x_offset;
        if (initial_x_offset > 0)
            width += initial_x_offset;
    }

    return width;
}

/**
 * @brief Returns the height (tallest character height) of specified string using the currently loaded font.
 *
 * @param format The string for which height is desired, supports variable arguments (ie printf style formatting)
 * @return The width of the specified string.
 */
uint16_t SH1122Oled::font_get_string_height(const char* format, ...)
{
    uint16_t current_height = 0U;
    uint16_t max_height = 0U;
    uint16_t encoding = 0U;
    int16_t chars_written = 0U;
    uint8_t* str = nullptr;
    char str_to_send[MAX_STR_SZ] = {};
    va_list args;

    // cannot get string width without font info
    if (font_info.font == nullptr)
    {
        SerialService::LOG_ln<BB_LL_ERROR>(TAG, "No font loaded.");
        return 0;
    }

    va_start(args, format);
    chars_written = vsnprintf(str_to_send, MAX_STR_SZ, format, args);
    va_end(args);

    if (chars_written >= MAX_STR_SZ)
        return 0;

    str = reinterpret_cast<uint8_t*>(str_to_send);

    while (1)
    {
        encoding = get_ascii_next(*str); // get next character

        if (encoding == 0x0ffff)
            break;

        if (encoding != 0x0fffe)
        {
            current_height = font_get_glyph_height(*str);
            // if the current height is greater than the largest height detected
            if (current_height > max_height)
                max_height = current_height; // overwrite max_height with tallest character height detected
        }

        str++; // increment string pointer to next glyph
    }

    return max_height;
}

/**
 * @brief Returns the width of specified glyph using the currently loaded font.
 *
 * @param encoding The encoding of the character for which width is desired, supports UTF-8 and UTF-16.
 * @return The width of the specified glyph.
 */
uint16_t SH1122Oled::font_get_glyph_width(uint16_t encoding)
{
    const uint8_t* glyph_data;
    sh1122_oled_font_decode_t decode;

    if (font_info.font == nullptr)
    {
        SerialService::LOG_ln<BB_LL_ERROR>(TAG, "No font loaded.");
        return 0;
    }

    glyph_data = font_get_glyph_data(encoding);

    if (glyph_data == NULL)
        return 0;

    font_setup_glyph_decode(&decode, glyph_data);
    font_decode_get_signed_bits(&decode, font_info.bits_per_char_x);
    font_decode_get_signed_bits(&decode, font_info.bits_per_char_y);

    return font_decode_get_signed_bits(&decode, font_info.bits_per_delta_x);
}

/**
 * @brief Returns the width of specified glyph using the currently loaded font. Overloaded with decode structure for calls to font_get_string_width()
 *
 * @param decode The decode structure to save the glyph x offset in, for use within get_string_width()
 * @param encoding The encoding of the character for which width is desired, supports UTF-8 and UTF-16.
 * @return The width of the specified glyph.
 */
uint16_t SH1122Oled::font_get_glyph_width(sh1122_oled_font_decode_t* decode, uint16_t encoding)
{
    const uint8_t* glyph_data;

    if (font_info.font == nullptr)
    {
        SerialService::LOG_ln<BB_LL_ERROR>(TAG, "No font loaded.");
        return 0;
    }

    glyph_data = font_get_glyph_data(encoding);

    if (glyph_data == NULL)
        return 0;

    font_setup_glyph_decode(decode, glyph_data);
    decode->glyph_x_offset = font_decode_get_signed_bits(decode, font_info.bits_per_char_x);
    font_decode_get_signed_bits(decode, font_info.bits_per_char_y);

    return font_decode_get_signed_bits(decode, font_info.bits_per_delta_x);
}

/**
 * @brief Returns the height of specified glyph using the currently loaded font.
 *
 * @param encoding The encoding of the character for which height is desired, supports UTF-8 and UTF-16.
 * @return The height of the specified glyph.
 */
uint16_t SH1122Oled::font_get_glyph_height(uint16_t encoding)
{
    const uint8_t* glyph_data;
    sh1122_oled_font_decode_t decode;

    if (font_info.font == nullptr)
    {
        SerialService::LOG_ln<BB_LL_ERROR>(TAG, "No font loaded.");
        return 0;
    }

    glyph_data = font_get_glyph_data(encoding);

    if (glyph_data == NULL)
        return 0;

    font_setup_glyph_decode(&decode, glyph_data);

    return decode.glyph_height;
    return 0;
}

/**
 * @brief Returns the x position required to horizontally center a given string.
 *
 * @param str The string for which a horizontal centering is desired.
 * @return The x position the string should be drawn at to center it horizontally.
 */
uint16_t SH1122Oled::font_get_string_center_x(const char* str)
{
    uint16_t str_width;

    if (font_info.font == nullptr)
    {
        SerialService::LOG_ln<BB_LL_ERROR>(TAG, "No font loaded.");
        return 0;
    }

    str_width = font_get_string_width(str);

    return (WIDTH - str_width) / 2;
}

/**
 * @brief Returns the y position required to vertically center a given string.
 *
 * @param str The string for which a vertical centering is desired.
 * @return The y position the string should be drawn at to center it vertically.
 */
uint16_t SH1122Oled::font_get_string_center_y(const char* str)
{
    uint16_t max_char_height;

    if (font_info.font == nullptr)
    {
        SerialService::LOG_ln<BB_LL_ERROR>(TAG, "No font loaded.");
        return 0;
    }

    max_char_height = font_get_string_height(str);

    return (HEIGHT - max_char_height) / 2;
}

/**
 * @brief Draws the selected glyph/character using the currently loaded font.
 *
 * @param loc_up_l_corner Glyph start location/anchor (upper left corner of glyph)
 * @param intensity Grayscale intensity of the drawn glyph
 * @param encoding The encoding of the character to be drawn, supports UTF-8 and UTF-16.
 * @return The change in x required to draw the next glyph in string without overlapping.
 */
uint16_t SH1122Oled::draw_glyph(sh1122_pixel_t loc_up_l_corner, SH1122PixIntens intensity, uint16_t encoding)
{
    const uint8_t* glyph_ptr = NULL;
    sh1122_oled_font_decode_t decode;
    uint16_t dx = 0;

    // must load font before attempting to write glyphs
    if (font_info.font == nullptr)
    {
        SerialService::LOG_ln<BB_LL_ERROR>(TAG, "No font loaded.");
        return 0;
    }

    // set up the decode structure
    decode.target_x = loc_up_l_corner.x;

    switch (font_dir)
    {
        case SH1122FontDir::left_to_right:
            loc_up_l_corner.y += font_info.ascent_A;
            break;

        case SH1122FontDir::top_to_bottom:
            break;

        case SH1122FontDir::right_to_left:
            break;

        case SH1122FontDir::bottom_to_top:
            decode.target_x += font_info.ascent_A;
            break;
    }

    decode.target_y = loc_up_l_corner.y;
    decode.fg_intensity = (uint8_t) intensity;

    glyph_ptr = NULL;

    if (encoding != 0x0ffff)
    {
        glyph_ptr = font_get_glyph_data(encoding); // get glyph data from lookup table
        if (glyph_ptr != NULL)
        {
            font_setup_glyph_decode(&decode, glyph_ptr);         // setup decode structure with important values from table
            dx = font_decode_and_draw_glyph(&decode, glyph_ptr); // decode and draw the glyph
        }
    }

    return dx;
}

/**
 * @brief Draws a string at the specified location using the currently loaded font.
 *
 * @param loc_up_l_corner String start location/anchor (upper left corner of string)
 * @param intensity Grayscale intensity of the drawn string
 * @param format The string to be drawn, supports variable arguments (ie printf style formatting)
 * @return The width of the drawn string.
 */
uint16_t SH1122Oled::draw_string(sh1122_pixel_t loc_up_l_corner, SH1122PixIntens intensity, const char* format, ...)
{
    uint16_t delta = 0;
    uint16_t encoding = 0;
    uint16_t sum = 0;
    int16_t chars_written = 0U;
    char str_to_send[MAX_STR_SZ] = {};
    uint8_t* str = nullptr;
    va_list args;

    va_start(args, format);
    chars_written = vsnprintf(str_to_send, MAX_STR_SZ, format, args);
    va_end(args);

    if (chars_written >= MAX_STR_SZ)
        return 0;

    str = reinterpret_cast<uint8_t*>(str_to_send);

    while (1)
    {
        encoding = get_ascii_next(*str); // check to ensure character is not null or new line (end of string)

        if (encoding == 0x0ffff)
            break;

        if (encoding != 0x0fffe)
        {
            delta = draw_glyph(loc_up_l_corner, intensity, encoding);

            switch (font_dir)
            {
                case SH1122FontDir::left_to_right:
                    loc_up_l_corner.x += delta;
                    break;

                case SH1122FontDir::top_to_bottom:
                    loc_up_l_corner.y += delta;
                    break;

                case SH1122FontDir::right_to_left:
                    loc_up_l_corner.x -= delta;
                    break;

                case SH1122FontDir::bottom_to_top:
                    loc_up_l_corner.y -= delta;
                    break;
            }

            sum += delta;
        }

        str++;
    }

    return sum;
}

/**
 * @brief Gives the width of the bitmap, for spacing purposes.
 *
 * @return Returns a uint16_t for the width
 */
uint16_t SH1122Oled::bitmap_get_width(const uint8_t* bitmap)
{
    return static_cast<uint16_t>(*(bitmap + 3));
}

/**
 * @brief Hard resets the SH1122 using the RST pin.
 *
 * @return void, nothing to return
 */
void SH1122Oled::reset()
{

    HAL_GPIO_WritePin(PIN_DISP_RST.port, PIN_DISP_RST.num, GPIO_PIN_RESET); // bring oled into reset (rst low)
    vTaskDelay(RST_DELAY_MS);                                               // wait RST_DELAY_MS
    HAL_GPIO_WritePin(PIN_DISP_RST.port, PIN_DISP_RST.num, GPIO_PIN_SET);   // bring oled out of reset (rst high)
    vTaskDelay(RST_DELAY_MS);                                               // wait RST_DELAY_MS to boot
}

/**
 * @brief Sends power off command.
 *
 * @return True if SPI transmission succeeded.
 */
bool SH1122Oled::off()
{
    uint8_t cmd = CMD_POWER_OFF;
    return send_cmds(&cmd, 1);
}

/**
 * @brief Sends power on command.
 *
 * @return True if SPI transmission succeeded.
 */
bool SH1122Oled::on()
{
    uint8_t cmd = CMD_POWER_ON;
    return send_cmds(&cmd, 1);
}

/**
 * @brief Sends commands to SH1122 over SPI.
 *
 * @param cmds Pointer to buffer containing commands to be sent.
 * @param length Total length of command buffer.
 * @return True if SPI transmission succeeded.
 */
bool SH1122Oled::send_cmds(uint8_t* cmds, uint16_t length)
{
    HAL_StatusTypeDef op_success = HAL_ERROR;

    HAL_GPIO_WritePin(PIN_DISP_DC.port, PIN_DISP_DC.num, GPIO_PIN_RESET); // bring oled into command mode

    HAL_GPIO_WritePin(PIN_DISP_CS.port, PIN_DISP_CS.num, GPIO_PIN_RESET); // bring chip select low
    vTaskDelay(1UL / portTICK_PERIOD_MS);

    op_success = HAL_SPI_Transmit(hdl_spi, cmds, length, 100UL);

    vTaskDelay(1UL / portTICK_PERIOD_MS);
    HAL_GPIO_WritePin(PIN_DISP_CS.port, PIN_DISP_CS.num, GPIO_PIN_SET); // bring chip select high

    return (op_success == HAL_OK);
}

void SH1122Oled::bitmap_read_byte(const uint8_t** data_ptr, int16_t& r_val_lim, SH1122PixIntens& intensity)
{
    intensity = static_cast<SH1122PixIntens>(**data_ptr & BITMAP_DECODE_PIXEL_INTENSITY_MASK);
    r_val_lim = static_cast<int16_t>((**data_ptr & BITMAP_DECODE_R_VAL_B_MASK) >> 5);
    *data_ptr += 1;
}

void SH1122Oled::bitmap_read_word(const uint8_t** data_ptr, int16_t& r_val_lim, SH1122PixIntens& intensity)
{
    intensity = static_cast<SH1122PixIntens>(*(*data_ptr + 1) & BITMAP_DECODE_PIXEL_INTENSITY_MASK);
    r_val_lim = static_cast<int16_t>(
            ((**data_ptr & ~BITMAP_DECODE_WORD_FLG_BIT) << 3) | (int16_t) (((*(*data_ptr + 1)) & BITMAP_DECODE_R_VAL_LOW_MASK) >> 5));
    *data_ptr += 2;
}

/**
 * @brief Decodes a single pixel block (an intensity and the amount of pixels it repeats) from sh1122 RLE bitmap data.
 *
 * @param data_ptr Pointer to current pixel block in bitmap data. Incremented after read is completed to next pixel block.
 * @param r_val_lim Repeated value limit returned from data, total amount of pixels returned intensity repeats for.
 * @param intensity Grayscale intensity value returned from data.
 * @return void, nothing to return
 */
void SH1122Oled::bitmap_decode_pixel_block(const uint8_t** data_ptr, int16_t& r_val_lim, SH1122PixIntens& intensity)
{
    if (**data_ptr & BITMAP_DECODE_WORD_FLG_BIT)
        bitmap_read_word(data_ptr, r_val_lim, intensity);
    else
        bitmap_read_byte(data_ptr, r_val_lim, intensity);
}

/**
 * @brief Reads an 8 bit value from specified font lookup table.
 *
 * @param font Pointer to first element of font lookup table.
 * @param offset Offset from initial address of lookup table to element desired to be read.
 * @return Read 8-bit value read from lookup table.
 */
uint8_t SH1122Oled::font_lookup_table_read_char(const uint8_t* font, uint8_t offset)
{
    return *static_cast<const uint8_t*>(font + offset);
}

/**
 * @brief Reads a 16 bit value from specified font lookup table.
 *
 * @param font Pointer to first element of font lookup table.
 * @param offset Offset from initial address of lookup table to element desired to be read.
 * @return Read 16-bit value read from lookup table.
 */
uint16_t SH1122Oled::font_lookup_table_read_word(const uint8_t* font, uint8_t offset)
{
    uint16_t word;

    word = static_cast<uint16_t>(*static_cast<const uint8_t*>(font + offset));
    word <<= 8;
    word += static_cast<uint16_t>(*static_cast<const uint8_t*>(font + offset + 1));

    return word;
}

/**
 * @brief Returns pointer to glyph data from current font lookup table.
 *
 * @param encoding Encoding of the glyph data is desired for.
 * @return a pointer to the first element of respective glyph data, NULL if not found.
 */
const uint8_t* SH1122Oled::font_get_glyph_data(uint16_t encoding)
{
    const uint8_t* glyph_ptr = font_info.font;
    const uint8_t* unicode_lookup_table = font_info.font;
    glyph_ptr += 23;
    unicode_lookup_table += 23 + font_info.start_pos_unicode;
    uint16_t unicode = 0;

    if (encoding <= 255)
    {
        if (encoding >= 'a')
            glyph_ptr += font_info.start_pos_lower_a;
        else if (encoding >= 'A')
            glyph_ptr += font_info.start_pos_upper_A;

        while (1)
        {
            if (*(glyph_ptr + 1) == 0)
            {
                glyph_ptr = NULL;
                break; // exit loop, reached end of font data and could not find glyph
            }
            else if (*glyph_ptr == encoding)
            {
                glyph_ptr += 2; // skip encoding and glyph size
                break;
            }

            glyph_ptr += *(glyph_ptr + 1);
        }
    }
    else
    {
        glyph_ptr += font_info.start_pos_unicode;

        do
        {
            glyph_ptr += font_lookup_table_read_word(unicode_lookup_table, 0);
            unicode = font_lookup_table_read_word(unicode_lookup_table, 2);
            unicode_lookup_table += 4;
        } while (unicode < encoding);

        while (1)
        {
            unicode = font_lookup_table_read_char(glyph_ptr, 0);
            unicode <<= 8;
            unicode |= font_lookup_table_read_char(glyph_ptr, 1);

            if (unicode == 0)
            {
                glyph_ptr = NULL;
                break;
            }

            if (unicode == encoding)
            {
                glyph_ptr += 3;
                break;
            }

            glyph_ptr += font_lookup_table_read_char(glyph_ptr, 2);
        }
    }

    return glyph_ptr;
}

/**
 * @brief Sets up glyph decoding structure with values from glyph data for decoding process.
 *
 * @param encoding Encoding of the glyph being decoded.
 * @return void, nothing to return
 */
void SH1122Oled::font_setup_glyph_decode(sh1122_oled_font_decode_t* decode, const uint8_t* glyph_data)
{
    decode->decode_ptr = glyph_data;
    decode->bit_pos = 0;

    decode->glyph_width = font_decode_get_unsigned_bits(decode, font_info.bits_per_char_width);
    decode->glyph_height = font_decode_get_unsigned_bits(decode, font_info.bits_per_char_height);
}

/**
 * @brief Decodes and draws a single glyph/character.
 *
 * @param decode Pointer to decode structure containing information about the glyph to be drawn.
 * @param glyph_data Pointer to the initial element of the data for the glyph to be drawn.
 * @return Amount for which x should be incremented before drawing the next glyph.
 */
int8_t SH1122Oled::font_decode_and_draw_glyph(sh1122_oled_font_decode_t* decode, const uint8_t* glyph_data)
{
    uint8_t bg_line_length, fg_line_length;
    int8_t x, y;
    int8_t d;
    int8_t h;

    h = decode->glyph_height;

    x = font_decode_get_signed_bits(decode, font_info.bits_per_char_x);
    y = font_decode_get_signed_bits(decode, font_info.bits_per_char_y);
    d = font_decode_get_signed_bits(decode, font_info.bits_per_delta_x);

    if (decode->glyph_width > 0)
    {
        // decode->target_x += x;
        // decode->target_y -= y + h;
        decode->target_x = font_apply_direction_x(decode->target_x, x, -(h + y), font_dir);
        decode->target_y = font_apply_direction_y(decode->target_y, x, -(h + y), font_dir);
        decode->x = 0;
        decode->y = 0;

        while (1)
        {
            bg_line_length = font_decode_get_unsigned_bits(decode, font_info.bits_per_0); // bits per background line
            fg_line_length = font_decode_get_unsigned_bits(decode, font_info.bits_per_1); // bits per foreground line

            do
            {
                font_draw_lines(decode, bg_line_length, 0);
                font_draw_lines(decode, fg_line_length, 1);
            } while (font_decode_get_unsigned_bits(decode, 1) != 0);

            if (decode->y >= h)
                break;
        }
    }

    return d;
}

/**
 * @brief Decodes bit values from font lookup table and returns them as an unsigned integer.
 *
 * @param cnt Amount to increment current bit position by.
 * @return Unsigned decoded values.
 */
uint8_t SH1122Oled::font_decode_get_unsigned_bits(sh1122_oled_font_decode_t* decode, uint8_t cnt)
{
    uint8_t val;
    uint8_t bit_pos = decode->bit_pos;
    uint8_t bit_pos_plus_cnt;
    uint8_t s = 8;

    val = *decode->decode_ptr; // value of element in font lookup table currently being decoded
    val >>= bit_pos;           // shift by current bit position such that only bits with positions greater than the current position are decoded

    // find next bit position
    bit_pos_plus_cnt = bit_pos;
    bit_pos_plus_cnt += cnt;

    // if the next bit position falls within next font lookup table element
    if (bit_pos_plus_cnt >= 8)
    {
        s -= bit_pos;         // subtract starting bit position from element width (8 bits) to determine how many bits lay within next element
        decode->decode_ptr++; // increment to next element of lookup table
        val |= *decode->decode_ptr << (s); // set the unoccupied bits of val to bits to be decoded in next element
        bit_pos_plus_cnt -= 8;             // subtract the width of a lookup table element to account for moving to next element
    }

    val &= (1U << cnt) - 1; // clear bits of value that were not used, result is undecoded value

    decode->bit_pos = bit_pos_plus_cnt; // save next bit position to decode structure

    return val; // return the decoded value
}

/**
 * @brief Decodes bit values from font lookup table and returns them as a signed integer.
 *
 * @param cnt Amount to increment current bit position by.
 * @return Signed decoded values.
 */
int8_t SH1122Oled::font_decode_get_signed_bits(sh1122_oled_font_decode_t* decode, uint8_t cnt)
{
    int8_t val;
    int8_t d;
    val = static_cast<int8_t>(font_decode_get_unsigned_bits(decode, cnt));
    d = 1;
    cnt--;
    d <<= cnt;
    val -= d;

    return val;
}

/**
 * @brief Apply rotation to y coordinate of lines being drawn for current glyph such that they match passed direction.
 *
 * @param dy Target y position of the glyph being drawn.
 * @param x Local x position of glyph being drawn.
 * @param y Local y position of glyph being drawn.
 * @param dir The desired drawing direction of the font, default is left to right.
 * @return Rotated y value.
 */
uint16_t SH1122Oled::font_apply_direction_y(uint16_t dy, int8_t x, int8_t y, SH1122FontDir dir)
{
    switch (dir)
    {
        case SH1122FontDir::left_to_right:
            dy += y;
            break;

        case SH1122FontDir::top_to_bottom:
            dy += x;
            break;

        case SH1122FontDir::right_to_left:
            dy -= y;
            break;

        case SH1122FontDir::bottom_to_top:
            dy -= x;
            break;
    }

    return dy;
}

/**
 * @brief Apply rotation to x coordinate of lines being drawn for current glyph such that they match passed direction.
 *
 * @param dx Target x position of the glyph being drawn.
 * @param x Local x position of glyph being drawn.
 * @param y Local y position of glyph being drawn.
 * @param dir The desired drawing direction of the font, default is left to right.
 * @return Rotated x value.
 */
uint16_t SH1122Oled::font_apply_direction_x(uint16_t dx, int8_t x, int8_t y, SH1122FontDir dir)
{
    switch (dir)
    {
        case SH1122FontDir::left_to_right:
            dx += x;
            break;

        case SH1122FontDir::top_to_bottom:
            dx -= y;
            break;

        case SH1122FontDir::right_to_left:
            dx -= x;
            break;

        case SH1122FontDir::bottom_to_top:
            dx += y;
            break;
    }

    return dx;
}

/**
 * @brief Draws a single font line, called from font_draw_lines
 *
 * @param decode Pointer to decode structure containing information about the glyph currently being drawn.
 * @param x Starting x position of line.
 * @param y Starting y position of line.
 * @param length Length of line in pixels.
 * @param intensity Grayscale intensity of the line being drawn.
 * @return void, nothing to return
 */
void SH1122Oled::font_draw_line(sh1122_oled_font_decode_t* decode, sh1122_pixel_t loc_start, uint16_t length, SH1122PixIntens intensity)
{
    sh1122_pixel_t loc_end;

    if (length != 0)
    {
        if (!SH1122_PIXEL_IN_BOUNDS(loc_start.x, loc_start.y))
            return;

        switch (font_dir)
        {
            case SH1122FontDir::left_to_right:
                loc_end.x = loc_start.x + (length - 1);
                loc_end.y = loc_start.y;
                draw_line(loc_start, loc_end, intensity);
                break;

            case SH1122FontDir::top_to_bottom:
                loc_end.x = loc_start.x;
                loc_end.y = loc_start.y + (length - 1);
                draw_line(loc_start, loc_end, intensity);
                break;

            case SH1122FontDir::right_to_left:
                loc_end.x = loc_start.x - (length - 1);
                loc_end.y = loc_start.y;
                draw_line(loc_end, loc_start, intensity);
                break;

            case SH1122FontDir::bottom_to_top:
                loc_end.x = loc_start.x;
                loc_end.y = loc_start.y - (length - 1);
                draw_line(loc_start, loc_end, intensity);
                break;
        }
    }
}

/**
 * @brief Draws lines/pixels for glyph being drawn.
 *
 * @param decode Pointer to decode structure containing information about the glyph currently being drawn.
 * @param len Total number of pixels which must be drawn for specified glyph.
 * @param is_foreground 0 if pixels are background/whitespace around the glyph, 1 if the actual glyph content to be drawn.
 * @return void, nothing to return
 */
void SH1122Oled::font_draw_lines(sh1122_oled_font_decode_t* decode, uint8_t len, uint8_t is_foreground)
{
    uint8_t cnt;     /* total number of remaining pixels, which have to be drawn */
    uint8_t rem;     /* remaining pixel to the right edge of the glyph */
    uint8_t current; /* number of pixels, which need to be drawn for the draw procedure */
                     /* current is either equal to cnt or equal to rem */

    /* local coordinates of the glyph */
    sh1122_pixel_t loc_local;

    /* target position on the screen */
    sh1122_pixel_t loc_anchor;

    cnt = len;

    /*get the local position*/
    loc_local.x = decode->x;
    loc_local.y = decode->y;

    while (1)
    {
        /*calculate the number of pixels to the right edge of the glyph*/
        rem = decode->glyph_width;
        rem -= loc_local.x;

        /*calculate how many pixels to draw*/
        current = rem;
        if (cnt < rem)
            current = cnt;

        loc_anchor.x = decode->target_x;
        loc_anchor.y = decode->target_y;

        loc_anchor.x = font_apply_direction_x(loc_anchor.x, loc_local.x, loc_local.y, font_dir);
        loc_anchor.y = font_apply_direction_y(loc_anchor.y, loc_local.x, loc_local.y, font_dir);

        if (is_foreground)
            font_draw_line(decode, loc_anchor, current, static_cast<SH1122PixIntens>(decode->fg_intensity));

        if (cnt < rem)
            break;

        cnt -= rem;
        loc_local.x = 0;
        loc_local.y++;
    }

    loc_local.x += cnt;
    decode->x = loc_local.x;
    decode->y = loc_local.y;
}

/**
 * @brief Checks a passed glyph for EOL conditions.
 *
 * @param b Encoding of the glyph being checked.
 * @return ascii value/encoding of the glyph, 0x0ffff if EOL.
 */
uint16_t SH1122Oled::get_ascii_next(uint8_t b)
{
    if (b == 0 || b == '\n')
        return 0x0ffff;
    else
        return b;
}

/**
 * @brief Sets clock divide ratio/oscillator frequency of internal display clocks. Display must be powered off before calling.
 *
 * @param freq_reg_val Clock divide ratio register value, see section 17 of commands in SH1122 datasheet.
 * @return True if SPI transmission succeeded.
 */
bool SH1122Oled::set_oscillator_freq(uint8_t freq_reg_val)
{
    uint8_t cmds[2] = {CMD_SET_OSCILLATOR_FREQ, freq_reg_val};
    return send_cmds(cmds, 2);
}

/**
 * @brief Sets the multiplex ratio of the display. Display must be powered off before calling.
 *
 * @param multiplex_ratio_reg_val Desired multiplex ratio step, from 1 to 64
 * @return True if SPI transmission succeeded.
 */
bool SH1122Oled::set_multiplex_ratio(uint8_t multiplex_ratio_reg_val)
{
    uint8_t cmds[2] = {CMD_SET_MULTIPLEX_RATION, multiplex_ratio_reg_val};
    return send_cmds(cmds, 2);
}

/**
 * @brief Sets display offset modifier. Display must be powered off before calling.
 *
 * @param mod Offset modifier value, see section 16 of commands in SH1122 datasheet.
 * @return True if SPI transmission succeeded.
 */
bool SH1122Oled::set_display_offset_mod(uint8_t mod)
{
    uint8_t cmds[2] = {CMD_SET_DISP_OFFSET_MOD, mod};
    return send_cmds(cmds, 2);
}

/**
 * @brief Sets the current row address in display internal RAM. Display must be powered off before calling.
 *
 * @param row_addr Desired row address from 0x00 (POR) to 0x3F.
 * @return True if SPI transmission succeeded.
 */
bool SH1122Oled::set_row_addr(uint8_t row_addr)
{
    uint8_t cmds[2] = {CMD_SET_ROW_ADDR, row_addr};
    return send_cmds(cmds, 2);
}

/**
 * @brief Sets the high column address of display. Display must be powered off before calling.
 *
 * @param high_column_addr High column address desired, from 0x10 to 0x17. (column address is 7 bits, with 3 msbs in higher column address register)
 * @return True if SPI transmission succeeded.
 */
bool SH1122Oled::set_high_column_address(uint8_t high_column_addr)
{
    uint8_t cmd = CMD_SET_HIGH_COLUMN_ADDR | high_column_addr;
    return send_cmds(&cmd, 1);
}

/**
 * @brief Sets the low column address of display. Display must be powered off before calling.
 *
 * @param low_column_addr Low column address desired, from 0x00 to 0x0F. (column address is 7 bits, with 4 lsbs in lower column address register)
 * @return True if SPI transmission succeeded.
 */
bool SH1122Oled::set_low_column_address(uint8_t low_column_addr)
{
    uint8_t cmd = CMD_SET_LOW_COLUMN_ADDR | low_column_addr;
    return send_cmds(&cmd, 1);
}

/**
 * @brief Sets the row address to be used as initial display line/COM0. Display must be powered off before calling.
 *
 * @param start_line Desired starting row address, from 0x00 (POR) to 0x3F.
 * @return True if SPI transmission succeeded.
 */
bool SH1122Oled::set_start_line(uint8_t start_line)
{
    uint8_t cmd = CMD_SET_DISP_START_LINE | (start_line & 0x3F);
    return send_cmds(&cmd, 1);
}

/**
 * @brief Sets segment output discharge voltage level. Display must be powered off before calling.
 *
 * @param discharge_level VSEGM discharge level register value, see section 21 of commands in SH1122 datasheet.
 * @return True if SPI transmission succeeded.
 */
bool SH1122Oled::set_vseg_discharge_level(uint8_t discharge_level)
{
    uint8_t cmd = CMD_SET_DISCHARGE_LEVEL | discharge_level;
    return send_cmds(&cmd, 1);
}

/**
 * @brief Sets the DC-DC voltage converter status and switch frequency. Display must be powered off before calling.
 *
 * @param mod DC-DC register value, see section 12 of commands in SH1122 datasheet.
 * @return True if SPI transmission succeeded.
 */
bool SH1122Oled::set_dc_dc_control_mod(uint8_t mod)
{
    uint8_t cmds[2] = {CMD_SET_DC_DC_CTRL_MOD, mod};
    return send_cmds(cmds, 2);
}

/**
 * @brief Change relationship between RAM column address and segment driver. Display must be powered off before calling.
 *
 * @param remapped True if remapped segment scheme is desired, false if otherwise. See section 8 of commands in SH1122 datasheet.
 * @return True if SPI transmission succeeded.
 */
bool SH1122Oled::set_segment_remap(bool remapped)
{
    uint8_t cmd = (remapped) ? (CMD_NORM_SEG_MAP | 0x01) : (CMD_NORM_SEG_MAP);
    return send_cmds(&cmd, 1);
}

/**
 * @brief Changes scan direction of display from 0 to N, to N to 0, thus vertically flipping display. Display must be powered off before calling.
 *
 * @param flipped True if a flipped orientation is desired, false if otherwise.
 * @return True if SPI transmission succeeded.
 */
bool SH1122Oled::set_orientation(bool flipped)
{
    uint8_t cmd = (flipped) ? CMD_SCAN_N_TO_0 : CMD_SCAN_0_TO_N;
    return send_cmds(&cmd, 1);
}

/**
 * @brief Sets the contrast of the display. Display must be powered off before calling.
 *
 * @param contrast_reg_val The desired contrast, SH1122 has 256 contrast steps from 0x00 to 0xFF.
 * @return True if SPI transmission succeeded.
 */
bool SH1122Oled::set_contrast(uint8_t contrast_reg_val)
{
    uint8_t cmds[2] = {CMD_SET_DISP_CONTRAST, contrast_reg_val};
    return send_cmds(cmds, 2);
}

/**
 * @brief Sets duration of precharge/discharge period of display. Display must be powered off before calling.
 *
 * @param period_reg_val Period register value, see section 18 of commands in SH1122 datasheet.
 * @return True if SPI transmission succeeded.
 */
bool SH1122Oled::set_precharge_period(uint8_t period_reg_val)
{
    uint8_t cmds[2] = {CMD_SET_PRE_CHARGE_PERIOD, period_reg_val};
    return send_cmds(cmds, 2);
}

/**
 * @brief Sets common pad output voltage of display at deselect stage. Display must be powered off before calling.
 *
 * @param vcom_reg_val VCOM deselect level register value, see section 19 of commands in SH1122 datasheet.
 * @return True if SPI transmission succeeded.
 */
bool SH1122Oled::set_vcom(uint8_t vcom_reg_val)
{
    uint8_t cmds[2] = {CMD_SET_VCOM, vcom_reg_val};
    return send_cmds(cmds, 2);
}

/**
 * @brief Sets segment pad output voltage level at precharge stage. Display must be powered off before calling.
 *
 * @param vseg_reg_val VSEGM precharge level register value, see section 20 of commands in SH1122 datasheet.
 * @return True if SPI transmission succeeded.
 */
bool SH1122Oled::set_vseg(uint8_t vseg_reg_val)
{
    uint8_t cmds[2] = {CMD_SET_VSEG, vseg_reg_val};
    return send_cmds(cmds, 2);
}

/**
 * @brief Inverts display pixel intensity. Display must be powered off before calling.
 *
 * @param inverted True if an inverted pixel intensity id desired, false if otherwise.
 * @return True if SPI transmission succeeded.
 */
bool SH1122Oled::set_inverted_intensity(bool inverted)
{
    uint8_t cmd = (inverted) ? CMD_SET_INV_DISPLAY : CMD_SET_NORMAL_DISPLAY;
    return send_cmds(&cmd, 1);
}