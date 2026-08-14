# Update Log

## 2026-08-14

* **v0.1.0**: Introduces the four span-precondition APIs
  [`at_least_span<T, N>`, `non_empty_span<T>`, `checked_span<N>`, and
  `block_span<T, N>`](api/span-family.md). Tagged releases follow SemVer: within the 0.x series,
  PATCH releases preserve compatibility and MINOR releases define the compatibility boundary, as
  specified by [ADR-0007](decisions/adr-0007-versioning-compatibility-and-support.md). Supported
  environments are those verified by required CI jobs and are listed in the
  [README](../README.md#supported-environments). See [issue #30](https://github.com/urario/precept-cpp/issues/30)
  for the release work and evidence.
