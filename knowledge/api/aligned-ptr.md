---
type: API Contract
title: aligned_ptr API Contract
description: Defines the alignment guarantee, validation boundary, nullability, conversions, and ownership rules of aligned_ptr.
status: draft
sources:
  - id: issue-41
    resource: https://github.com/urario/precept-cpp/issues/41
    title: Design and evidence issue for representing a stable pointer-alignment property
    author: human:urario
  - id: issue-41-production-admission
    resource: https://github.com/urario/precept-cpp/issues/41#issuecomment-5294631358
    title: Production admission after the portable validation and compiler spike
    author: human:urario
  - id: issue-41-codegen-addendum
    resource: https://github.com/urario/precept-cpp/issues/41#issuecomment-5295018842
    title: Code-generation result and current-address contract refinement
    author: human:urario
  - id: issue-41-final-contract
    resource: https://github.com/urario/precept-cpp/issues/41#issuecomment-5295112396
    title: Final diagnostics and ergonomics gate freezing the production contract
    author: human:urario
tags: [api, alignment, pointer, structural-property, contract]
---

# Scope

This contract defines the public behavior of:

```cpp
precept::aligned_ptr<T, N>
```

It is the first admitted Precept type that carries a stable structural property of one value. It
is a nullable, non-owning pointer carrier, not an ownership, allocation, range, or pointer
arithmetic abstraction.

# Type admission and invariant

`aligned_ptr<T, N>` is a valid specialization only when:

* `T` is a complete object type;
* `N` is non-zero and a power of two; and
* `N >= alignof(T)`.

These conditions are class-level constraints. Invalid specializations are rejected during
constraint checking rather than diagnosed by assertions in the class body.

Every value stores either a null pointer or a pointer whose current address satisfies at least
`N`-byte alignment. The guarantee describes the current address only. It does not guarantee:

* which allocation or declaration mechanism established the alignment;
* ownership or extension of the pointed-to object's lifetime;
* the alignment of a pointer produced by arithmetic; or
* the additional preconditions of a downstream standard or third-party facility.

The representation contains one pointer. Copy and move operations preserve the stored address and
the alignment fact.

# Construction and validation

There is no public default constructor, raw-pointer constructor, unchecked tag, or implicit
raw-pointer conversion. A raw pointer enters the type only through:

```cpp
precept::aligned_ptr<T, N>::try_from(T*)
```

For a non-null input, the caller is responsible for providing a pointer to a live `T` object backed
by at least `sizeof(T)` bytes of contiguous storage. Validation checks whether the pointer's current
address satisfies `N`-byte alignment. It does not read or modify the object.

The portable C++20 validation route uses `std::align` with the candidate address and
`sizeof(T)` bytes of available storage. This defines the alignment test without making a public
contract depend on an implementation-defined pointer-to-integer mapping.

`try_from()` is `[[nodiscard]]` but is not `noexcept`. The C++20 `std::align` interface does not
provide the portable non-throwing guarantee that Precept would need in order to promise one. An
exception from the underlying validation operation propagates.

# Nullability and failure states

Null is part of the ordinary pointer value domain and does not mean that alignment validation
failed. `try_from()` therefore has three observable outcomes:

| Input | Result |
| --- | --- |
| non-null and `N`-byte aligned | engaged optional containing the original pointer |
| null | engaged optional containing a null `aligned_ptr` |
| non-null and not `N`-byte aligned | `std::nullopt` |

This type carries alignment only. Code that also requires non-null input must express or check that
separate rule at the appropriate boundary.

# Safe weakening

The converting constructor is implicit exactly when:

```cpp
M >= N && std::is_convertible_v<U (*)[], T (*)[]>
```

Because admitted alignments are powers of two, `aligned_ptr<U, M>` then proves the target's weaker
alignment. The array-pointer conversion test permits qualification-only compatibility such as
mutable-to-const conversion while rejecting const removal and array covariance. In particular,
`aligned_ptr<Derived, N>` does not convert to `aligned_ptr<Base, N>` because a base-subobject
conversion may adjust the physical address.

Alignment strengthening requires fresh validation. No constructor converts a weaker carrier to a
stronger one.

# Standard-pointer escape

`get()` returns the stored `T*` as an explicit, named weakening. It is `constexpr` and `noexcept`.
It performs no validation and does not apply `std::assume_aligned` or another optimizer hint.

Optimizer assumptions belong at the consumption point. A caller that passes the result to
`std::assume_aligned` or another facility remains responsible for that facility's complete
preconditions; `aligned_ptr` proves only its own current-address alignment fact.

# Template deduction boundary

Implicit weakening applies when a function parameter already has a fixed target type:

```cpp
void consume(precept::aligned_ptr<const int, 32>);

precept::aligned_ptr<int, 64> source = /* validated */;
consume(source); // accepted
```

Function-template argument deduction does not consider user-defined conversions. A generic
consumer whose parameter pattern is `aligned_ptr<const T, 32>` therefore cannot deduce `T` from an
`aligned_ptr<int, 64>` argument. Generic code that abstracts over alignment exposes the source
element type and alignment as its own template parameters or constraints.

# Non-goals

The public surface deliberately excludes:

* ownership and allocation;
* pointer arithmetic;
* `operator*`, `operator->`, and an implicit `T*` conversion;
* a range, span, or container wrapper;
* hidden optimizer assumptions; and
* a generic pointer-property framework.

When a richer domain view naturally owns the real contract, that view is preferred. When an
alignment fact is established and consumed only once in local code, a local check or operation is
preferred over materializing an `aligned_ptr`.

# Structural-property admission finding

A stable structural property is not sufficient by itself to justify a semantic carrier. The fact
must cross an API boundary and change or constrain downstream behavior often enough to be reused.
The carrier preserves only that fact and permits only address-preserving weakening. A richer domain
view wins when the actual contract includes more than the structural property.

# Public API sketch

```cpp
namespace precept {

template<class T, std::size_t N>
    requires /* T is a complete object type; N is a valid alignment for T */
class aligned_ptr {
public:
    static constexpr std::size_t alignment = N;

    template<class U, std::size_t M>
        requires (M >= N) &&
                 std::is_convertible_v<U (*)[], T (*)[]>
    constexpr aligned_ptr(const aligned_ptr<U, M>& source) noexcept; // implicit

    [[nodiscard]]
    static std::optional<aligned_ptr> try_from(T* pointer);

    [[nodiscard]]
    constexpr T* get() const noexcept;
};

} // namespace precept
```

# Required implementation verification

Production verification covers:

* aligned, misaligned-but-live, and null inputs;
* preservation of the original pointer;
* an engaged null result distinct from `std::nullopt`;
* stronger-to-weaker alignment and mutable-to-const conversion;
* rejection of alignment strengthening, const removal, and array covariance;
* rejection of zero, non-power-of-two, and below-`alignof(T)` alignments;
* rejection of `void` and incomplete element types;
* rejection of unchecked raw-pointer construction and implicit raw-pointer conversion;
* the accepted function-template deduction boundary; and
* a single-pointer wrapper compiling without additional work at the consumption boundary.

Runtime behavior belongs in GoogleTest, type and conversion properties in `static_assert`, and
focused rejection behavior in negative compile tests, as required by the
[Test Strategy](../testing/test-strategy.md).
