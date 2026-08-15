---
type: API Contract
title: nonzero API Contract
description: Defines the non-zero guarantee, admitted element types, validation boundary, observation, and the arithmetic propagation boundary of nonzero.
status: draft
sources:
  - id: issue-42
    resource: https://github.com/urario/precept-cpp/issues/42
    title: Investigation of whether a scalar non-zero fact belongs in the public vocabulary as a validated argument carrier
    author: human:urario
  - id: issue-40
    resource: https://github.com/urario/precept-cpp/issues/40
    title: v0.2 semantic vocabulary investigation, including the design laws this candidate is measured against
    author: human:urario
  - id: issue-65-final-admission
    resource: https://github.com/urario/precept-cpp/issues/65#issuecomment-5301911732
    title: Final v0.2 admission matrix establishing the 8/8 stable portfolio
    author: chatgpt/gpt-5.6-sol
tags: [api, scalar-property, integral, validation, contract]
---

# Scope

This contract defines the public behavior of:

```cpp
precept::nonzero<T>
```

`nonzero<T>` carries only the fact that the stored scalar is not zero. It is not a general numeric
wrapper and does not propagate the guarantee through arbitrary arithmetic.

It is the first admitted Precept type that carries a property of an ordinary scalar value rather
than of a range or a pointer. It admitted under a narrower scope than the candidate issue
considered: integral element types only.

# Type admission and invariant

`nonzero<T>` is a valid specialization only when `T` is an integral type that is neither `bool` nor
cv-qualified.

`bool` is excluded because the fact would be degenerate: a `nonzero<bool>` could only ever hold
`true`, so the type would carry no information the value does not already carry. Cv-qualification
is excluded because it would remove ordinary value semantics — assignment from another validated
`nonzero` — without making any fact stronger.

Floating-point types are outside this contract. `+0.0`, `-0.0`, and NaN make "not zero" a
materially different rule, and no evidence was gathered for it here.

Every value stores an integer that compares unequal to `T{}`. The representation is the single
stored `T`; the type is trivially copyable, and copy and move preserve the value and the fact.

# Construction and validation

There is no public default constructor, raw-value constructor, unchecked tag, or implicit
conversion from `T`. A raw value enters the type only through:

```cpp
precept::nonzero<T>::try_from(T)
```

Validation is a single comparison against `T{}`. It is `constexpr` and `noexcept`, and returns
`std::nullopt` for zero. On success the stored value is the input unchanged, including its sign.

A compile-time constant reaches the type through the same factory. There is no separate
compile-time constructor.

# Observation

`value()` returns the stored `T` by value. It is `constexpr` and `noexcept`, and is the only way
out of the type. There is no implicit conversion to `T`, no `operator*`, and no
`explicit operator bool`.

Returning to a standard scalar is expected rather than exceptional: any computation on a validated
value is written on `value()` and produces ordinary integers.

# Arithmetic and comparison boundary

The type provides no arithmetic operators and no comparison operators.

Arithmetic is not closed over the guarantee — the difference of two non-zero values can be zero —
so under
[Design Principles](../architecture/design-principles.md), the result of such an operation must not
be given the stronger type. Rather than offering operators that silently return `T`, the type
offers none, and a result that must carry the fact is validated again.

# What the fact does not cover

Non-zero is necessary but not always sufficient for the operations that motivate it:

* signed division additionally requires the result to be representable, so
  `std::numeric_limits<T>::min() / T{-1}` is undefined for a validated divisor;
* a validated index or handle still needs its own range check;
* the fact describes the value only, not any state it refers to.

Code that needs more than "not zero" states the additional rule separately. Combining them into
this type would break the One rule requirement of the
[API Admission Rules](../architecture/api-admission-rules.md).

# Non-goals

The public surface deliberately excludes:

* arithmetic operators and a numeric wrapper surface;
* comparison and ordering operators;
* implicit conversion to `T`;
* conversion between `nonzero<T>` and `nonzero<U>` of a different width or signedness;
* floating-point element types; and
* a generic refined-value or predicate-composition framework.

Width conversion is excluded for lack of evidence, not because it is unsound: value-preserving
integral widening is closed over the guarantee. It is left out until a real call site needs it.

# Scalar-property admission finding

The evidence gathered in issue #42 is recorded there. In short: the carrier pays for itself where
the validated value is **stored** and read by more than one later layer, and it does not where the
value is validated and consumed at the next call. Both patterns appear in the representative
usage, so the final v0.2 admission is **STABLE** with that usage boundary preserved: carry the fact
when it crosses API boundaries; prefer a local check when validation and consumption are adjacent.

Measured on GCC 13 at `-O2`, a function taking `nonzero<std::size_t>` and the same function taking
`std::size_t` generate identical code, and validating a literal constant folds away entirely. The
cost of the carrier is in the source, not in the object file.

# Public API sketch

```cpp
namespace precept {

template<class T>
    requires /* T is an integral type other than bool, without cv-qualification */
class nonzero {
public:
    using value_type = T;

    [[nodiscard]]
    static constexpr std::optional<nonzero> try_from(T value) noexcept;

    [[nodiscard]]
    constexpr T value() const noexcept;
};

} // namespace precept
```

# Required implementation verification

Production verification covers:

* rejection of zero, and preservation of positive, negative, unsigned, and extreme values;
* rejection of `bool`, cv-qualified, floating-point, enumeration, reference, pointer, and
  incomplete element types;
* absence of default construction, raw-value construction, and conversion in either direction;
* absence of arithmetic, comparison, and dereference operators;
* preservation of the value through copy and copy-assignment;
* `constexpr` validation and observation, including a compile-time constant;
* use from an installed consumer; and
* representative call sites in which the fact crosses more than one API boundary.

Runtime behavior belongs in GoogleTest, type properties in `static_assert`, and the two rejection
behaviors that protect the public invariant — unchecked construction and arithmetic propagation —
in negative compile tests, as required by the
[Test Strategy](../testing/test-strategy.md).
