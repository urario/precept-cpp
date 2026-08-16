---
type: API Contract
title: v0.1 Span Family API Contract
description: Defines the guarantees, construction boundaries, conversions, lifetime rules, and operations of the v0.1 span APIs.
status: draft
sources:
  - id: issue-4
    resource: https://github.com/urario/precept-cpp/issues/4
    title: Design issue for the v0.1 span API contract, failure model, and conversions
    author: human:urario
  - id: issue-4-final-review
    resource: https://github.com/urario/precept-cpp/issues/4#issuecomment-5287680856
    title: Final review resolution for static construction, validation inputs, block size, and borrowed iterators
    author: human:urario
  - id: issue-20-resolution
    resource: https://github.com/urario/precept-cpp/issues/20#issuecomment-5288715908
    title: Resolution prioritizing truthful range interoperability and safe semantic weakening
    author: human:urario
  - id: issue-79-v0-3
    resource: https://github.com/urario/precept-cpp/issues/79
    title: v0.3 design resolution for minimum-extent fact preservation across compile-time subviews
    author: human:urario
  - id: issue-80-v0-3
    resource: https://github.com/urario/precept-cpp/issues/80
    title: v0.3 implementation contract for proof-preserving subviews of at_least_span
    author: human:urario
  - id: issue-81-v0-3
    resource: https://github.com/urario/precept-cpp/issues/81
    title: v0.3 subview contract tests, diagnostics, and zero-extra-validation evidence
    author: human:urario
tags: [api, span, v0.1, v0.3, contract]
---

# Scope

This contract defines the v0.1 public behavior of:

```cpp
precept::at_least_span<T, N>
precept::non_empty_span<T>
precept::checked_span<N>(...)
precept::block_span<T, N>
```

It also defines the v0.3 `subspan<Offset>()` addition to `at_least_span<T, N>`.

It is normative for issues #5, #6, and #7 and for the v0.3 subview contract. The reasons for the validation and conversion
boundaries are recorded separately in
[ADR-0006](../decisions/adr-0006-validation-and-conversion-boundaries.md).

# Common contract

## Validation and failure

Runtime validation returns `std::optional`. A size mismatch produces `std::nullopt`; it does not
throw, assert, terminate, truncate the input, or create a partially valid object. Validation
functions are `[[nodiscard]]`.

Assertions may diagnose an internal implementation contradiction, but they are not part of the
public validation contract. Standard preconditions on operations such as an out-of-range
`operator[]` remain caller responsibilities.

## Validation inputs

Validation functions accept `std::span` only. v0.1 does not provide generic range, container,
initializer-list, iterator-pair, or pointer-and-count validation overloads. A caller with a
contiguous container forms the standard view explicitly:

```cpp
std::vector<std::byte> bytes = receive();
auto packet = precept::at_least_span<const std::byte, 16>::try_from(
    std::span<const std::byte>{bytes});
```

This boundary does not make a dangling `std::span` valid. The caller remains responsible for the
validity of the span supplied to Precept.

## Static and runtime facts

Compile-time-safe construction and runtime validation have distinct responsibilities:

* A fixed extent that proves a wrapper invariant uses a constructor.
* A fixed extent that disproves an invariant is rejected during overload resolution.
* A dynamic extent uses runtime validation.

`at_least_span::try_from` and `block_span::try_from` therefore accept only a span whose deduced
extent is `std::dynamic_extent`. They do not offer an always-engaged optional path for a fixed-valid
source. A caller may explicitly erase a fixed extent by constructing a dynamic span and then ask
for runtime validation.

`checked_span` has no Precept wrapper constructor. It accepts a fixed source only when its extent
equals `N`, and otherwise accepts a dynamic source. Its return type remains optional in both cases.

## Element-type compatibility

Conversions between element types permit only array qualification conversions, tested as
`std::is_convertible_v<U (*)[], T (*)[]>`. Mutable elements may become const; const may not be
removed. Array covariance, including treating `Derived[]` as `Base[]`, is not supported.

## Standard-span conversion

Precept wrappers do not define implicit conversion operators to `std::span`.

`at_least_span` intentionally models the standard contiguous, sized, and borrowed range concepts.
Consequently, the standard library's range constructor may implicitly construct a dynamic-extent
`std::span` from it. This is allowed semantic weakening: the source satisfies the fact-free target
contract without runtime validation, without changing element qualification, ownership, or
lifetime, and without creating an invalid target state.

