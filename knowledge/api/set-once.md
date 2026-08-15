---
type: API Contract
title: set_once API Contract
description: Defines the one-way slot transition, observation, construction, copy and move, failure, and contained-value boundaries of set_once.
status: draft
sources:
  - id: issue-45
    resource: https://github.com/urario/precept-cpp/issues/45
    title: Investigation of whether a set-once transition belongs in the public vocabulary
    author: human:urario
  - id: issue-55
    resource: https://github.com/urario/precept-cpp/issues/55
    title: Production contract and implementation issue for set_once
tags: [api, transition, initialization, value-holder]
---

# Scope

This contract defines the public behavior of:

```cpp
precept::set_once<T>
```

`set_once<T>` is a non-thread-safe value holder whose slot can transition from unset to set at
most once. It owns its contained `T`. It is not a synchronization primitive, execution-once
facility, dependency-injection container, or generic state machine.

# Type admission and invariant

`set_once<T>` is a valid specialization only when `T` is a complete, non-array, destructible
object type. Individual setting operations participate only when `T` can be constructed from their
arguments.

Default construction creates an unset slot. Once construction of a contained `T` succeeds, that
object's slot remains set until the `set_once` object's lifetime ends. The public API has no reset,
replacement, assignment, or mutable-storage escape that can return the slot to unset or install a
second `T`.

Destroying one `set_once` and beginning a new object lifetime creates a different object and a new
history. The contract does not attempt to survive object destruction, placement construction, or
other operations that begin a new C++ object lifetime.

# Setting and failure

The public mutation operations are:

```cpp
bool try_set(const T& value);
bool try_set(T&& value);

template<class... Args>
bool try_emplace(Args&&... args);
```

For an unset slot, each operation constructs `T` in the owned storage and returns `true`.
`try_set` copies or moves from its argument; `try_emplace` constructs directly from the forwarded
arguments.

For a set slot, each operation returns `false` without copying or moving from a `try_set` argument
and without invoking a `T` constructor from `try_emplace`. C++ evaluates argument expressions
before the call as usual; the guarantee concerns work performed by `set_once` after entry.

Construction exceptions propagate to the caller. If construction throws, the slot remains unset
and a later setting attempt may retry. The mutation functions make no public `noexcept` guarantee.

# Observation

`has_value()` reports whether the slot is set and is `noexcept`.

`value()` is an lvalue-only observer returning `const T&`. It throws `std::bad_optional_access`
when the slot is unset. Restricting it to lvalues prevents obtaining a reference from a temporary
`set_once`.

The API deliberately does not provide `operator bool`, `operator*`, `operator->`, mutable
observation, or an escape to the underlying `std::optional`. The named operations keep the
transition rule visible and avoid reproducing the full optional interface.

# Copy and move

Copy construction is available exactly when the owned representation can copy-construct `T`.
Move construction is available exactly when it can move-construct `T`. Both operations preserve
whether the newly constructed slot is unset or set.

Moving a set object does not reset its source slot. The destination contains the move-constructed
`T`; the source remains set and contains a valid but otherwise type-defined moved-from `T`. A
second setting attempt on either object is rejected.

Copy assignment and move assignment are always deleted. Assignment would otherwise create a
replacement path or require a failure policy that C++ assignment syntax cannot state clearly.

# Contained-value boundary

The guarantee covers slot assignment, not deep immutability of `T`. `value()` does not expose a
mutable `T&` through `set_once`, but mutation may still be possible through:

* an alias retained before the value was stored;
* state shared by `T` with another object;
* a const member of `T` that mutates reachable state; or
* a pointer-like `T` whose const interface provides mutable access to its pointee.

Those effects do not replace the contained `T` and are outside this contract. Callers needing
deep immutability, conflict detection, lifecycle deadlines, non-nullness, or same-value retry
semantics need a different, more specific abstraction.

# Concurrency and ownership

`set_once` owns its contained value and releases it when the holder is destroyed. It performs no
allocation beyond whatever behavior `T` itself has.

Concurrent access follows ordinary C++ data-race rules. The type provides no synchronization and
is not an alternative to `std::once_flag`, `std::call_once`, atomics, or a mutex-protected state
machine.

# API admission summary

The admitted rule is the slot transition itself: `unset -> set`, with every later setting attempt
rejected. It is mechanically enforced, visible in signatures, implemented as a thin layer over
`std::optional`, and does not require hidden state or a transition framework.

Unlike `std::optional<T>`, the public surface has no reset, replacement, or assignment path. The
type is appropriate only when this narrower transition is itself the reusable semantic contract;
domain-specific non-null, conflict, or lifecycle policies must not be generalized away merely
because they also contain an unset-to-set transition.
