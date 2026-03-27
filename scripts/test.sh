#!/bin/bash
# AMLP Test Script - builds test target and runs tests
# ALL output captured to logs/. Nothing is lost.
set -uo pipefail

PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
LOG_DIR="$PROJECT_ROOT/logs"
BUILD_DIR="$PROJECT_ROOT/build"
TIMESTAMP=$(date +%Y%m%d_%H%M%S)
CORES=$(sysctl -n hw.ncpu 2>/dev/null || nproc 2>/dev/null || echo 4)

mkdir -p "$LOG_DIR"

TEST_LOG="$LOG_DIR/test_latest.log"
ERROR_LOG="$LOG_DIR/errors_latest.log"
TEST_ARCHIVE="$LOG_DIR/test_${TIMESTAMP}.log"

{
echo "=== AMLP Tests Started: $(date) ==="

# Step 1: Build test target
echo "--- Building test target ---"
BUILD_EXIT=0
cmake --build "$BUILD_DIR" --target AMLPTests -j"$CORES" 2>&1 | tee "$TEST_LOG.build" || BUILD_EXIT=$?

if [ $BUILD_EXIT -ne 0 ]; then
    echo "Test build: FAILED (exit $BUILD_EXIT)"
    echo "=== TEST BUILD ERRORS ($TIMESTAMP) ===" >> "$ERROR_LOG"
    grep -E "(error:|warning:|fatal error:)" "$TEST_LOG.build" >> "$ERROR_LOG" 2>/dev/null || true
    echo "" >> "$ERROR_LOG"
    echo "=== LAST 100 LINES ===" >> "$ERROR_LOG"
    tail -100 "$TEST_LOG.build" >> "$ERROR_LOG"
    cat "$TEST_LOG.build" >> "$TEST_LOG"
    rm -f "$TEST_LOG.build"
    exit $BUILD_EXIT
fi
echo "Test build: OK"
cat "$TEST_LOG.build" >> "$TEST_LOG"
rm -f "$TEST_LOG.build"

# Step 2: Find test binary
echo "--- Running tests ---"
TEST_BINARY=""
for candidate in \
    "$BUILD_DIR/tests/AMLPTests_artefacts/AMLPTests" \
    "$BUILD_DIR/AMLPTests_artefacts/AMLPTests" \
    "$BUILD_DIR/tests/AMLPTests_artefacts/AMLPTests.app/Contents/MacOS/AMLPTests" \
    "$BUILD_DIR/AMLPTests_artefacts/AMLPTests.app/Contents/MacOS/AMLPTests"; do
    if [ -f "$candidate" ]; then
        TEST_BINARY="$candidate"
        break
    fi
done

if [ -z "$TEST_BINARY" ]; then
    echo "ERROR: Test binary not found" | tee -a "$ERROR_LOG"
    echo "Searched build directory:" | tee -a "$ERROR_LOG"
    find "$BUILD_DIR" -name "AMLPTests" -type f 2>/dev/null | tee -a "$ERROR_LOG"
    exit 1
fi
echo "Test binary: $TEST_BINARY"

# Step 3: Run tests, capture all output
TEST_EXIT=0
"$TEST_BINARY" > "$TEST_LOG.run" 2>&1 || TEST_EXIT=$?

# Always save full output
cat "$TEST_LOG.run" >> "$TEST_LOG"
cp "$TEST_LOG" "$TEST_ARCHIVE"

# Extract the summary line from our test runner
echo ""
grep -E "(=== AMLP|Tests run:|Passes:|Failures:|FAILED:|No AMLP)" "$TEST_LOG.run" 2>/dev/null || true

if [ $TEST_EXIT -ne 0 ]; then
    echo ""
    echo "Tests: FAILED (exit $TEST_EXIT)"
    # Only put failures into error log, not the entire output
    echo "=== TEST FAILURES ($TIMESTAMP) ===" >> "$ERROR_LOG"
    grep -E "(FAILED:|Failures:|error|=== .* FAILED)" "$TEST_LOG.run" >> "$ERROR_LOG" 2>/dev/null || true
    echo "" >> "$ERROR_LOG"
    echo "Full test output in: $TEST_LOG" >> "$ERROR_LOG"
    rm -f "$TEST_LOG.run"
    exit $TEST_EXIT
fi

rm -f "$TEST_LOG.run"
echo ""
echo "Tests: PASSED"
echo "=== Tests Complete: $(date) ==="

} 2>&1 | tee "$LOG_DIR/pipeline_test_latest.log"
