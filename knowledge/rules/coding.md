---
type: Project Rule
title: Coding Rules
description: Source-level conventions that apply to code authored for Precept.
status: draft
sources:
  - id: adr-0003
    resource: ../decisions/adr-0003-apache-2-0.md
    title: License Precept under Apache-2.0
  - id: apache-license-faq
    resource: https://www.apache.org/foundation/license-faq.html
    title: Apache Licensing and Distribution FAQ
    author: organization:apache-software-foundation
tags: [rules, coding, licensing, spdx]
---

# Source license headers

Every C++ source and header file authored for Precept starts with this short license header:

```cpp
// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0
```

Do not paste the full Apache-2.0 license notice into each source file. The repository-level
`LICENSE` file must carry the complete license text. Generated files and vendored third-party
sources retain the notices required by their origin and are not rewritten to use Precept's
header.

The Apache Software Foundation distinguishes its own CLA-related source header from the
guidance for applying Apache-2.0 to software owned outside the ASF. Its licensing FAQ lists
the copyright line plus `SPDX-License-Identifier: Apache-2.0` as the shorter variant for
an owner applying Apache-2.0 to their own software.
