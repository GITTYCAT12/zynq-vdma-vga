# Architecture

## Data path

1. The ARM Cortex-A9 application writes a 640×480, 24-bit RGB test pattern into DDR.
2. The application flushes the relevant CPU cache range so the VDMA sees the framebuffer contents in external memory.
3. AXI VDMA is configured for the **MM2S (memory-to-stream)** channel.
4. VDMA reads the framebuffer from DDR and produces an AXI4-Stream video stream.
5. The AXI4-Stream video path feeds the video output block while the VTC supplies video timing.
6. The CPU does not need to copy every pixel into the stream; after initialization, the VDMA performs the repeated DDR-to-stream transfers in hardware.

## Clocking

The Zynq PS configuration in the block design enables `FCLK_CLK0` at **100 MHz** and uses that clock for the PL/AXI infrastructure.

## Framebuffer parameters

The application source defines:

- Width: 640 pixels
- Height: 480 lines
- Pixel format: 24-bit RGB
- Bytes/pixel: 3
- Stride: 1920 bytes/line
- Frame size: 921,600 bytes

The application uses a single framebuffer address at `0x10000000` and enables circular VDMA operation.

## Efficiency-oriented design choices

The important optimization is architectural rather than a CPU-frequency claim: the ARM creates the frame once, flushes the cache once, and then leaves continuous frame transport to the AXI VDMA hardware. This reduces processor involvement in the steady-state video path.
