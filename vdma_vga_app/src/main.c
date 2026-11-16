#include <stdio.h>
#include <stdint.h>
#include "xparameters.h"
#include "xil_types.h"
#include "xstatus.h"
#include "xaxivdma.h"
#include "xil_cache.h"
#include "xil_printf.h"
#include "framebuffer_range.h"
#include "framebuffer_pattern.h"

// --- Display Parameters (640x480 @ 24-bit RGB) ---
#define FRAME_WIDTH          640
#define FRAME_HEIGHT         480
#define BYTES_PER_PIXEL      3   // 24-bit RGB = 3 bytes per pixel
#define STRIDE               (FRAME_WIDTH * BYTES_PER_PIXEL) // 1920 bytes per row

// --- Framebuffer DDR Memory Start Address ---
#define FRAME_BUFFER_BASE_ADDR  0x10000000U // 256MB offset in DDR

// --- Macro Fallbacks for Vitis 2026.1 SDT Flow ---
#ifndef VDMA_LOOKUP_ID
    #if defined(XPAR_XAXIVDMA_0_BASEADDR)
        #define VDMA_LOOKUP_ID XPAR_XAXIVDMA_0_BASEADDR
    #elif defined(XPAR_AXI_VDMA_0_BASEADDR)
        #define VDMA_LOOKUP_ID XPAR_AXI_VDMA_0_BASEADDR
    #elif defined(XPAR_AXI_VDMA_0_DEVICE_ID)
        #define VDMA_LOOKUP_ID XPAR_AXI_VDMA_0_DEVICE_ID
    #else
        #define VDMA_LOOKUP_ID 0
    #endif
#endif

// Global VDMA Hardware Instance
XAxiVdma VdmaInstance;

// Function Prototypes
int Validate_Framebuffer_Config(uintptr_t FrameAddr, uint32_t Width, uint32_t Height, uint32_t Stride);
int Draw_RGB_Color_Bars(uintptr_t FrameAddr, uint32_t Width, uint32_t Height, uint32_t Stride);
int Init_And_Start_VDMA(uintptr_t FrameAddr, uint32_t Width, uint32_t Height, uint32_t Stride);

int main(void) {
    xil_printf("\r\n=====================================================\r\n");
    xil_printf("   ZYNQ VDMA VGA DISPLAY SYSTEM - HARDWARE INITIALIZED\r\n");
    xil_printf("=====================================================\r\n");

    // Reject an invalid framebuffer geometry or byte range before touching DDR or VDMA registers.
    if (Validate_Framebuffer_Config((uintptr_t)FRAME_BUFFER_BASE_ADDR, FRAME_WIDTH, FRAME_HEIGHT, STRIDE) != XST_SUCCESS) {
        xil_printf("FATAL ERROR: Invalid framebuffer configuration!\r\n");
        return -1;
    }

    // 1. Generate 8 RGB Color Bars in DDR Memory
    xil_printf("1. Writing 640x480 RGB Color Pattern to DDR RAM (0x%08X)...\r\n", (unsigned int)FRAME_BUFFER_BASE_ADDR);
    if (Draw_RGB_Color_Bars((uintptr_t)FRAME_BUFFER_BASE_ADDR, FRAME_WIDTH, FRAME_HEIGHT, STRIDE) != XST_SUCCESS) {
        xil_printf("FATAL ERROR: Framebuffer generation failed!\r\n");
        return -1;
    }

    // 2. Configure and Start VDMA Engine
    xil_printf("2. Initializing AXI VDMA Read Controller...\r\n");
    if (Init_And_Start_VDMA((uintptr_t)FRAME_BUFFER_BASE_ADDR, FRAME_WIDTH, FRAME_HEIGHT, STRIDE) != XST_SUCCESS) {
        xil_printf("FATAL ERROR: VDMA Hardware Initialization Failed!\r\n");
        return -1;
    }

    xil_printf("3. SUCCESS: Video Streaming Active! VDMA reading DDR continuously.\r\n");
    xil_printf("=====================================================\r\n");

    // Main CPU stays idle while VDMA streams framebuffers in background hardware
    while (1) {
        // Application running
    }

    return 0;
}