Implicit weakening is acceptable only when the source contract logically satisfies the target
contract without runtime validation and without creating a false semantic guarantee. A conversion
that strengthens an invariant or removes element qualification remains rejected or requires
validation.

`as_span()` remains the named observer for explicitly obtaining the complete dynamic-extent
`std::span<T>` by value. It makes standard-span use visible when that is useful, but it is not the
only possible semantic-weakening path. It never returns a mutable reference to a wrapper's internal
state.

This weakening applies to a dynamic-extent span. `at_least_span<T, N>` proves `size() >= N`, not
`size() == N`; callers use `prefix()` when they require a guaranteed `std::span<T, N>`.

## Ownership, lifetime, and const

Every Precept span type is a non-owning borrowed view. It does not allocate, own storage, or extend
the lifetime of the viewed elements. Destruction or reallocation of the owner, or the end of an
element's lifetime, invalidates views, iterators, and references under the same rules as
`std::span`.

Constness of a view object is shallow. `const at_least_span<int, 4>` still refers to mutable
`int`; read-only elements require `at_least_span<const int, 4>`. The same rule applies to
`block_span` and to all returned standard spans.

The wrappers model `std::ranges::borrowed_range`. Their iterators depend on the underlying storage,
not on the lifetime of the wrapper object.

## `constexpr`, `noexcept`, and CTAD

Constructors, validation, observers, iterator operations, subviews, and `as_span()` are
`constexpr`. Operations in this contract accept or manipulate `std::span` state only and are
`noexcept`; an invalid size is represented by `std::nullopt`.

No custom deduction guides are provided. Callers state the semantic constant and requested element
type explicitly. Natural copy deduction supplied by the language is not a documented construction
path.

# `at_least_span<T, N>`

## Guarantee and edge cases

Every object guarantees `size() >= N`. `N` must be greater than zero and must not equal
`std::dynamic_extent`. `at_least_span<T, 0>` is rejected because it carries no fact beyond an
ordinary dynamic span.

`T` has the same complete, non-abstract object-type requirements as the element type of
`std::span`.

## Construction and validation

The type is not default-constructible and exposes no unchecked, tagged, pointer-and-count, or
dynamic-span constructor.

* Copy and move construction preserve the invariant.
* `std::span<U, E>` constructs implicitly when `E` is fixed, `E >= N`, and `U` is
  qualification-compatible with `T`.
* `at_least_span<U, M>` converts implicitly when `M >= N` and the element types are
  qualification-compatible.
* `try_from(std::span<U, E>)` participates only when `E == std::dynamic_extent` and returns an
  engaged optional exactly when `source.size() >= N`.
* A fixed source with `E < N` is rejected at compile time. A fixed-valid source uses the
  constructor and is also rejected by `try_from` so there is no duplicate always-successful path.

A weaker minimum cannot convert directly to a stronger one. If its runtime size might satisfy the
stronger invariant, the caller obtains its dynamic span with `as_span()` and invokes the stronger
type's `try_from()` so the strengthening remains explicit and validated.

## Operations

The v0.1 surface, plus the v0.3 subview addition, contains:

* `minimum_size`, equal to `N`
* `size()`, `size_bytes()`, and `data()`
* `begin()`, `end()`, and `operator[]`
* guaranteed `front()` and `back()`
* `prefix()`, returning `std::span<T, N>`
* `rest()`, returning the dynamic span `[N, size())`
* `subspan<Offset>()`, returning a compile-time subview while preserving a derivable residual minimum
* `as_span()`, returning the complete dynamic element view

There is no member `empty()`: it would always return false and add no semantic value. Generic
`std::ranges::empty()` remains usable through `size()`. Other `std::span` convenience operations
are available from the standard span returned by `as_span()` rather than being reimplemented.

When `size() == N`, `prefix()` covers the entire view and `rest()` is a valid empty span.

### Compile-time subviews (v0.3)

The v0.3 addition is limited to this operation:

```cpp
template<std::size_t Offset>
    requires (Offset <= N)
[[nodiscard]] constexpr auto subspan() const noexcept;
```

Its result is determined by the compile-time offset:

* `subspan<Offset>()`, `Offset < N` returns `at_least_span<T, N - Offset>`.
* `subspan<N>()` returns `std::span<T>` and is the same view as `rest()`.
* `subspan<Offset>()`, `Offset > N`, is not a valid call because the constraint is not satisfied.

The runtime actual size is preserved; the view is not truncated. The operation performs no new
runtime validation because the residual minimum follows mechanically from the source guarantee.
The result remains non-owning, and its lifetime and invalidation rules follow the source view.
`at_least_span<T, 0>` does not exist: when the guarantee is exhausted, the result is the standard
type. `first<K>()` is not part of v0.3; use `prefix().first<K>()` when that standard fixed-extent
view is the required result.

