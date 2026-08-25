# Block Design Summary

The supplied Vivado block design contains these major IP blocks:

- `processing_system7_0` — Zynq-7000 Processing System
- `axi_vdma_0` — AXI VDMA 6.3, MM2S enabled, 24-bit stream, one frame store
- `axi_protocol_convert_0` — AXI3 to AXI4-Lite control conversion
- `axi_smc_hp0` — SmartConnect for the PS HP0 DDR path
- `rst_ps7_0_100M` — synchronized PL reset
- `v_tc_0` — Video Timing Controller
- `v_axi4s_vid_out_0` — AXI4-Stream video output

## Key connections

```text
PS M_AXI_GP0
   │
   ▼
AXI Protocol Converter
   │
   ▼
VDMA S_AXI_LITE

VDMA M_AXI_MM2S
   │
   ▼
SmartConnect HP0
   │
   ▼
PS S_AXI_HP0 ──► DDR

VDMA M_AXIS_MM2S
   │
   ▼
AXI4-Stream Video Output
   │
   ├── timing input ◄── VTC
   └── video output
```

The block design routes the PS `FCLK_CLK0` to the AXI/VDMA/video timing clock domains. The supplied design data configures that clock at 100 MHz.

The VDMA control register segment is mapped at `0x43000000`; the MM2S data path accesses the PS DDR/low-OCM address space.
