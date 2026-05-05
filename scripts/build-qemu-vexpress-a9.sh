#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BSP_DIR="${ROOT_DIR}/bsp/qemu-vexpress-a9"

SCONS_CMD=""
if command -v scons >/dev/null 2>&1; then
  SCONS_CMD="scons"
elif [ -x "${HOME}/.local/bin/scons" ]; then
  SCONS_CMD="${HOME}/.local/bin/scons"
elif python3 -c "import SCons" >/dev/null 2>&1; then
  SCONS_CMD="python3 -m SCons.Script"
else
  echo "Error: scons not found."
  echo "Install with: pip install --user scons"
  echo "Or add ~/.local/bin to PATH."
  exit 1
fi

if ! command -v arm-none-eabi-gcc >/dev/null 2>&1; then
  echo "Error: arm-none-eabi-gcc not found in PATH."
  echo "Please install GNU Arm Embedded Toolchain."
  exit 1
fi

echo "[build] BSP: ${BSP_DIR}"
cd "${BSP_DIR}"
${SCONS_CMD} -j"$(nproc)"

echo "[build] done"
echo "[build] ELF: ${BSP_DIR}/rtthread.elf"
