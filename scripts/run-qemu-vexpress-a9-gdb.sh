#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BSP_DIR="${ROOT_DIR}/bsp/qemu-vexpress-a9"
SD_IMAGE="${BSP_DIR}/sd.bin"
KERNEL_BIN="${BSP_DIR}/rtthread.bin"
QEMU_BIN="${QEMU_BIN:-qemu-system-arm}"
GDB_PORT="${GDB_PORT:-1234}"

if ! command -v "${QEMU_BIN}" >/dev/null 2>&1; then
  echo "Error: ${QEMU_BIN} not found in PATH."
  echo "Install with: sudo apt install qemu-system-arm"
  exit 1
fi

if [ ! -f "${KERNEL_BIN}" ]; then
  echo "Error: ${KERNEL_BIN} not found."
  echo "Run ./scripts/build-qemu-vexpress-a9.sh first."
  exit 1
fi

if [ ! -f "${SD_IMAGE}" ]; then
  dd if=/dev/zero of="${SD_IMAGE}" bs=1024 count=65536 status=none
fi

# Make failures explicit when the previous QEMU session was not closed.
if ! python3 - "${GDB_PORT}" <<'PY'
import socket
import sys

port = int(sys.argv[1])
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
try:
    s.bind(("127.0.0.1", port))
except OSError:
    sys.exit(1)
finally:
    s.close()
PY
then
  echo "Error: gdb port ${GDB_PORT} is already in use."
  echo "Close previous QEMU session first (Ctrl+A, then X), then retry."
  exit 1
fi

cd "${BSP_DIR}"
echo "[qemu] waiting gdb on tcp::${GDB_PORT}"
echo "[qemu] press Ctrl+A then X to quit qemu"

exec "${QEMU_BIN}" \
  -M vexpress-a9 \
  -smp cpus=2 \
  -kernel "${KERNEL_BIN}" \
  -nographic \
  -sd "${SD_IMAGE}" \
  -S \
  -gdb "tcp::${GDB_PORT}"
