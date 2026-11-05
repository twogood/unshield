# Security Policy

## Threat model

Unshield assumes its input is trustworthy. It is not hardened against
malicious InstallShield files, and there are certainly many ways to make it
crash or read out of bounds with crafted input. That is known and expected.

If you need to process untrusted files, run Unshield in a sandbox.

## Reporting

Memory-safety bugs triggered by malformed input (fuzzer findings, ASan
reports, out-of-bounds reads or writes, crashes) are treated as ordinary
bugs, not vulnerabilities. There are no CVEs, advisories or embargoes for
them.

**Issues without a fix will be closed. Please send a pull request instead.**
A good pull request:

- adds bounds checks where file-controlled offsets and sizes are used
- fixes the whole class of bug where practical, not just one fuzzer input
- includes the reproducer file under `test/` if it is small

If you believe you have found something beyond a crash on malformed input,
open a pull request or a short issue describing it.
