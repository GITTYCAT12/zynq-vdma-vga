# Build and Reopen

## Vivado

1. Install Vivado 2026.1 with Zynq-7000 support.
2. Open `vivado/zynq_vdma_vga/zynq_vdma_vga.xpr`.
3. Let Vivado regenerate missing generated IP/cache data if prompted.
4. Open the `zynq_video_system` block design and run **Validate Design**.
5. Generate the HDL wrapper/output products as required by the local Vivado installation.
6. Run synthesis/implementation if a physical board build is required.

The repository intentionally does not track `.cache`, `.gen`, `.runs`, `.hw`, or other generated directories.

## Vitis / standalone software

The application source is under `vdma_vga_app/src/main.c`.

The application:
- generates the RGB color-bar framebuffer;
- flushes the data cache;
- looks up and initializes AXI VDMA;
- configures the MM2S read channel;
- supplies the framebuffer address;
- starts the VDMA in circular mode.

The exported hardware handoff is `zynq_video_platform/hw/zynq_video_system_wrapper.xsa`.

## Portability

The original Vivado project contains a machine-specific project path in the `.xpr` metadata. If Vivado reports a path mismatch after cloning, use **Project Settings / Add Sources** or reopen the project from its cloned location and allow Vivado to update its local metadata.
