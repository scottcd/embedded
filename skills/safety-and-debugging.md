# Safety And Debugging

## Purpose

Keep firmware iteration safe on physical hardware and make low-level failures easier to diagnose.

## Use When

- Flashing new firmware
- Investigating boot failures, resets, hangs, watchdog events, or peripheral faults
- Changing low-level startup, memory, clocking, or transport behavior

## Defaults

- Favor reversible, inspectable changes during bring-up
- Preserve observability before optimizing behavior
- Assume real hardware can fail in ways that static reasoning will miss

## Do

- Keep serial or USB logging available during bring-up whenever possible.
- Capture reset reasons, panic output, and early boot logs before making multiple new changes.
- Change one low-level variable at a time when debugging unstable systems.
- Treat power, boot mode, flash settings, and peripheral contention as likely root causes for early failures.
- Use conservative defaults before tuning performance-sensitive settings.

## Avoid

- Do not combine partition, clock, memory, and driver changes in one debugging step if symptoms are unclear.
- Do not suppress logs too early.
- Do not treat intermittent boot issues as solved without repeated verification.
- Do not assume debugger attachment, USB console behavior, or auto-reset wiring is identical across boards.

## Verify

- The device still boots consistently after low-level changes.
- Logs remain sufficient to diagnose regressions.
- Watchdog, panic, and reset behavior is understood for the changed code path.
- Recovery steps are known if a flash or config change leaves the board hard to access.

## Notes For Agents

When hardware behavior is unstable, reduce scope before increasing theory. Prefer a smaller reproducible failing case over a broad refactor plus uncertain diagnosis.
