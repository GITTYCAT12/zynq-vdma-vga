#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "../vdma_vga_app/src/framebuffer_pattern.h"

static int failures = 0;

static void check(const char *name, int actual, int expected)
{
    if (actual == expected) {
        printf("PASS: %s\n", name);
    } else {
        printf("FAIL: %s (expected %d, got %d)\n", name, expected, actual);
        failures++;
    }
}

static void check_padding(const char *name, const uint8_t *buffer,
                          uint32_t height, uint32_t stride, uint32_t active_bytes)
{
    uint32_t row;
    int unchanged = 1;

    for (row = 0U; row < height; ++row) {
        uint32_t offset = (row * stride) + active_bytes;
        uint32_t byte;
        for (byte = 0U; byte < (stride - active_bytes); ++byte) {
            if (buffer[offset + byte] != 0xA5U) {
                unchanged = 0;
            }
        }
    }
    check(name, unchanged, 1);
}

int main(void)
{
    static const uint8_t expected[8][3] = {
        {255U, 255U, 255U}, {255U, 255U, 0U},
        {0U, 255U, 255U}, {0U, 255U, 0U},
        {255U, 0U, 255U}, {255U, 0U, 0U},
        {0U, 0U, 255U}, {0U, 0U, 0U}
    };
    uint8_t buffer[2U * 52U];
    uint8_t invalid_buffer[2U * 52U];
    uint32_t bar;

    memset(buffer, 0xA5, sizeof(buffer));
    check("16x2 RGB color bars are generated",
          Framebuffer_Fill_RGB_Color_Bars(buffer, 16U, 2U, 52U), 1);

    for (bar = 0U; bar < 8U; ++bar) {
        uint32_t x = bar * 2U;
        uint32_t first_row = x * 3U;
        uint32_t second_row = 52U + first_row;
        check("first-row bar color", memcmp(&buffer[first_row], expected[bar], 3U) == 0, 1);
        check("second-row bar color", memcmp(&buffer[second_row], expected[bar], 3U) == 0, 1);
    }
    check_padding("row padding is not overwritten", buffer, 2U, 52U, 48U);

    memset(invalid_buffer, 0xA5, sizeof(invalid_buffer));
    check("width not divisible by eight is rejected",
          Framebuffer_Fill_RGB_Color_Bars(invalid_buffer, 10U, 2U, 32U), 0);
    check("invalid width leaves the buffer untouched",
          invalid_buffer[0] == 0xA5U && invalid_buffer[sizeof(invalid_buffer) - 1U] == 0xA5U, 1);

    check("stride smaller than RGB row is rejected",
          Framebuffer_Fill_RGB_Color_Bars(invalid_buffer, 16U, 2U, 47U), 0);
    check("null framebuffer is rejected",
          Framebuffer_Fill_RGB_Color_Bars(NULL, 16U, 2U, 48U), 0);
    check("zero height is rejected",
          Framebuffer_Fill_RGB_Color_Bars(buffer, 16U, 0U, 48U), 0);

    if (failures != 0) {
        printf("FAIL: %d framebuffer pattern check(s) failed.\n", failures);
        return 1;
    }

    printf("PASS: framebuffer RGB pattern checks completed.\n");
    return 0;
}
