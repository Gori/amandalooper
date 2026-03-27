#!/bin/bash
# AMLP Build Script - captures ALL output to logs/
# Every line of stdout and stderr is preserved. Nothing is lost.
set -uo pipefail

PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
LOG_DIR="$PROJECT_ROOT/logs"
BUILD_DIR="$PROJECT_ROOT/build"
TIMESTAMP=$(date +%Y%m%d_%H%M%S)
CORES=$(sysctl -n hw.ncpu 2>/dev/null || nproc 2>/dev/null || echo 4)

mkdir -p "$LOG_DIR"

CMAKE_LOG="$LOG_DIR/cmake_latest.log"
BUILD_LOG="$LOG_DIR/build_latest.log"
ERROR_LOG="$LOG_DIR/errors_latest.log"

# Also keep timestamped archives
CMAKE_ARCHIVE="$LOG_DIR/cmake_${TIMESTAMP}.log"
BUILD_ARCHIVE="$LOG_DIR/build_${TIMESTAMP}.log"

# Clear error log at start
> "$ERROR_LOG"

{
echo "=== AMLP Build Started: $(date) ==="
echo "Build dir: $BUILD_DIR"
echo "Cores: $CORES"
echo ""

# Step 1: CMake configure
echo "--- CMake Configure ---"
CMAKE_EXIT=0
cmake -B "$BUILD_DIR" "$PROJECT_ROOT" > "$CMAKE_LOG" 2>&1 || CMAKE_EXIT=$?
cp "$CMAKE_LOG" "$CMAKE_ARCHIVE"

if [ $CMAKE_EXIT -ne 0 ]; then
    echo "CMake configure: FAILED (exit $CMAKE_EXIT)"
    echo "=== CMAKE CONFIGURE ERRORS ($TIMESTAMP) ===" >> "$ERROR_LOG"
    cat "$CMAKE_LOG" >> "$ERROR_LOG"
    exit $CMAKE_EXIT
fi
echo "CMake configure: OK"
echo ""

# Step 2: Build - capture everything
echo "--- Build (${CORES} cores) ---"
BUILD_EXIT=0
cmake --build "$BUILD_DIR" -j"$CORES" 2>&1 | tee -a "$BUILD_LOG.tmp" || BUILD_EXIT=$?

# Move to final location
mv "$BUILD_LOG.tmp" "$BUILD_LOG" 2>/dev/null || true
cp "$BUILD_LOG" "$BUILD_ARCHIVE"

# Always extract warnings and errors (even on success)
WARNINGS=$(grep -cE "warning:" "$BUILD_LOG" 2>/dev/null || true)
WARNINGS=${WARNINGS:-0}
ERRORS=$(grep -cE "(^.*error:|fatal error:)" "$BUILD_LOG" 2>/dev/null || true)
ERRORS=${ERRORS:-0}

if [ $BUILD_EXIT -ne 0 ]; then
    echo ""
    echo "Build: FAILED (exit $BUILD_EXIT)"
    echo "=== BUILD ERRORS ($TIMESTAMP) ===" >> "$ERROR_LOG"
    echo "Errors: $ERRORS, Warnings: $WARNINGS" >> "$ERROR_LOG"
    echo "" >> "$ERROR_LOG"
    grep -E "(error:|warning:|fatal error:|undefined|linker)" "$BUILD_LOG" >> "$ERROR_LOG" 2>/dev/null || true
    echo "" >> "$ERROR_LOG"
    echo "=== LAST 150 LINES OF BUILD OUTPUT ===" >> "$ERROR_LOG"
    tail -150 "$BUILD_LOG" >> "$ERROR_LOG"
    exit $BUILD_EXIT
fi

echo ""
echo "Build: OK (warnings: $WARNINGS)"

# Even on success, save warnings to error log
if [ "$WARNINGS" -gt 0 ]; then
    echo "=== BUILD WARNINGS ($TIMESTAMP) ===" >> "$ERROR_LOG"
    grep -E "warning:" "$BUILD_LOG" >> "$ERROR_LOG" 2>/dev/null || true
else
    echo "=== NO ERRORS OR WARNINGS - Build succeeded: $(date) ===" >> "$ERROR_LOG"
fi

echo "=== Build Complete: $(date) ==="

} 2>&1 | tee "$LOG_DIR/pipeline_latest.log"
