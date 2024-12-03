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
    vTaskDelay(1UL / portTICK_PERIOD_MS);

    op_success = HAL_SPI_Transmit_DMA(hdl_spi, frame_buffer, FRAME_BUFFER_LENGTH);

    vTaskDelay(1UL / portTICK_PERIOD_MS);
    HAL_GPIO_WritePin(PIN_DISP_CS.port, PIN_DISP_CS.num, GPIO_PIN_SET); // bring chip select high

    return (op_success == HAL_OK);
}

/**
 * @brief Sets respective pixel to specified grayscale intensity.
 *
 * @param x Pixel x location.
 * @param y Pixel y location.
 * @param intensity Grayscale intensity of the drawn pixel.
 * @return True if valid pixel bounds.
 */
bool SH1122Oled::set_pixel(uint16_t x, uint16_t y, SH1122PixIntens intensity)
{
    int16_t x_it = 0;
    int16_t y_it = 0;
    int16_t high_byte = 0;
    if (SH1122_PIXEL_IN_BOUNDS(x, y))
    {
        if (intensity != SH1122PixIntens::level_transparent)
        {
            if (x != 0)
            {
                x_it = x / 2;
                high_byte = x % 2;
            }

            if (y != 0)
                y_it = (y * WIDTH) / 2;

            uint8_t* pixel = (frame_buffer + x_it + y_it);

            if (high_byte == 1)
                *pixel = ((uint8_t) intensity & 0x0F) | (*pixel & 0xF0);
            else
                *pixel = (((uint8_t) intensity << 4) & 0xF0) | (*pixel & 0x0F);
        }

        return true;
    }

    return false;
}

/**
 * @brief Draws a line between two points.
 *
 * @param x_1 Line starting x location.
 * @param y_1 Line starting y location.
 * @param x_2 Line ending x location.
 * @param y_2 Line ending y location.
 * @param intensity Grayscale intensity of the drawn line.
 * @return void, nothing to return (out of bounds pixels will be ignored)
 */
bool SH1122Oled::draw_line(int16_t x_1, int16_t y_1, int16_t x_2, int16_t y_2, SH1122PixIntens intensity)
{
    const int16_t delta_x = abs(x_2 - x_1);
    const int16_t delta_y = abs(y_2 - y_1);
    const int16_t sign_x = x_1 < x_2 ? 1 : -1;
    const int16_t sign_y = y_1 < y_2 ? 1 : -1;
    int16_t error = delta_x - delta_y;

    set_pixel(x_2, y_2, intensity);

    while (x_1 != x_2 || y_1 != y_2)
    {
        set_pixel(x_1, y_1, intensity);

        const int16_t error_2 = error * 2;

        if (error_2 > -delta_y)
        {
            error -= delta_y;
            x_1 += sign_x;
        }
        if (error_2 < delta_x)
        {
            error += delta_x;
            y_1 += sign_y;
        }
    }

    return true;
}

/**
 * @brief Draws rectangular frame at the specified location.
 *
 * @param x Frame x location (upper left corner of frame)
 * @param y Frame y location (upper left corner of frame)
 * @param width Frame width.
 * @param height Frame height.
 * @param thickness Frame thickness (drawn towards center of rectangle)
 * @param intensity Grayscale intensity of the drawn frame.
 * @return void, nothing to return (out of bounds pixels will be ignored)
 */
void SH1122Oled::draw_rectangle_frame(int16_t x_1, int16_t y_1, int16_t width, int16_t height, int16_t thickness, SH1122PixIntens intensity)
{
    for (int i = 0; i < thickness; i++)
        draw_line(x_1 + i, y_1 + thickness, x_1 + i, (y_1 + height - 1) - thickness, intensity);

    for (int i = 0; i < thickness; i++)
        draw_line((x_1 + width - 1) - i, y_1 + thickness, (x_1 + width - 1) - i, (y_1 + height - 1) - thickness, intensity);

    for (int i = 0; i < thickness; i++)
        draw_line(x_1, y_1 + i, (x_1 + width - 1), y_1 + i, intensity);

    for (int i = 0; i < thickness; i++)
        draw_line(x_1, (y_1 + height - 1) - i, (x_1 + width - 1), (y_1 + height - 1) - i, intensity);
}

/**
 * @brief Draws a filled rectangle at the specified location.
 *
 * @param x Rectangle x location (upper left corner of rectangle)
 * @param y Rectangle y location (upper left corner of rectangle)
 * @param width Rectangle width.
 * @param height Rectangle height.
 * @param intensity Grayscale intensity of the drawn rectangle.
 * @return void, nothing to return (out of bounds pixels will be ignored)
 */
void SH1122Oled::draw_rectangle(int16_t x_1, int16_t y_1, int16_t width, int16_t height, SH1122PixIntens intensity)
{
    for (uint16_t j = 0; j < height; j++)
    {
        for (uint16_t i = 0; i < width; i++)
            set_pixel((x_1 + i), (y_1 + j), intensity);
    }
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

    op_success = HAL_SPI_Transmit_DMA(hdl_spi, cmds, length);

    vTaskDelay(1UL / portTICK_PERIOD_MS);
    HAL_GPIO_WritePin(PIN_DISP_CS.port, PIN_DISP_CS.num, GPIO_PIN_SET); // bring chip select high

    return (op_success == HAL_OK);
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