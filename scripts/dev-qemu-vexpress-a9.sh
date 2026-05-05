#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_SCRIPT="${ROOT_DIR}/scripts/build-qemu-vexpress-a9.sh"
RUN_SCRIPT="${ROOT_DIR}/scripts/run-qemu-vexpress-a9-gdb.sh"

SKIP_BUILD=0

usage() {
  cat <<'EOF'
Usage:
  ./scripts/dev-qemu-vexpress-a9.sh [--skip-build]

Options:
  --skip-build   Do not build, only run QEMU in gdb-wait mode.
  -h, --help     Show this help message.
EOF
}

for arg in "$@"; do
  case "$arg" in
    --skip-build)
      SKIP_BUILD=1
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      echo "Unknown argument: $arg"
      usage
      exit 1
      ;;
  esac
done

if [ "${SKIP_BUILD}" -eq 0 ]; then
  "${BUILD_SCRIPT}"
fi

echo "[dev] start qemu with gdb wait mode..."
echo "[dev] then press F5 in VS Code."
"${RUN_SCRIPT}"
