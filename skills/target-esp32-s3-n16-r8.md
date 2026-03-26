# ESP32-S3-N16-R8 Target Profile

## Purpose

Define the default hardware assumptions for this repository so firmware decisions do not rely on guesswork.

## Use When

- Starting a new embedded program in this repository
- Making memory-sensitive changes
- Configuring build, flash, partitions, logging, or peripheral behavior
- Reviewing whether a proposed change is safe for the default module

## Defaults

- Baseline target: `ESP32-S3-N16-R8`
- Assume ESP32-S3-class dual-core MCU behavior unless a specific board note narrows it further
- Assume `16 MB` flash and `8 MB` PSRAM as the working memory profile implied by the module name
- Assume external peripherals and exact pin mappings are program-specific unless documented locally

## Do

- Treat memory layout, flash use, and PSRAM use as first-class design constraints.
- Keep board-specific pin assignments outside shared guidance unless they are confirmed by a program note or schematic.
- Verify partition sizing before adding large assets, OTA layouts, filesystem images, or memory-heavy features.
- Prefer conservative boot and logging settings for bring-up work.
- Confirm that USB, serial console, and debug access assumptions match the actual board in use.

## Avoid

- Do not invent exact pin mappings, sensor attachments, or power-tree assumptions.
- Do not assume PSRAM use is free; validate latency, stack placement, DMA compatibility, and startup behavior.
- Do not change flash mode, partition layout, or clock-related settings casually.
- Do not hard-code module-specific assumptions inside reusable libraries unless they are passed in through configuration.

## Verify

- Target and flash settings match the real board configuration.
- Any PSRAM-dependent feature still behaves correctly when memory pressure changes.
- Boot logs are captured for bring-up or low-level failures.
- Peripheral choices are backed by local documentation when they affect pins, timing, or power.

## Notes For Agents

If a task depends on exact hardware details that are not present in the repository, stop claiming certainty. Continue with code structure or interface work, but mark hardware-bound decisions as assumptions requiring board-level confirmation.
