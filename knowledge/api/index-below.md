---
type: API Contract
title: index_below API Contract
description: Defines the compile-time upper-bound guarantee, zero-bound policy, construction and observation boundaries, and the limit between a numeric bound and a domain-specific index.
status: draft
sources:
  - id: issue-43
    resource: https://github.com/urario/precept-cpp/issues/43
    title: Experiment testing whether a compile-time index bound is valuable as a carrier across API boundaries
    author: human:urario
  - id: issue-40
    resource: https://github.com/urario/precept-cpp/issues/40
    title: v0.2 semantic vocabulary investigation and carrier admission boundary
    author: human:urario
  - id: issue-42
    resource: https://github.com/urario/precept-cpp/issues/42
    title: nonzero scalar-carrier experiment used for comparison
    author: human:urario
  - id: issue-47
    resource: https://github.com/urario/precept-cpp/issues/47
    title: narrow_exact factory-to-standard-type experiment used for comparison
    author: human:urario
  - id: issue-65-final-admission
    resource: https://github.com/urario/precept-cpp/issues/65#issuecomment-5301911732
    title: Final v0.2 admission matrix establishing the 8/8 stable portfolio
    author: chatgpt/gpt-5.6-sol
tags: [api, index, scalar-property, fixed-extent, validation, contract]
---

# Scope

This contract defines the public behavior of:

```cpp
precept::index_below<N>
```

`index_below<N>` carries exactly one fact about an ordinary `std::size_t` value:

```text
value < N
```

The bound is part of the type and remains visible after validation. The type does not identify a
particular table, protocol field family, register bank, container object, or other domain. It is
not a bounded-integer framework.

# Bound and invariant

Every publicly obtainable value stores an index less than `N`. The representation is the validated
`std::size_t`; copy and move preserve both the value and the fact.

`index_below<0>` is a valid specialization. Its mathematical value set is empty, so no public call
can produce a value and `try_from()` always returns `std::nullopt`. Keeping the type valid lets
generic code name the index type of a zero-extent fixed table without creating an exceptional
template constraint. This does not weaken the invariant because the raw-value constructor remains
inaccessible.

# Construction and validation

There is no public default constructor, raw-index constructor, unchecked tag, or implicit
conversion from `std::size_t`. A raw index enters the type only through:

```cpp
precept::index_below<N>::try_from(std::size_t)
```

The function is `constexpr`, `noexcept`, and `[[nodiscard]]`. It returns `std::nullopt` when the
input is greater than or equal to `N`; otherwise it stores the input unchanged.

# Observation and fixed-extent composition

`value()` returns the stored `std::size_t` by value. It is `constexpr` and `noexcept`, and is the
only scalar observation. There is no implicit conversion to `std::size_t`.

Fixed-extent standard containers remain the storage and view vocabulary. Composition is explicit:

```cpp
std::array<Entry, 16> table;
precept::index_below<16> index = /* validated */;

use(table[index.value()]);
```

The same spelling works with `std::span<T, 16>`. The `.value()` call was acceptable at the tested
lookup sites, but it is still ceremony. It pays for itself only when the bound fact is passed to or
stored for another API. When validation and subscript are adjacent, a local comparison with
`table.size()` remains clearer.

# Domain-semantic boundary

The type prevents an out-of-bound value from reaching a parameter whose complete numeric
precondition is `index < N`. It does not prevent exchange between unrelated domains that happen to
use the same bound:

```cpp
using opcode_index = precept::index_below<32>;
using register_index = precept::index_below<32>; // the same type
```

This is not a hidden guarantee. A function name, parameter name, or enclosing aggregate may supply
enough domain context when only the numeric bound needs mechanical enforcement. If confusing two
same-sized domains must be rejected by the type system, the domain needs its own type; aliases of
`index_below<N>` are insufficient.

The carrier is also inappropriate for an ordinary loop index. A loop condition establishes and
consumes the bound locally, and no fact crosses an API boundary.

# Conversion and arithmetic boundary

The type provides no arithmetic, increment, decrement, ordering, dereference, or subscript
operators. Computation happens on `value()` and returns an ordinary scalar that must be validated
again if it needs to carry the bound.

No conversion between different bounds is provided. Numerically,
`index_below<8>` could safely weaken to `index_below<16>`, but the representative usages did not
need that conversion. Adding it would also make it easier to move a value between differently
bounded domains without establishing that the domains are related. The conversion remains absent
until a same-domain use demonstrates value beyond its mathematical validity.

# Non-goals

The public surface deliberately excludes:

* indices tied to a dynamic container instance;
* container ownership, lifetime tracking, and proof freshness for mutable runtime state;
* lower bounds, arbitrary intervals, and a general bounded-integer framework;
* arithmetic, increment, decrement, comparison, and implicit scalar conversion;
* conversion between bounds;
* policy templates and custom error types; and
* domain identity or a strong-typedef framework.

# Carrier admission finding

The representative fixed-protocol usage validates one external index and reuses it in two deeper
operations. There the carrier makes the bound visible in each signature and removes both repeated
checks. A fixed lookup table also composes naturally with `std::array` and fixed-extent `std::span`,
with one explicit `.value()` at the subscript.

The same experiment found two negative cases. A one-shot lookup was clearer as a local bounds check,
and an ordinary loop needed no carrier. It also confirmed that the compile-time bound is only part
of the semantic contract when two same-extent tables must not exchange indices.

The final v0.2 admission is **STABLE (use-boundary)**: use the generic carrier for a stable numeric
bound that is independently useful across API boundaries; use a local check for immediate
consumption and a domain-specific type when identity matters.

# Relationship to other v0.2 shapes

[`nonzero<T>`](nonzero.md) is another scalar carrier: an ordinary scalar cannot retain either
`value != 0` or `index < N`, so both need a carrier when the fact has downstream reuse value.
`index_below<N>` adds a compile-time parameter and exposes the new risk that the same numeric fact
can describe several unrelated domains.

[`narrow_exact<T>`](narrow-exact.md) returns a standard type instead. A successful destination `T`
fully embodies representability in `T`, while an ordinary `std::size_t` cannot embody an upper
bound. That makes `index_below<N>` a carrier candidate, but only actual cross-boundary reuse—not
fact loss by itself—justifies using it.

# Public API sketch

```cpp
namespace precept {

template<std::size_t N>
class index_below {
public:
    [[nodiscard]]
    static constexpr std::optional<index_below>
    try_from(std::size_t index) noexcept;

    [[nodiscard]]
    constexpr std::size_t value() const noexcept;
};

} // namespace precept
```

# Required implementation verification

Production verification covers:

* acceptance of zero and `N - 1` for non-zero bounds;
* rejection of `N` and `N + 1` where representable;
* the bounds zero and one;
* preservation of the original value and the fact through copies;
* absence of default construction, raw construction, and implicit scalar conversion;
* `constexpr` validation and observation;
* fixed-extent `std::array` and `std::span` composition;
* use from an installed consumer;
* a runnable example with both cross-boundary and local-check usages; and
* representative protocol, lookup-table, local-check, and ordinary-loop call sites.

Runtime behavior belongs in GoogleTest and type properties in `static_assert`, per the
[Test Strategy](../testing/test-strategy.md). No negative compile test is needed: the invariant
bypass and conversion boundaries are directly observable with standard type traits.
