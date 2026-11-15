#include <stdint.h>
#include <stdio.h>

#include "../vdma_vga_app/src/framebuffer_range.h"

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

int main(void)
{
    check("configured 640x480 framebuffer range",
          Framebuffer_Range_Is_Valid((uintptr_t)0x10000000U, 480U, 1920U), 1);
    check("zero height is rejected",
          Framebuffer_Range_Is_Valid((uintptr_t)0x10000000U, 0U, 1920U), 0);
    check("zero stride is rejected",
          Framebuffer_Range_Is_Valid((uintptr_t)0x10000000U, 480U, 0U), 0);
    check("32-bit byte-offset overflow is rejected",
          Framebuffer_Range_Is_Valid((uintptr_t)0x10000000U, UINT32_MAX, 2U), 0);
    check("unsigned 32-bit cache length above INT32_MAX remains representable",
          Framebuffer_Range_Is_Valid((uintptr_t)0U, 1U, 0x80000000U), 1);
    check("address-space wrap is rejected",
          Framebuffer_Range_Is_Valid(UINTPTR_MAX - (uintptr_t)1000U, 2U, 600U), 0);
    check("range ending at the maximum one-past address is accepted",
          Framebuffer_Range_Is_Valid(UINTPTR_MAX - (uintptr_t)1024U, 1U, 1024U), 1);


    if (failures != 0) {
        printf("FAIL: %d framebuffer range check(s) failed.\n", failures);
        return 1;
    }

    printf("PASS: framebuffer range checks completed.\n");
    return 0;
}
