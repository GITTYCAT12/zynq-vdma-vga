#ifndef FRAMEBUFFER_PATTERN_H
#define FRAMEBUFFER_PATTERN_H

#include <stddef.h>
#include <stdint.h>

#include "framebuffer_range.h"

/*
 * Fill an RGB888 framebuffer with eight equal vertical color bars.
 * Returns 1 on success and 0 without writing when the geometry/range is invalid.
 * This helper is portable so the pixel layout can be tested without the Xilinx BSP.
 */
static inline int Framebuffer_Fill_RGB_Color_Bars(uint8_t *buffer,
                                                   uint32_t width,
                                                   uint32_t height,
                                                   uint32_t stride)
{
    static const uint8_t palette[8][3] = {
        {255U, 255U, 255U}, /* White */
        {255U, 255U,   0U}, /* Yellow */
        {  0U, 255U, 255U}, /* Cyan */
        {  0U, 255U,   0U}, /* Green */
        {255U,   0U, 255U}, /* Magenta */
        {255U,   0U,   0U}, /* Red */
        {  0U,   0U, 255U}, /* Blue */
        {  0U,   0U,   0U}  /* Black */
    };
    uint32_t row;
    uint32_t col;
    uint32_t bar_width;

    if (buffer == NULL || width == 0U || height == 0U) {
        return 0;
    }
    if (width > (UINT32_MAX / 3U) || (width % 8U) != 0U) {
        return 0;
    }
    if (stride < (width * 3U)) {
        return 0;
    }
    if (!Framebuffer_Range_Is_Valid((uintptr_t)buffer, height, stride)) {
        return 0;
    }

    bar_width = width / 8U;
    for (row = 0U; row < height; ++row) {
        for (col = 0U; col < width; ++col) {
            uint32_t bar = col / bar_width;
            uint32_t offset = (row * stride) + (col * 3U);

            buffer[offset + 0U] = palette[bar][0];
            buffer[offset + 1U] = palette[bar][1];
            buffer[offset + 2U] = palette[bar][2];
        }
    }

    return 1;
}

#endif
