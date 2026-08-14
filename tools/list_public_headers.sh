#!/usr/bin/env bash
# Canonical, recursive list of Precept's public headers under include/precept/.
# CI's format/clang-tidy jobs and CONTRIBUTING's documented commands all call this
# script, so a new public header directory never needs a second place to widen a glob.
set -euo pipefail
cd "$(dirname "$0")/.."
find include/precept -name '*.hpp' | sort
