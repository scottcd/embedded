# ESP-IDF Workflow

## Purpose

Define the default development workflow for ESP-IDF-based projects in this repository.

## Use When

- Creating or modifying an ESP-IDF application
- Reasoning about menuconfig, flashing, monitor output, or component layout
- Planning build and debug steps for firmware changes

## Defaults

- ESP-IDF is the repository-standard SDK
- Favor reproducible local commands over IDE-specific steps
- Keep SDK version assumptions explicit once the repository pins one

## Do

- Organize applications so build logic, configuration, and source layout are easy to inspect.
- Keep component boundaries clear: app logic, drivers, transport layers, and board adaptation should not blur together.
- Prefer checked-in defaults over undocumented local machine tweaks.
- Keep build and flash steps scriptable and easy for an agent to repeat.
- Capture the expected validation steps alongside any meaningful firmware change.

## Avoid

- Do not assume a global ESP-IDF installation path or shell setup unless the repo documents it.
- Do not bury critical configuration in undocumented interactive steps.
- Do not mix board adaptation logic into portable components without a clear reason.
- Do not rely on menuconfig-only changes as the only expression of system behavior when a checked-in config can represent it.

## Verify

- The change can be built and reasoned about from repository state and documented local tooling.
- Configuration-sensitive behavior is called out explicitly.
- Any flashing or monitor workflow needed for validation is documented in the task result.

## Notes For Agents

If the repository later pins an ESP-IDF version, treat version-specific APIs and Kconfig symbols as strict interfaces. Until then, avoid claiming exact symbol names unless verified locally.