## Example

```cpp
void parse(precept::at_least_span<const std::byte, 16> packet)
{
    std::span<const std::byte, 16> header = packet.prefix();
    std::span<const std::byte> payload = packet.rest();
    // ...
}

std::span<const std::byte> input = receive_view();
if (auto packet =
        precept::at_least_span<const std::byte, 16>::try_from(input)) {
    parse(*packet);
}
```

Misuse is rejected:

```cpp
std::span<const std::byte, 8> short_packet;

// Ill-formed: the fixed extent disproves the invariant.
precept::at_least_span<const std::byte, 16> packet = short_packet;

// Ill-formed: fixed spans do not use the runtime-validation path.
auto duplicate =
    precept::at_least_span<const std::byte, 4>::try_from(short_packet);
```

# `non_empty_span<T>`

## Guarantee and representation

`non_empty_span<T>` is exactly an alias:

```cpp
template<class T>
using non_empty_span = at_least_span<T, 1>;
```

It guarantees at least one element and inherits all construction, validation, conversion,
lifetime, const, and operation rules from `at_least_span<T, 1>`. In particular, `front()` and
`back()` are always valid for an existing object, and an empty dynamic source produces
`std::nullopt`.

The alias and `at_least_span<T, 1>` are the same type and cannot form distinct overloads. A thin
`precept/span/non_empty_span.hpp` forwarding header exposes the vocabulary without introducing a
second implementation.

## Example

```cpp
std::span<int> values = current_values();
if (auto items = precept::non_empty_span<int>::try_from(values)) {
    use(items->front(), items->back());
}
```

# `checked_span<N>(source)`

## Guarantee and validation

`checked_span<N>` validates exact size and returns
`std::optional<std::span<T, N>>`. It succeeds only when `source.size() == N`; it never truncates to
a prefix.

The overload accepts `std::span<T, E>` only when `E == std::dynamic_extent || E == N`. A fixed
source whose extent differs from `N` is rejected at compile time. A fixed-equal source returns an
engaged optional; keeping one return type avoids source-dependent result types.

`N == 0` is allowed and succeeds for an empty source, producing `std::span<T, 0>`.
`N == std::dynamic_extent` is rejected because it does not express an exact-size check.

The returned standard span owns no storage and preserves the source element type, including const.
After validation, all operations and conversions are those of `std::span<T, N>`; Precept adds no
wrapper.

## Example

```cpp
std::span<std::byte> input = receive_into(buffer);
if (auto header = precept::checked_span<16>(input)) {
    consume_header(*header); // std::span<std::byte, 16>
}

std::span<std::byte, 32> fixed;
// Ill-formed: failure is statically certain.
auto impossible = precept::checked_span<16>(fixed);
```

# `block_span<T, N>`

## Guarantee and edge cases

`N` must be greater than zero and must not equal `std::dynamic_extent`. Every object guarantees
that its underlying element count is divisible by `N`; iteration therefore never exposes a
partial block.

Empty input is valid and represents zero complete blocks. Default construction produces this
valid empty state. `N == 1` is valid. `N == 0` is rejected because division, stepping, and a
zero-element logical block are not meaningful for this API.

## Construction and validation

* Copy and move construction preserve the invariant.
* `std::span<U, E>` constructs implicitly when `E` is fixed, `E % N == 0`, and the element types
  are qualification-compatible.
* `block_span<U, N>` converts implicitly when the element types are qualification-compatible.
* `try_from(std::span<U, E>)` participates only when `E == std::dynamic_extent` and returns an
  engaged optional exactly when `source.size() % N == 0`.
* A fixed, non-divisible source is rejected at compile time. A fixed divisible source uses the
  constructor and is rejected by `try_from` to avoid an always-engaged duplicate path.

There is no conversion between different block sizes. Repartitioning requires
`target::try_from(source.as_span())`, which makes both repartitioning and runtime validation
visible.

## Range and iterator model

The logical elements are blocks. `block_span<T, N>` is a common, sized, random-access, borrowed
range, but not a contiguous range. Its associated types are:

```cpp
using value_type = std::span<T, N>;
using reference = std::span<T, N>; // returned by value
```

Dereferencing an iterator returns a block view by value. The underlying elements are still aliased;
copying the returned span does not copy elements.

