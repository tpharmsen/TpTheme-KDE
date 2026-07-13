#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd desklets/
rm -rf build/
mkdir -p build
cd build
cmake ..
make -j$(nproc)

