---
type: API Contract
title: narrow_exact API Contract
description: Defines the exactness rule, admitted integer types, failure model, and the standard-type return boundary of narrow_exact.
status: draft
sources:
  - id: issue-47
    resource: https://github.com/urario/precept-cpp/issues/47
    title: Investigation of whether a validating factory that returns a standard type is clearer than the standard range check and cast
    author: human:urario
  - id: issue-40
    resource: https://github.com/urario/precept-cpp/issues/40
    title: v0.2 semantic vocabulary investigation, including the carrier and factory design laws this candidate is measured against
    author: human:urario
tags: [api, integral, conversion, validation, contract]
---

# Scope

This contract defines the public behavior of:

```cpp
precept::narrow_exact<T>(value)
```

It converts one integer to another when the destination represents the value exactly. It is the
first Precept API in the factory-to-standard-type shape that carries no Precept type at all: the
input is a standard integer, the output is a standard integer, and the verified fact is the
returned value itself.

Only integer-to-integer conversion is in scope. Floating-point, enumeration, and user-defined
conversions are outside it, as are saturating, clamping, and wrapping conversions.

# Exactness rule

A conversion succeeds when `T` can represent the mathematical value of `value`. The comparison is
on values, not on bit patterns or on the result of the usual arithmetic conversions, so:

* a negative value never converts to an unsigned type;
* a value above the destination maximum is never wrapped or truncated;
* a widening conversion always succeeds; and
* a successful result compares equal to the input.

The decision is made by `std::in_range<T>(value)`. Precept does not reimplement the rule; the
function exists for the shape of the call site, not for the arithmetic.

# Admitted types

`T` and `U` are signed or unsigned integer types that are not cv-qualified. `bool`, `char`,
`wchar_t`, `char8_t`, `char16_t`, and `char32_t` are excluded on both sides. Fixed-width aliases
such as `std::uint8_t` and `std::int64_t` are admitted, because they name plain integer types.

Two reasons hold that boundary together. It is the set `std::in_range` itself accepts, and this
API is defined in terms of `std::in_range`. Independently, `char` has implementation-defined
signedness, so admitting it would let the same conversion succeed on one platform and fail on
another — the opposite of what an exactness rule is for.

This set is deliberately narrower than the one admitted by
[`nonzero`](nonzero.md), which accepts `char` and the character types: a non-zero test needs no
cross-type value comparison and no portable signedness.

# Failure model

The return type is `std::optional<T>`. A representable value returns the converted value; a
value that is not representable returns `std::nullopt`. Validation does not throw, does not
assert, and has no other failure channel. This matches the non-throwing validation boundary the
rest of the vocabulary uses, recorded in
[ADR-0006](../decisions/adr-0006-validation-and-conversion-boundaries.md).

The function is `constexpr` and `noexcept`, and is marked `[[nodiscard]]`. A compile-time constant
is narrowed by the same call as a runtime value.

# Standard-type return boundary

There is no `narrow_exact<T>` type. The verified fact — "this value fits in `T`" — is fully
embodied by a `T` holding that value, so under the Factory to standard type principle in
[Design Principles](../architecture/design-principles.md) no wrapper is introduced.

This is the boundary against [`nonzero`](nonzero.md). A `nonzero<T>` exists because its fact is a
restriction on the value that `T` cannot express: any `T` can still be zero, so downstream code
would have to re-check. Here the destination type is the fact. `T` cannot hold a value outside
`T`, so a `narrow_exact` carrier would restate what the type already guarantees and would have to
be unwrapped at every use.

# Non-goals

The public surface deliberately excludes:

* floating-point, enumeration, and user-defined conversions;
* saturating, clamping, wrapping, and truncating conversions;
* a throwing or asserting variant, and a policy or error-type parameter;
* a conversion framework parameterized on the conversion rule; and
* a carrier type for the narrowed value.

# What the fact does not cover

Exact representability is about the value only. It says nothing about whether the value is
meaningful in the destination's domain: a payload length that fits a 16-bit field may still exceed
what a protocol permits, and a slot number that fits an `std::int32_t` handle may still be out of
range for the table it indexes. Those are separate rules, stated separately.

# Public API sketch

```cpp
namespace precept {

template<class T, class U>
    requires /* T and U are signed or unsigned integer types, without cv-qualification */
[[nodiscard]]
constexpr std::optional<T> narrow_exact(U value) noexcept;

} // namespace precept
```

# Comparison finding

The evidence gathered in issue #47 is recorded there. In short, against the standard spelling

```cpp
if (!std::in_range<T>(value)) { /* fail */ }
auto narrowed = static_cast<T>(value);
```

the measured differences are that the destination type is named once instead of twice — the
two-liner compiles silently when the checked type and the cast type disagree — and that the
unchecked value cannot be used by accident, because the result is an `std::optional<T>` rather
than a `T` sitting beside a boolean. Neither spelling prevents a caller from writing a bare
`static_cast` instead. The saved line is not the argument.

# Required implementation verification

Production verification covers:

* same-width, widening, and narrowing conversions that succeed;
* rejection above the destination maximum and below the destination minimum;
* rejection of negative values for unsigned destinations, and of large unsigned values for signed
  destinations, including the values on both sides of each boundary;
* rejection of `bool`, character, cv-qualified, floating-point, enumeration, pointer, reference,
  and incomplete types on both sides;
* `constexpr` use, including a narrowed compile-time constant;
* agreement with the standard `std::in_range` and `static_cast` spelling over a boundary table;
* use from an installed consumer; and
* representative call sites in more than one domain.

Runtime behavior belongs in GoogleTest and type properties in `static_assert`, per the
[Test Strategy](../testing/test-strategy.md). No negative compile test is required: the admitted
type set is observable as a constraint, so `static_assert` on the constrained call expression
verifies the same rejection without a separate CMake compile check.
