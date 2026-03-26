# Firmware Architecture

## Purpose

Provide a consistent structure for small embedded programs and experiments so they remain debuggable as the repository grows.

## Use When

- Designing a new embedded program
- Refactoring app logic, drivers, or shared code
- Reviewing whether a change increases coupling or hides hardware assumptions

## Defaults

- Favor small standalone applications with explicit dependencies
- Prefer composition over deep abstraction layers
- Use state machines, queues, and narrow interfaces instead of hidden global behavior

## Do

- Separate application logic from hardware adaptation.
- Keep peripheral drivers thin and focused on one responsibility.
- Prefer explicit initialization order over lazy hidden startup.
- Make ownership of buffers, tasks, timers, and interrupts easy to trace.
- Use narrow interfaces between modules so behavior can be reviewed without reading the whole program.
- Isolate any board-specific glue from reusable logic.

## Avoid

- Do not let business logic depend directly on register-level details when a smaller boundary can isolate it.
- Do not create shared utility layers with vague purpose.
- Do not spread timing-critical behavior across multiple unrelated modules.
- Do not hide startup dependencies in side effects or global constructors.

## Verify

- A reviewer can point to where initialization starts, where tasks are created, and where shutdown or error paths go.
- Hardware-facing code is visibly distinct from app behavior.
- Time-sensitive or memory-sensitive paths are easy to identify.
- Shared code remains reusable across multiple programs in this repository.

## Preferred Shape

For most programs, keep these concepts distinct even if the final folder names change:

- app orchestration
- board or target adaptation
- drivers and protocol handling
- shared utilities with narrow scope
- tests or host-side checks where practical
