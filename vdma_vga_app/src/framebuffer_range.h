#ifndef FRAMEBUFFER_RANGE_H
#define FRAMEBUFFER_RANGE_H

#include <stdint.h>

/*
 * Validate the arithmetic used by the RGB framebuffer writer and cache flush.
 * This helper is host-testable and does not depend on the Xilinx BSP.
 */
static inline int Framebuffer_Range_Is_Valid(uintptr_t base,
                                             uint32_t height,
                                             uint32_t stride)
{
    uint64_t frame_bytes;

    if (height == 0U || stride == 0U) {
        return 0;
    }

    /* Byte offsets in main.c are uint32_t, so their full range must fit. */
    if (height > (UINT32_MAX / stride)) {
        return 0;
    }

    frame_bytes = (uint64_t)height * (uint64_t)stride;

    /* Xil_DCacheFlushRange receives a signed pointer-sized length. */
    if (frame_bytes > (uint64_t)INTPTR_MAX) {
        return 0;
    }

    /* Reject a buffer whose one-past-end address would wrap uintptr_t. */
    if (frame_bytes > (uint64_t)(UINTPTR_MAX - base)) {
        return 0;
    }

    return 1;
}

#endif
