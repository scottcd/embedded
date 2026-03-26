# ESP32-S3-N16-R8 Board Notes

This directory holds board-specific defaults for the repository baseline target.

## Intent

- Keep target assumptions explicit
- Prevent app and component code from hard-coding board details everywhere
- Provide one place to document hardware constraints as the repository grows

## Current Policy

- Shared modules should accept board-dependent values through config structs.
- Pin maps and peripheral assignments should be added here or in app-local notes once they are confirmed.
- Do not invent exact board wiring until it is backed by a schematic or hardware documentation.

## Expected Contents Over Time

- board pin mappings
- peripheral routing notes
- memory-sensitive defaults
- bring-up checklists
- app-specific overrides where needed

## Current Reference App Assumptions

- `DHT22` data line defaults to `GPIO4`
- reference app reads the sensor every `5000 ms`

These are implementation defaults for initial bring-up, not confirmed wiring facts. Update them once the real hardware is connected.