// Validate geometry and the complete byte range before writing memory or programming VDMA.
int Validate_Framebuffer_Config(uintptr_t FrameAddr, uint32_t Width, uint32_t Height, uint32_t Stride) {
    uint32_t MinimumStride;

    if (Width == 0 || Height == 0) {
        xil_printf("ERROR: Frame width/height must be non-zero.\r\n");
        return XST_FAILURE;
    }

    // Protect the width-to-byte conversion from 32-bit overflow.
    if (Width > (UINT32_MAX / BYTES_PER_PIXEL)) {
        xil_printf("ERROR: Frame width overflows RGB byte calculation.\r\n");
        return XST_FAILURE;
    }

    MinimumStride = Width * BYTES_PER_PIXEL;
    if (Stride < MinimumStride) {
        xil_printf("ERROR: Stride (%u) is smaller than required (%u).\r\n", (unsigned int)Stride, (unsigned int)MinimumStride);
        return XST_FAILURE;
    }

    // The color-bar generator expects the width to divide into 8 equal bars.
    if ((Width % 8U) != 0U) {
        xil_printf("ERROR: Width (%u) must be divisible by 8 for color bars.\r\n", (unsigned int)Width);
        return XST_FAILURE;
    }

    // Keep the actual framebuffer base word-aligned for typical AXI VDMA configurations.
    if ((FrameAddr & 0x3U) != 0U) {
        xil_printf("ERROR: Framebuffer base address is not word-aligned.\r\n");
        return XST_FAILURE;
    }

    // row * Stride, Height * Stride, the cache-flush length and the address
    // end must all remain representable before the buffer is touched.
    if (!Framebuffer_Range_Is_Valid(FrameAddr, Height, Stride)) {
        xil_printf("ERROR: Framebuffer byte range overflows supported size or address limits.\r\n");
        return XST_FAILURE;
    }

    return XST_SUCCESS;
}

// Draw 8 SMPTE RGB Color Bars (White, Yellow, Cyan, Green, Magenta, Red, Blue, Black)
int Draw_RGB_Color_Bars(uintptr_t FrameAddr, uint32_t Width, uint32_t Height, uint32_t Stride) {
    uint8_t *BufferPtr = (uint8_t *)FrameAddr;

    if (Validate_Framebuffer_Config(FrameAddr, Width, Height, Stride) != XST_SUCCESS) {
        return XST_FAILURE;
    }

    if (!Framebuffer_Fill_RGB_Color_Bars(BufferPtr, Width, Height, Stride)) {
        xil_printf("ERROR: RGB color-bar framebuffer generation failed.\r\n");
        return XST_FAILURE;
    }

    // Flush the complete validated framebuffer range before VDMA reads DDR.
    Xil_DCacheFlushRange((INTPTR)FrameAddr, (u32)(Height * Stride));

    return XST_SUCCESS;
}

// Configure and Start VDMA Read Channel (MM2S)
int Init_And_Start_VDMA(uintptr_t FrameAddr, uint32_t Width, uint32_t Height, uint32_t Stride) {
    int Status;
    XAxiVdma_Config *VdmaConfig;
    XAxiVdma_DmaSetup ReadChannelCfg;
    UINTPTR BufferAddrArray[1];

    if (Validate_Framebuffer_Config(FrameAddr, Width, Height, Stride) != XST_SUCCESS) {
        return XST_FAILURE;
    }

    // Lookup Hardware Config using Vitis 2026.1 compatible ID/BaseAddr
    VdmaConfig = XAxiVdma_LookupConfig(VDMA_LOOKUP_ID);
    if (!VdmaConfig) {
        xil_printf("ERROR: VDMA LookupConfig failed!\r\n");
        return XST_FAILURE;
    }

    // Initialize Driver
    Status = XAxiVdma_CfgInitialize(&VdmaInstance, VdmaConfig, VdmaConfig->BaseAddress);
    if (Status != XST_SUCCESS) {
        xil_printf("ERROR: VDMA CfgInitialize failed!\r\n");
        return XST_FAILURE;
    }

    // Set DMA Read Channel Parameters
    ReadChannelCfg.VertSizeInput       = Height;
    ReadChannelCfg.HoriSizeInput       = Stride;
    ReadChannelCfg.Stride              = Stride;
    ReadChannelCfg.FrameDelay          = 0;
    ReadChannelCfg.EnableCircularBuf   = 1; // Continuous Looping Mode
    ReadChannelCfg.EnableSync          = 0;
    ReadChannelCfg.PointNum            = 0;
    ReadChannelCfg.EnableFrameCounter  = 0;
    ReadChannelCfg.FixedFrameStoreAddr = 0;

    Status = XAxiVdma_DmaConfig(&VdmaInstance, XAXIVDMA_READ, &ReadChannelCfg);
    if (Status != XST_SUCCESS) {
        xil_printf("ERROR: VDMA DmaConfig failed!\r\n");
        return XST_FAILURE;
    }

    // Set Framebuffer Address
    BufferAddrArray[0] = (UINTPTR)FrameAddr;
    Status = XAxiVdma_DmaSetBufferAddr(&VdmaInstance, XAXIVDMA_READ, BufferAddrArray);
    if (Status != XST_SUCCESS) {
        xil_printf("ERROR: VDMA SetBufferAddr failed!\r\n");
        return XST_FAILURE;
    }

    // Start DMA Streaming Engine
    Status = XAxiVdma_DmaStart(&VdmaInstance, XAXIVDMA_READ);
    if (Status != XST_SUCCESS) {
        xil_printf("ERROR: VDMA DmaStart failed!\r\n");
        return XST_FAILURE;
    }

    return XST_SUCCESS;
}
