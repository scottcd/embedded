# Testing And Validation

## Purpose

Define what "done" means for embedded changes in this repository.

## Use When

- Implementing a feature or bug fix
- Planning validation work
- Deciding whether a change is safe to merge without hardware access

## Defaults

- Firmware changes require evidence, not just compile confidence
- Hardware validation matters more than theoretical correctness for device-facing behavior
- When hardware is unavailable, limits must be stated explicitly

## Do

- Validate the smallest meaningful scope first: build, boot, peripheral init, feature behavior, then stress or regression checks.
- Distinguish between host-side checks and on-device validation.
- Record what was verified, what was assumed, and what remains untested.
- Add focused tests for logic that can be exercised without real hardware.
- Treat boot logs, reset reasons, and error traces as first-class validation artifacts.

## Avoid

- Do not report success based only on static inspection when runtime behavior is central to the change.
- Do not collapse all validation into a single manual smoke test.
- Do not ignore timing, memory pressure, or peripheral contention if the change could affect them.
- Do not claim board-level correctness without the board or equivalent evidence.

## Verify

- Build success for the intended target
- Boot behavior for startup or configuration changes
- Peripheral behavior for hardware-facing changes
- Regression risk around memory, timing, task interaction, and error handling

## Minimum Closeout Format

When reporting a completed change, include:

- what changed
- what was verified
- what could not be verified
- what assumptions remain hardware-dependent
