# Embedded Programs

This repository is organized for embedded firmware work with AI-assisted development.

Current default assumptions:

- Primary target: `ESP32-S3-N16-R8`
- Primary SDK: `ESP-IDF`
- Primary language style: `C-first`
- Repository style: multiple small embedded programs and experiments

The operational guidance for humans and coding agents lives in [`skills/`](./skills/).
Start with [`skills/README.md`](./skills/README.md) before making changes.

## Repository Direction

This repository is intended to hold embedded programs, prototypes, and reusable patterns for ESP32-class development. It is intentionally documentation-first at this stage so future code can inherit consistent engineering and agent workflow rules.

## Working Model

- Keep hardware assumptions explicit.
- Prefer small, testable modules over tangled application logic.
- Treat hardware validation as a required part of "done" for firmware changes.
- Make agent behavior predictable by documenting defaults, constraints, and verification steps.

## Code Layout

The coding portion of the repository is organized as a monorepo with:

- `apps/` for independent firmware programs
- `components/` for reusable modules such as logging, wifi, storage, and system
- `boards/` for target-specific defaults and hardware notes

See [`CODEBASE.md`](./CODEBASE.md) for the repository code organization rules.

## Common Scripts

- `./scripts/install_deps.sh` installs Linux-side repository dependencies and bootstraps ESP-IDF through `eim`
- `./scripts/flash.sh <target>` flashes an app from `apps/<target>` using `/dev/ttyACM0` by default
- `./scripts/monitor.sh [target]` opens a UART monitor for an app using `/dev/ttyACM0` by default

Run either script with `--help` for available options.

## App Configuration

Apps may keep local build-time configuration in `apps/<target>/.env`.

- real `.env` files are local-only and ignored by git
- example values should live in `apps/<target>/.env.example`
- the reference Wi-Fi flow expects `WIFI_SSID` and `WIFI_PASSWORD`
