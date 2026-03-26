# Reference App

This is the baseline example application for the repository structure.

## Purpose

- demonstrate how an app consumes shared components
- keep app-specific wiring out of reusable components
- serve as the template for future programs under `apps/`

## Current Composition

- system module
- logging module
- wifi module
- storage module

This app is intentionally minimal. It proves the repository structure and module boundaries before product-specific behavior is added.

## Local Configuration

Create `apps/reference_app/.env` from `apps/reference_app/.env.example` before building.

Required keys:

- `WIFI_SSID`
- `WIFI_PASSWORD`

## Sensor Defaults

The reference app now includes a DHT22 integration using the board-profile defaults.

- default DHT22 data pin: `GPIO4`
- default read period: `5000 ms`

These defaults are defined in the board profile and should be updated once your real wiring is known.
