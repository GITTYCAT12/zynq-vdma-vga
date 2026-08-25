# Zynq-7000 Real-Time Video Interfacing using AXI VDMA

A hardware/software video pipeline for the Zynq-7000 SoC. The design uses the Processing System (PS), AXI infrastructure, AXI VDMA, DDR frame buffering, Video Timing Controller (VTC), and AXI4-Stream video output path.

## Highlights

- Zynq-7000 / XC7Z020CLG400-1 target captured in the Vivado project.
- 100 MHz PL/AXI clocking configured from the Zynq PS (`FCLK_CLK0`).
- AXI VDMA MM2S reads a framebuffer from DDR and continuously streams video data.
- DDR-backed framebuffer decouples CPU frame generation from continuous video transport.
- 24-bit RGB software framebuffer at 640×480, with a 1920-byte line stride.
- Cache-coherent handoff: the ARM application flushes the data cache before VDMA starts reading DDR.
- Circular-buffer VDMA operation keeps the video stream running without repeated CPU transfers.
- VTC + AXI4-Stream video output provide timing/control for the video path.
- Exported XSA is included for the software/platform side.

## Architecture

```text
                 Zynq-7000 SoC
┌───────────────────────────────────────────────────────────┐
│  ARM Cortex-A9 (PS)                                       │
│       │ AXI control                                       │
│       ▼                                                   │
│  AXI VDMA ────────► DDR Frame Buffer                     │
│       │ MM2S                                             │
│       ▼                                                   │
│  AXI4-Stream Video ──► VTC / Video Output ──► VGA path  │
│                                                           │
│  FCLK_CLK0 = 100 MHz                                     │
└───────────────────────────────────────────────────────────┘
```

## Repository layout

- `vivado/` — Vivado project, block design, and editable IP configuration.
- `vdma_vga_app/` — standalone ARM application that creates the framebuffer and starts AXI VDMA.
- `zynq_video_platform/hw/` — exported hardware handoff (`.xsa`).
- `docs/` — architecture, build, verification, and design notes.

## Build environment

- Vivado/Vitis 2026.1
- Zynq-7000 / XC7Z020CLG400-1
- Standalone ARM Cortex-A9 software flow

## Important note

Generated Vivado/Vitis caches, run directories, and build products are intentionally excluded from Git. The repository keeps the editable design sources, block design, IP configuration, application source, and exported hardware handoff.

Board-specific VGA pin constraints depend on the physical Zynq board used and are not invented in this repository.
