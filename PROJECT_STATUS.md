# Project Status

**Repository packaging:** source-archive review required

The repository contains the Vivado project metadata, standalone ARM application sources, and documentation. However, the tracked Vivado project file currently references generated/source paths under `vivado/zynq_vdma_vga/zynq_vdma_vga.srcs/` that are not present in the Git tree.

### Included
- Vivado project metadata (`.xpr`)
- Standalone ARM application source
- Application build metadata
- Documentation and verification notes
- Project-ready `.gitignore`

### Current reproducibility gap
- The `.xpr` references the editable block design at `sources_1/bd/zynq_video_system/zynq_video_system.bd` and a generated wrapper, but those files are not currently tracked in the repository.
- The manifest also references an XSA path that is not part of the current tracked tree.
- Therefore, the repository should **not yet be described as fully reproducible from Git alone**.

### Intentionally excluded
- Vivado cache/generated/run directories
- Vitis build output
- IDE workspace metadata
- tool logs and temporary files

### Required next step
Commit the editable Vivado block design (`.bd`), required IP configuration/source files, wrapper/necessary HDL, and the intended hardware handoff or a reproducible regeneration procedure. Keep generated caches, run directories, and temporary build output excluded.
