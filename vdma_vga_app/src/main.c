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
