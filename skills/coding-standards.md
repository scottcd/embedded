# Coding Standards

## Purpose

Set repository-wide code quality expectations for C-first embedded firmware.

## Use When

- Writing new firmware code
- Reviewing code style and implementation quality
- Deciding whether a C++ feature is justified

## Defaults

- Primary implementation style: C-first
- Add C++ only when it solves a real problem better than plain C
- Prefer explicitness, traceability, and hardware-safe behavior over cleverness

## Do

- Keep functions small enough to reason about quickly.
- Make error paths explicit and consistent.
- Use fixed-width integer types for hardware-facing and protocol-facing code.
- Keep headers minimal and free of unnecessary dependencies.
- Document invariants where timing, memory ownership, or interrupt context matters.
- Name modules by responsibility, not by implementation detail.
- Prefer compile-time configuration and narrow runtime configuration surfaces.

## Avoid

- Do not hide control flow in macros unless the pattern is established and justified.
- Do not allocate dynamically on hot paths without a documented reason.
- Do not mix ISR-safe and task-only behavior in the same interface without marking the boundary clearly.
- Do not use C++ features that obscure memory cost, exception behavior, or startup semantics.
- Do not add abstraction layers that exist only to look generic.

## Verify

- Error handling matches the failure model of the subsystem.
- Concurrency boundaries are obvious.
- Logging is useful for diagnosis without flooding normal operation.
- Public headers communicate contract, ownership, and thread or ISR assumptions.

## C++ Exception Rule

If C++ is introduced, keep it constrained and explain why C was insufficient. Favor RAII only where object lifetime is simple and visible. Avoid exceptions, hidden heap-heavy types, and template-heavy designs unless the repository later adopts a stronger C++ policy.
