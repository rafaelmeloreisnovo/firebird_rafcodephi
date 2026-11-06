#!/bin/sh
# SPDX-License-Identifier: MIT
# Local-only; no GitHub Actions, network, database or installer.
set -eu
cd "$(dirname "$0")/.."
CC="${CC:-cc}"
OUT="${TMPDIR:-/tmp}/raf_nucleos_guard_test_$$"
trap 'rm -f "$OUT"' EXIT HUP INT TERM
"$CC" -std=c11 -O2 -ffreestanding -fno-builtin -Wall -Wextra -Werror -pedantic \
    -Iinclude src/raf_guard.c tests/test_guard.c -o "$OUT"
"$OUT"
