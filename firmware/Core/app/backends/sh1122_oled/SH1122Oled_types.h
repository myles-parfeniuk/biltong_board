#pragma once

/// @brief Pixel intensity/grayscale level passed to drawing functions, SH1122 supports 16 different shades.
enum class SH1122PixIntens
{
    level_0,
    level_1,
    level_2,
    level_3,
    level_4,
    level_5,
    level_6,
    level_7,
    level_8,
    level_9,
    level_10,
    level_11,
    level_12,
    level_13,
    level_14,
    level_15,
    level_transparent,
    max
};

/// @brief Drawing directions for strings, passed to set_font_direction().
enum class SH1122FontDir
{
    left_to_right,
    top_to_bottom,
    right_to_left,
    bottom_to_top
};