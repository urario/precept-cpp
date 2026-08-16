# Update Log

## 2026-08-16

* **v0.2.0**: Extends Precept beyond the original v0.1 span-precondition family while keeping that
  family public. The semantic vocabulary exploration admitted eight APIs:
  `aligned_ptr<T, N>` and `nonzero<T>` as property/refined carriers, `index_below<N>` as a
  parameterized property, `narrow_exact<T>(...)` as a factory to a standard type, `set_once<T>` and
  `never_decrease<T>` as lifecycle/transition types, and `same_size_pair` / `checked_same_size(...)`
  plus `non_overlapping_spans` / `checked_non_overlapping(...)` as relations. The stable scope of
  `non_overlapping_spans` is limited to byte-sized element spans (`sizeof(T) == 1`). The installed
  public surface is fixed at 12 headers. C++20, header-only delivery, and zero consumer
  dependencies remain unchanged. Under [ADR-0007](decisions/adr-0007-versioning-compatibility-and-support.md),
  0.2 is the 0.x MINOR compatibility boundary. No intentional breaking changes were introduced to
  the v0.1 span-family public surface. See [release issue #77](https://github.com/urario/precept-cpp/issues/77)
  for the release work and evidence.

## 2026-08-14

* **v0.1.0**: Introduces the four span-precondition APIs
  [`at_least_span<T, N>`, `non_empty_span<T>`, `checked_span<N>`, and
  `block_span<T, N>`](api/span-family.md). Tagged releases follow SemVer: within the 0.x series,
  PATCH releases preserve compatibility and MINOR releases define the compatibility boundary, as
  specified by [ADR-0007](decisions/adr-0007-versioning-compatibility-and-support.md). Supported
  environments are those verified by required CI jobs and are listed in the
  [README](../README.md#supported-environments). See [issue #30](https://github.com/urario/precept-cpp/issues/30)
  for the release work and evidence.
