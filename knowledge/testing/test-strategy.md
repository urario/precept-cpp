---
type: Test Strategy
title: Precept Test Strategy
description: Keep CTest as the common entry point while assigning each kind of property to the smallest suitable test mechanism.
status: draft
sources:
  - id: issue-2
    resource: https://github.com/urario/precept-cpp/issues/2
    title: Foundation issue for the C++20 header-only build, test, and CI environment
    author: human:urario
  - id: pull-request-12-review
    resource: https://github.com/urario/precept-cpp/pull/12#issuecomment-5282522532
    title: Review establishing the minimum test strategy knowledge for the build foundation
    author: human:urario
  - id: issue-8
    resource: https://github.com/urario/precept-cpp/issues/8
    title: Documentation issue introducing the runnable examples under CTest
    author: human:urario
tags: [testing, ctest, googletest, build]
---

# Common entry point

CTest is the common test execution entry point for local development and CI. Contributors
configure with `BUILD_TESTING=ON`, build normally, and run `ctest`. Tests are added only
when Precept is the top-level project.

# Test responsibilities

* Runtime behavior uses GoogleTest. GoogleTest is a development-only dependency linked
  only by runtime test executables; it is never a usage requirement of `Precept::Precept`.
* Compile-time properties use `static_assert` and normal compilation.
* Positive compile tests are ordinary CMake build targets.
* Negative compile tests do not run inside GoogleTest. Add a focused CMake compile check
  only when a public API has a constraint whose rejection behavior needs verification.
* Knowledge bundle validation runs under CTest as a development-only check, so an ordinary test
  run covers the knowledge bundle as well as the code.
* Examples in `examples/` are built and run under CTest, because documentation that stops
  compiling is worse than no documentation. They link `Precept::Precept` only: an example shows
  what a consumer writes, so a test framework in it would misrepresent the library's cost. What
  an example asserts about behavior belongs in a GoogleTest case as well, where a failure names
  the property that broke.
* Installed-package and consumer smoke tests are owned by issue #9.

Configuring with `BUILD_TESTING=OFF` must not fetch or require GoogleTest. Development
warnings and test dependencies remain private to development targets and do not propagate
to consumers.

# Deferred quality gates

Sanitizers are reconsidered after production API code is introduced; the foundation does
not add a sanitizer abstraction or CI matrix before there is production behavior to test.
