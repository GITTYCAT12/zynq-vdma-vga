# Verification Checklist

## Static checks

- Confirm the target device is `xc7z020clg400-1`.
- Confirm `FCLK_CLK0` is configured to 100 MHz.
- Confirm AXI VDMA MM2S is present in the block design.
- Confirm the VTC and AXI4-Stream video output blocks are present.
- Confirm the PS exposes the HP0 path used for high-performance DDR access.
- Confirm the application uses a 640×480, 24-bit RGB framebuffer.
- Run the host-only framebuffer arithmetic checks with `bash tests/run_host_tests.sh`.

## Host-side framebuffer tests

Run both portable C regression suites with:

```bash
bash tests/run_host_tests.sh
```

The range suite checks the configured frame size, zero dimensions, 32-bit byte-count overflow and address-space wrap. The RGB pattern suite checks all eight bar colors across multiple rows, verifies that row padding is preserved, and rejects invalid width/stride/null-buffer inputs before writing. Both run in the `Framebuffer range checks` GitHub Actions workflow.

These host tests cover arithmetic and the in-memory RGB888 pixel layout only. They do not compile the Xilinx BSP application and do not prove that the framebuffer address is mapped to DDR, that cache maintenance is correct on the target, or that the VDMA/display path works.

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

## Verification stages

Use the following progression so failures can be localized instead of being treated as one end-to-end failure:

| Stage | What to verify | Expected evidence |
|---|---|---|
| Design | PS, VDMA, VTC and video path are present | Vivado block-design review |
| Software | Framebuffer and VDMA configuration complete | UART/console log |
| Memory path | Framebuffer is written to the intended DDR region | Address/configuration inspection |
| Stream path | MM2S starts and remains active | VDMA status/register evidence |
| Display | RGB color-bar pattern is visible | Monitor/output observation |
| Stability | Stream remains active after CPU setup finishes | Continuous output with idle main loop |

The uploaded source archive does not claim a specific physical VGA connector pinout because the board was not specified in the source package.

The repository currently documents the verification procedure and expected evidence; successful physical-display validation still depends on a compatible Zynq board and the required VGA/pin constraints.
