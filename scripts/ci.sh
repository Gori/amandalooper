#!/bin/bash
# AMLP CI Script - full build + test pipeline
# ALL output from every step is captured in logs/
# After running, check: logs/errors_latest.log
set -uo pipefail

PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
LOG_DIR="$PROJECT_ROOT/logs"
TIMESTAMP=$(date +%Y%m%d_%H%M%S)

mkdir -p "$LOG_DIR"

CI_LOG="$LOG_DIR/ci_latest.log"
ERROR_LOG="$LOG_DIR/errors_latest.log"

# Clear error log at start of CI run
> "$ERROR_LOG"

{
echo "========================================"
echo "  AMLP CI Pipeline"
echo "  $TIMESTAMP"
echo "========================================"
echo ""

# Build
echo ">>> STEP 1: BUILD"
BUILD_EXIT=0
"$PROJECT_ROOT/scripts/build.sh" || BUILD_EXIT=$?

if [ $BUILD_EXIT -ne 0 ]; then
    echo ""
    echo ">>> CI FAILED at BUILD step (exit $BUILD_EXIT)"
    echo ">>> Errors saved to: logs/errors_latest.log"
    exit $BUILD_EXIT
fi

echo ""

# Tests
echo ">>> STEP 2: TESTS"
if grep -rq "AMLPTests" "$PROJECT_ROOT/CMakeLists.txt" "$PROJECT_ROOT/tests/CMakeLists.txt" 2>/dev/null; then
    TEST_EXIT=0
    "$PROJECT_ROOT/scripts/test.sh" || TEST_EXIT=$?

    if [ $TEST_EXIT -ne 0 ]; then
        echo ""
        echo ">>> CI FAILED at TEST step (exit $TEST_EXIT)"
        echo ">>> Errors saved to: logs/errors_latest.log"
        exit $TEST_EXIT
    fi
else
    echo "No test target found, skipping tests."
fi

echo ""
echo "========================================"
echo "  CI PASSED"
echo "  Logs: $LOG_DIR/"
echo "========================================"

} 2>&1 | tee "$CI_LOG"

# Final: copy ci log to archive
cp "$CI_LOG" "$LOG_DIR/ci_${TIMESTAMP}.log"
