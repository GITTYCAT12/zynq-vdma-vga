# Verification Checklist

## Static checks

- Confirm the target device is `xc7z020clg400-1`.
- Confirm `FCLK_CLK0` is configured to 100 MHz.
- Confirm AXI VDMA MM2S is present in the block design.
- Confirm the VTC and AXI4-Stream video output blocks are present.
- Confirm the PS exposes the HP0 path used for high-performance DDR access.
- Confirm the application uses a 640×480, 24-bit RGB framebuffer.

## Software checks

Expected console sequence:

1. framebuffer pattern is written to DDR;
2. AXI VDMA initialization starts;
3. VDMA is configured and the framebuffer address is supplied;
4. the read channel starts and continuously streams the frame.

## Hardware checks

On a compatible Zynq board with the appropriate VGA/pin constraints:

- program the generated bitstream;
- run the standalone ARM application;
- observe the RGB color-bar output;
- verify that the image remains continuously active while the ARM CPU is idle in the main loop.

The uploaded source archive does not claim a specific physical VGA connector pinout because the board was not specified in the source package.
