#!/bin/bash
# MyPL test runner.
# - Positive tests in tests/*.logic must parse and run with exit code 0.
# - Negative tests in tests/negative/*.logic must produce a parse error
#   (non-zero exit) — except files starting with `err_undefined_` or
#   `err_arity_` which are *runtime* failures that still exit 0 with
#   "false" output.

set -u
cd "$(dirname "$0")/.."

MYPL=./mypl
if [ ! -x "$MYPL" ]; then
    echo "build mypl first (make)"
    exit 2
fi

pass=0
fail=0
failures=""

run_positive() {
    local f=$1
    if "$MYPL" run "$f" >/dev/null 2>&1; then
        pass=$((pass+1))
    else
        fail=$((fail+1))
        failures+=$'\n  + '"$f (expected exit 0)"
    fi
}

run_negative_parse() {
    local f=$1
    if "$MYPL" run "$f" >/dev/null 2>&1; then
        fail=$((fail+1))
        failures+=$'\n  - '"$f (expected parse error, got exit 0)"
    else
        pass=$((pass+1))
    fi
}

run_negative_runtime() {
    local f=$1
    local out
    out=$("$MYPL" run "$f" 2>&1) || { fail=$((fail+1)); failures+=$'\n  - '"$f (parse failed, expected runtime false)"; return; }
    if echo "$out" | grep -q "false"; then
        pass=$((pass+1))
    else
        fail=$((fail+1))
        failures+=$'\n  - '"$f (expected 'false' in output)"
    fi
}

echo "=== Positive tests (tests/*.logic) ==="
for f in tests/*.logic; do
    [ -f "$f" ] || continue
    run_positive "$f"
done

echo "=== Examples (examples/*.logic) ==="
for f in examples/*.logic; do
    [ -f "$f" ] || continue
    run_positive "$f"
done

echo "=== Negative tests (tests/negative/*.logic) ==="
for f in tests/negative/*.logic; do
    [ -f "$f" ] || continue
    case "$(basename "$f")" in
        err_undefined_*|err_arity_*) run_negative_runtime "$f" ;;
        *) run_negative_parse "$f" ;;
    esac
done

echo
echo "passed: $pass"
echo "failed: $fail"
if [ "$fail" -gt 0 ]; then
    echo "failures:$failures"
    exit 1
fi
