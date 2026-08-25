# Design Notes

## Why AXI VDMA?

AXI VDMA provides a hardware-managed path between DDR frame storage and the streaming video interface. This is preferable to having the ARM CPU repeatedly push pixel data through software.

## Why cache flush?

The ARM Cortex-A9 may hold recently written framebuffer data in cache. `Xil_DCacheFlushRange()` is used after the framebuffer is generated so the DDR copy observed by VDMA is current.

## Why circular mode?

`EnableCircularBuf = 1` allows the read channel to keep reusing the framebuffer. The CPU therefore does not need to restart the DMA engine for every frame.

## Pixel layout

The application writes pixels in R/G/B byte order:

`[Red][Green][Blue]`

The line stride is `640 × 3 = 1920` bytes.

## Scope

This repository represents the completed logical hardware/software project supplied in the source archive. Generated implementation products are intentionally left out of version control; they can be regenerated from the tracked Vivado project and IP configuration.
