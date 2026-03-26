# Codebase Layout

This repository is a single-device-first embedded monorepo for `ESP32-S3-N16-R8`.

## Layout

- `apps/`
  Independent firmware programs. Each app owns its entrypoint, build configuration, and module wiring.
  Per-app config files such as `.env` belong here.
- `components/`
  Reusable shared modules. These are the primary unit of code reuse across apps.
- `boards/`
  Board-specific defaults and hardware notes. Shared modules must not assume board details unless they are passed in explicitly.
- `scripts/`
  Repository helper scripts for common workflows like flashing and UART monitoring.
- `skills/`
  Repository operating rules for humans and coding agents.

## Placement Rules

- Put code in `apps/` when it is product-specific or only meaningful for one program.
- Put app-owned configuration templates such as `.env.example` next to the app that consumes them.
- Put code in `components/` when it is reusable across multiple firmware programs.
- Put pin maps, hardware constraints, and target-specific defaults in `boards/`.
- Keep ESP-IDF specifics near the boundary of a module instead of letting them leak into unrelated application logic.

## Shared Module Conventions

Each shared component should expose:

- a public header under `include/`
- a typed config struct
- explicit lifecycle APIs such as `init/start/stop/deinit`
- clear ownership of tasks, buffers, and callbacks

## Dependency Rules

- Apps may depend on shared components.
- Shared components must not depend on a specific app.
- Higher-level components should not hard-code board pin mappings.
- Cross-component dependencies should stay narrow and intentional.
