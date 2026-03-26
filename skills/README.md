# Skills Index

This directory defines the repository's operating rules for embedded development and AI-assisted coding.

Use these files as a small, explicit contract:

1. Read this file first.
2. Load the target-specific file next.
3. Load only the task-relevant skill files after that.
4. Prefer repository defaults unless a program-specific note overrides them.

## Defaults

- Target baseline: `ESP32-S3-N16-R8`
- SDK baseline: `ESP-IDF`
- Language baseline: `C-first`
- Repo shape baseline: multiple small embedded programs and experiments

## Skill Files

- `target-esp32-s3-n16-r8.md`
  Default hardware assumptions, resource profile, and target-specific checks.
- `esp-idf-workflow.md`
  Expected SDK workflow, build/flash/debug conventions, and project hygiene.
- `firmware-architecture.md`
  Structural guidance for app code, drivers, interfaces, and state handling.
- `coding-standards.md`
  Code-level standards for C-first firmware work.
- `testing-and-validation.md`
  What must be verified before a change is considered done.
- `safety-and-debugging.md`
  Safe device handling, bring-up discipline, and failure-oriented debugging.
- `agent-workflow.md`
  Rules for how coding agents should operate in this repository.

## Precedence

Apply guidance in this order:

1. Task-specific constraints from the user
2. Program-specific local notes, if present
3. The target profile in this directory
4. The relevant workflow, architecture, testing, and safety skills
5. General preference for minimal, reversible changes

## Override Model

Future programs may add local notes to override board pins, peripherals, memory budgets, or app-specific constraints. Unless such a note exists, assume the repository defaults in this directory are authoritative.

## Authoring Rules For Future Skills

- Keep files short and operational.
- Start with `Purpose`, `Use When`, and `Defaults`.
- Prefer `Do`, `Avoid`, and `Verify` sections over prose-heavy explanations.
- State unknowns explicitly instead of implying certainty.
- If a workflow depends on external hardware or local tooling, say so directly.
