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

static int all_pixels_match(const uint8_t *buffer,
                            uint32_t width,
                            uint32_t height,
                            uint32_t stride,
                            const uint8_t palette[8][3])
{
    uint32_t row;
    uint32_t col;
    uint32_t bar_width = width / 8U;

    for (row = 0U; row < height; ++row) {
        for (col = 0U; col < width; ++col) {
            size_t offset = ((size_t)row * (size_t)stride) + ((size_t)col * 3U);
            uint32_t bar = col / bar_width;
            if (memcmp(&buffer[offset], palette[bar], 3U) != 0) {
                return 0;
            }
        }
    }
    return 1;
}

static int buffer_is_unchanged(const uint8_t *buffer,
                               const uint8_t *snapshot,
                               size_t size)
{
    return memcmp(buffer, snapshot, size) == 0;
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
    uint8_t invalid_snapshot[sizeof(invalid_buffer)];

    memset(buffer, 0xA5, sizeof(buffer));
    check("16x2 RGB color bars are generated",
          Framebuffer_Fill_RGB_Color_Bars(buffer, 16U, 2U, 52U), 1);
    check("every active pixel matches its expected color bar",
          all_pixels_match(buffer, 16U, 2U, 52U, expected), 1);
    check_padding("row padding is not overwritten", buffer, 2U, 52U, 48U);

    memset(invalid_buffer, 0xA5, sizeof(invalid_buffer));
    memcpy(invalid_snapshot, invalid_buffer, sizeof(invalid_buffer));

    check("width not divisible by eight is rejected",
          Framebuffer_Fill_RGB_Color_Bars(invalid_buffer, 10U, 2U, 32U), 0);
    check("invalid width leaves every byte untouched",
          buffer_is_unchanged(invalid_buffer, invalid_snapshot, sizeof(invalid_buffer)), 1);

    check("stride smaller than RGB row is rejected",
          Framebuffer_Fill_RGB_Color_Bars(invalid_buffer, 16U, 2U, 47U), 0);
    check("invalid stride leaves every byte untouched",
          buffer_is_unchanged(invalid_buffer, invalid_snapshot, sizeof(invalid_buffer)), 1);

    check("zero width is rejected",
          Framebuffer_Fill_RGB_Color_Bars(invalid_buffer, 0U, 2U, 48U), 0);
    check("zero height is rejected",
          Framebuffer_Fill_RGB_Color_Bars(invalid_buffer, 16U, 0U, 48U), 0);
    check("RGB width multiplication overflow is rejected",
          Framebuffer_Fill_RGB_Color_Bars(invalid_buffer, UINT32_MAX - 7U, 1U, UINT32_MAX), 0);
    check("frame byte-range overflow is rejected",
          Framebuffer_Fill_RGB_Color_Bars(invalid_buffer, 16U, 2U, UINT32_MAX), 0);
    check("all rejected geometries leave the buffer untouched",
          buffer_is_unchanged(invalid_buffer, invalid_snapshot, sizeof(invalid_buffer)), 1);
    check("null framebuffer is rejected",
          Framebuffer_Fill_RGB_Color_Bars(NULL, 16U, 2U, 48U), 0);

    if (failures != 0) {
        printf("FAIL: %d framebuffer pattern check(s) failed.\n", failures);
        return 1;
    }

    printf("PASS: framebuffer RGB pattern checks completed.\n");
    return 0;
}