An iterator must not depend on the lifetime of the originating `block_span` object. If the
underlying storage remains valid, an iterator obtained from a wrapper remains usable after that
wrapper is destroyed. This is a behavioral guarantee; the contract does not prescribe an iterator
representation.

Because dereference returns a prvalue view, these loops are supported:

```cpp
for (std::span<const std::byte, 16> block : blocks) { /* ... */ }
for (auto block : blocks) { /* ... */ }
```

`for (auto& block : blocks)` is ill-formed because a non-const lvalue reference cannot bind to the
returned prvalue span.

Constness is shallow: iterating `const block_span<T, N>` produces `std::span<T, N>`. Read-only
elements require `block_span<const T, N>`.

## Operations

The v0.1 surface contains:

* `block_size`, equal to `N`
* `size()`, returning the number of logical blocks
* `block_count()`, a semantic alias returning the same value as `size()`
* `empty()`, testing whether the block count is zero
* `begin()` and `end()`
* `operator[]`, returning `std::span<T, N>` with precondition `index < size()`
* `as_span()`, returning the underlying dynamic element view

For every valid object:

```cpp
blocks.size() == blocks.block_count();
std::ranges::size(blocks) == blocks.size();
blocks.as_span().size() == blocks.size() * N;
```

`front()` and `back()` are not added because an empty `block_span` is valid and those operations
would not gain a stronger guarantee than an ordinary range.

## Example

```cpp
std::span<const std::byte> input = receive_view();
auto result = precept::block_span<const std::byte, 16>::try_from(input);
if (!result) {
    return malformed_input;
}

for (std::span<const std::byte, 16> block : *result) {
    consume(block);
}
```

Misuse is rejected:

```cpp
std::span<std::byte, 17> fixed;

// Ill-formed: the fixed extent is not divisible by 16.
precept::block_span<std::byte, 16> blocks = fixed;

// Ill-formed: fixed spans do not use the runtime-validation path.
auto duplicate = precept::block_span<std::byte, 1>::try_from(fixed);

// Ill-formed: a zero block size is not a valid specialization.
precept::block_span<std::byte, 0> zero_blocks;
```

# Public API sketch

The following sketch fixes names, return types, constraints, and conversion explicitness. Helper
concepts shown under `detail` are implementation-only and do not create public vocabulary.

