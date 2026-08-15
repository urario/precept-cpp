---
type: Project Rule
title: Public Header Layout Rule
description: Defines how Precept public headers are grouped, when a public subdirectory is justified, and why the span family is grouped separately.
status: draft
sources:
  - id: issue-70
    resource: https://github.com/urario/precept-cpp/issues/70
    title: Public header layout and include-tree architecture review
    author: chatgpt/gpt-5.6-sol
  - id: issue-70-review
    resource: https://github.com/urario/precept-cpp/issues/70#issuecomment-5302123709
    title: Architecture and consumer-view review conclusion
    author: chatgpt/gpt-5.6-sol
  - id: pr-71-review
    resource: https://github.com/urario/precept-cpp/pull/71#pullrequestreview-4943817959
    title: Review requiring family-promotion and header-relocation compatibility rules
    author: human:urario
tags: [architecture, public-api, headers, include-tree, compatibility]
---

# Purpose

The public include tree is part of Precept's consumer-facing API. This rule defines how public
headers are placed so that contributors can extend the library without inventing a directory
taxonomy for every new semantic vocabulary.

This rule applies to headers installed under `include/precept/`. It does not prescribe the layout
of private implementation files, tests, examples, or the knowledge bundle itself.

# Root is the default

A new public API goes directly under `precept/` unless it belongs to an already established,
cohesive public API family.

For example:

```cpp
#include <precept/nonzero.hpp>
#include <precept/index_below.hpp>
#include <precept/set_once.hpp>
#include <precept/same_size.hpp>
```

A new directory is not created merely because an API can be described as a property, relation,
transition, integer utility, pointer utility, factory, or another abstract architectural category.
Those categories help reason about API design, but consumers should not need to learn that taxonomy
to predict an include path.

# Public subdirectories represent user-facing API families

A public subdirectory represents a family that users can recognize and browse as one coherent set
of alternatives. It does not represent an implementation category or simply the type currently
used as an operand.

Creating a public subdirectory requires all of the following:

1. More than one public API belongs to the proposed family.
2. The APIs share a user-facing problem domain, not only an implementation technique or C++ type.
3. Grouping improves discovery: a user browsing the directory benefits from seeing the members
   together.
4. The family boundary can be stated without requiring users to understand Precept's internal
   semantic taxonomy.
5. The expected benefit is greater than the extra include-path rule users must remember.

File count alone is not a reason to create a directory. A single API remains at the root even when
it may later become the first member of a family.

# `precept/span/` is the single-span refinement family

`precept/span/` is the established family for APIs that refine or validate the size or shape of one
`std::span` value itself.

The current family is:

```text
precept/span/
├── at_least_span.hpp
├── block_span.hpp
├── checked_span.hpp
└── non_empty_span.hpp
```

These APIs answer questions about one span such as:

* whether its runtime size satisfies an exact or minimum bound,
* whether its element count forms complete fixed-size blocks, or
* whether the span is non-empty.

The directory is therefore **not** a bucket for every API that accepts `std::span`.

# Operand type does not determine header location

An API stays at the root when its primary semantic is independent of the single-span refinement
family, even if its current operands are spans.

For example:

```cpp
#include <precept/same_size.hpp>
#include <precept/non_overlapping.hpp>
```

`checked_same_size(...)` proves a binary cardinality relation between two span snapshots.
`checked_non_overlapping(...)` proves a binary storage relation between two span snapshots.
Their primary contract is the relation between two objects, not a refinement of one span.

Moving these headers to `precept/span/` merely because they accept spans would make the operand type,
rather than the semantic contract, determine the public architecture.

# Do not mirror the semantic design taxonomy into include paths

Precept uses concepts such as property, factory, transition, relation, and proof carrier when
designing APIs. They are intentionally not mirrored mechanically as public directories such as:

```text
precept/property/
precept/relation/
precept/transition/
precept/integer/
precept/pointer/
```

Many useful APIs legitimately fit more than one such label. For example, a bounded integer is both
a numeric type and a property carrier, while a one-way value holder can be described in terms of
both transition and lifecycle semantics. Requiring one taxonomy choice would move design ambiguity
into every consumer include path.

A later public family may still justify a new directory, but it must satisfy the user-facing family
criteria above rather than merely correspond to an architectural label.

# Family promotion does not silently relocate shipped headers

A root header does not move merely because later APIs make a new public family possible. Forming a
family and relocating an already shipped include path are separate decisions.

The default is to preserve the existing root path. A new family may group later APIs without
forcing older headers to move when keeping those paths is clearer and avoids unnecessary migration.
Visual completeness of a directory is not a reason to break an include path.

If moving an existing header into a newly established family provides enough consumer value to
justify migration, use a compatibility transition instead of an immediate rename:

1. Add the new family path as the canonical location.
2. Keep the old public path as a forwarding header that includes the canonical header; do not
   duplicate declarations or definitions between the two paths.
3. Keep the forwarding path for at least one MINOR release before removal.
4. Document the preferred path and planned removal in the release or migration notes.
5. Remove the old path only at a compatibility boundary permitted by
   [ADR-0007](../decisions/adr-0007-versioning-compatibility-and-support.md).

This transition is for headers that have already shipped as public API. A path that has never been
released does not need a compatibility forwarding period.

# Header location is compatibility surface

The installed package copies the public include tree, so a source-tree path under `include/precept/`
becomes the path consumers write in `#include` directives.

Changing:

```cpp
#include <precept/span/at_least_span.hpp>
```

to:

```cpp
#include <precept/at_least_span.hpp>
```

is therefore a public compatibility decision, not repository cleanup.

A relocation must be reviewed under
[ADR-0007](../decisions/adr-0007-versioning-compatibility-and-support.md), which defines the
project's versioning and compatibility policy. When a relocation is chosen, its migration strategy
belongs in the decision that authorizes the move; the new location must not be introduced silently
as an internal refactor.

# Current layout decision

For v0.2, the existing layout is kept:

```text
include/precept/
├── aligned_ptr.hpp
├── index_below.hpp
├── narrow_exact.hpp
├── never_decrease.hpp
├── non_overlapping.hpp
├── nonzero.hpp
├── same_size.hpp
├── set_once.hpp
└── span/
    ├── at_least_span.hpp
    ├── block_span.hpp
    ├── checked_span.hpp
    └── non_empty_span.hpp
```

The asymmetry is intentional: root is the default, while `span/` is currently the only public API
family with enough cohesion and discovery value to justify a separate directory.

# Review checklist for a new public header

When adding a public header, review its location in this order:

1. **Start at root.** Assume `<precept/name.hpp>`.
2. **Check existing families.** Place it in one only if its primary user-facing contract belongs to
   that family.
3. **Do not classify by operand alone.** Using `std::span`, an integer, or a pointer does not by
   itself determine the directory.
4. **Do not create a one-file taxonomy.** A hypothetical future family is not a public directory.
5. **If proposing a new directory, explain the consumer benefit.** Show why grouping improves
   discovery and why the boundary is predictable to users.
6. **Do not relocate existing paths just to complete a family.** Treat family promotion and header
   migration as separate decisions.
7. **Treat moves as API changes.** Review compatibility and forwarding requirements before
   relocating an existing header.

The goal is not a visually balanced tree. The goal is a public include structure that stays small,
predictable, and useful to consumers as the semantic vocabulary grows.
