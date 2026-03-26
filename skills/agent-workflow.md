# Agent Workflow

## Purpose

Define how coding agents should behave in this repository so embedded changes are careful, auditable, and useful.

## Use When

- Starting any task in this repository
- Preparing a plan for firmware work
- Reporting results after a change

## Defaults

- Read `skills/README.md` first
- Load only the skill files relevant to the task
- Prefer small changes with explicit assumptions and visible verification

## Do

- Confirm the task's hardware assumptions before changing hardware-facing code.
- State uncertainty clearly when board-specific information is missing.
- Preserve target-specific behavior unless the task explicitly changes it.
- Keep changes small enough that failure causes remain diagnosable.
- Report validation limits honestly when real hardware is unavailable.
- Prefer checked-in documentation or scripts over undocumented local steps.

## Avoid

- Do not invent pin mappings, electrical assumptions, or exact ESP-IDF symbols without evidence.
- Do not broaden scope from one embedded fix into an architectural rewrite unless the task requires it.
- Do not mark firmware work complete without saying what was actually verified.
- Do not hide risky low-level changes inside large mixed commits.

## Required Task Flow

1. Read the skills index and relevant target or workflow files.
2. Inspect the affected code and identify hardware-sensitive surfaces.
3. Make the smallest change that satisfies the task.
4. Verify what can be verified locally.
5. Report results with explicit limits and next hardware checks.

## Required Final Report Elements

- concise summary of the change
- files affected
- validation performed
- validation not performed
- remaining assumptions or hardware-dependent risks
