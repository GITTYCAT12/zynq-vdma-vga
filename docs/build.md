# Build and Reopen

## Current source-package limitation

The tracked Vivado project file is metadata, not a self-contained hardware design. At this repository revision it references:

- The editable block design at `$PSRCDIR/sources_1/bd/zynq_video_system/zynq_video_system.bd`.
- A generated HDL wrapper at `$PGENDIR/sources_1/bd/zynq_video_system/hdl/zynq_video_system_wrapper.v`.

Neither file is present in the Git tree. The block design and its IP configuration are the editable source needed to recreate the wrapper and generated products. Regenerating Vivado cache or run directories cannot reconstruct a missing editable block design.

`MANIFEST.txt` also lists `zynq_video_platform/hw/zynq_video_system_wrapper.xsa`, but that XSA is not currently tracked. Do not treat the manifest entry as an available hardware handoff.

Consequently, a fresh clone is **not currently sufficient to reproduce the Vivado hardware design or build the standalone application**. `PROJECT_STATUS.md` records this packaging gap.

## Vivado

After restoring the editable block design and its required IP configuration/source files:

1. Install Vivado with Zynq-7000 support.
2. Open `vivado/zynq_vdma_vga/zynq_vdma_vga.xpr` from the cloned repository.
3. Regenerate the HDL wrapper and required IP output products from the restored block design.
4. Open the `zynq_video_system` block design and run **Validate Design**.
5. Run synthesis/implementation if a physical board build is required.
6. Export the hardware platform (XSA) from the validated design and add the intended handoff artifact or a reproducible generation procedure to the repository.

The repository intentionally does not track `.cache`, `.gen`, `.runs`, `.hw`, or other generated directories. Those exclusions do not remove the requirement to version the editable design sources.

## Vitis / standalone software

The application source is under `vdma_vga_app/src/main.c`.

The application is intended to:

- generate the RGB color-bar framebuffer;
- flush the data cache;
- look up and initialize AXI VDMA;
- configure the MM2S read channel;
- supply the framebuffer address;
- start the VDMA in circular mode.

Building it requires an XSA exported from the matching hardware design and the corresponding standalone BSP/platform. The XSA path currently listed in `MANIFEST.txt` is absent from Git, so this checkout does not yet provide that handoff. The host-side C tests do not compile the Xilinx BSP application.

## Portability

The original Vivado project contains a machine-specific project path in the `.xpr` metadata. If Vivado reports a path mismatch after cloning, reopen the project from its cloned location and allow Vivado to update its local metadata after the missing design sources have been restored.