```cpp
namespace precept {

namespace detail {

template<class From, class To>
concept qualification_compatible =
    std::is_convertible_v<From (*)[], To (*)[]>;

} // namespace detail

template<class T, std::size_t N>
    requires (N > 0 && N != std::dynamic_extent)
class at_least_span {
public:
    using element_type = T;
    using value_type = std::remove_cv_t<T>;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using reference = T&;
    using iterator = std::span<T>::iterator;

    static constexpr size_type minimum_size = N;

    at_least_span() = delete;
    constexpr at_least_span(const at_least_span&) noexcept = default;
    constexpr at_least_span(at_least_span&&) noexcept = default;
    constexpr at_least_span& operator=(const at_least_span&) noexcept = default;
    constexpr at_least_span& operator=(at_least_span&&) noexcept = default;

    template<class U, std::size_t E>
        requires (E != std::dynamic_extent && E >= N &&
                  detail::qualification_compatible<U, T>)
    constexpr at_least_span(std::span<U, E> source) noexcept; // implicit

    template<class U, std::size_t M>
        requires (M >= N && detail::qualification_compatible<U, T>)
    constexpr at_least_span(const at_least_span<U, M>& source) noexcept; // implicit

    template<class U, std::size_t E>
        requires (E == std::dynamic_extent &&
                  detail::qualification_compatible<U, T>)
    [[nodiscard]] static constexpr std::optional<at_least_span>
    try_from(std::span<U, E> source) noexcept;

    [[nodiscard]] constexpr std::span<T> as_span() const noexcept;
    [[nodiscard]] constexpr std::span<T, N> prefix() const noexcept;
    [[nodiscard]] constexpr std::span<T> rest() const noexcept;
    template<std::size_t Offset>
        requires (Offset <= N)
    [[nodiscard]] constexpr auto subspan() const noexcept;
    [[nodiscard]] constexpr size_type size() const noexcept;
    [[nodiscard]] constexpr size_type size_bytes() const noexcept;
    [[nodiscard]] constexpr pointer data() const noexcept;
    [[nodiscard]] constexpr reference front() const noexcept;
    [[nodiscard]] constexpr reference back() const noexcept;
    [[nodiscard]] constexpr reference operator[](size_type index) const noexcept;
    [[nodiscard]] constexpr iterator begin() const noexcept;
    [[nodiscard]] constexpr iterator end() const noexcept;
};

template<class T>
using non_empty_span = at_least_span<T, 1>;

template<std::size_t N, class T, std::size_t E>
    requires (N != std::dynamic_extent &&
              (E == std::dynamic_extent || E == N))
[[nodiscard]] constexpr std::optional<std::span<T, N>>
checked_span(std::span<T, E> source) noexcept;

template<class T, std::size_t N>
    requires (N > 0 && N != std::dynamic_extent)
class block_span {
public:
    using element_type = T;
    using block_type = std::span<T, N>;
    using value_type = block_type;
    using reference = block_type;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    class iterator; // std::random_access_iterator, independent of wrapper lifetime

    static constexpr size_type block_size = N;

    constexpr block_span() noexcept = default;
    constexpr block_span(const block_span&) noexcept = default;
    constexpr block_span(block_span&&) noexcept = default;
    constexpr block_span& operator=(const block_span&) noexcept = default;
    constexpr block_span& operator=(block_span&&) noexcept = default;

    template<class U, std::size_t E>
        requires (E != std::dynamic_extent && E % N == 0 &&
                  detail::qualification_compatible<U, T>)
    constexpr block_span(std::span<U, E> source) noexcept; // implicit

    template<class U>
        requires detail::qualification_compatible<U, T>
    constexpr block_span(const block_span<U, N>& source) noexcept; // implicit

    template<class U, std::size_t E>
        requires (E == std::dynamic_extent &&
                  detail::qualification_compatible<U, T>)
    [[nodiscard]] static constexpr std::optional<block_span>
    try_from(std::span<U, E> source) noexcept;

    [[nodiscard]] constexpr std::span<T> as_span() const noexcept;
    [[nodiscard]] constexpr size_type size() const noexcept;
    [[nodiscard]] constexpr size_type block_count() const noexcept;
    [[nodiscard]] constexpr bool empty() const noexcept;
    [[nodiscard]] constexpr block_type operator[](size_type index) const noexcept;
    [[nodiscard]] constexpr iterator begin() const noexcept;
    [[nodiscard]] constexpr iterator end() const noexcept;
};

} // namespace precept

namespace std::ranges {

template<class T, std::size_t N>
    requires (N > 0 && N != std::dynamic_extent)
inline constexpr bool
    enable_borrowed_range<precept::at_least_span<T, N>> = true;

template<class T, std::size_t N>
    requires (N > 0 && N != std::dynamic_extent)
inline constexpr bool
    enable_borrowed_range<precept::block_span<T, N>> = true;

} // namespace std::ranges
```

# Required implementation verification

Issue #5 must verify:

* exact and greater dynamic sizes, insufficient dynamic size, and `N == 1`
* fixed-valid construction and fixed-invalid construction rejection
* rejection of fixed spans by `try_from`, including a fixed-valid span
* mutable-to-const and stronger-to-weaker conversions, with reverse conversions rejected
* contiguous, sized, and borrowed range modeling
* implicit safe weakening to a compatible dynamic-extent `std::span`, with the named `as_span`
  form also supported
* `prefix`, `rest`, `front`, `back`, range operations, and `as_span` return types, including use of
  `prefix()` for the guaranteed fixed-extent view
* rejection of `N == 0` and `N == std::dynamic_extent`
* alias identity of `non_empty_span<T>` and `at_least_span<T, 1>`

Issue #6 must verify:

* exact, shorter, and longer dynamic inputs
* engaged fixed-equal input and compile-time rejection of fixed-unequal input
* mutable and const element propagation
* engaged empty input for `N == 0`
* rejection of `N == std::dynamic_extent`
* no truncation and no exception/assertion on runtime mismatch

Issue #7 must verify:

* default empty, empty input, one block, multiple blocks, remainder, and `N == 1`
* fixed-divisible construction and rejection of fixed-non-divisible construction
* rejection of all fixed spans by `try_from`, including fixed-divisible spans
* `size() == block_count() == std::ranges::size()` and the element-count relationship
* `value_type`, prvalue `reference`, random-access/common/sized/borrowed range, and not
  contiguous range
* mutable-to-const conversion and shallow const behavior
* rejection of different block-size conversion, `N == 0`, and dynamic block extent
* iterator use after the originating wrapper is destroyed while storage remains valid

Runtime behavior belongs in GoogleTest, type and concept properties in `static_assert`, and focused
rejection behavior in negative compile tests, as required by the
[Test Strategy](../testing/test-strategy.md).
