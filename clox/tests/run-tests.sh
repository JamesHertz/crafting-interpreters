#!/usr/bin/env bash

TMP_FILE=/tmp/out.txt
USAGE="usage $0: [ --all | --help | <test-name> ]"

function echo() {
    command echo -e $*
}

function error() {
    echo $1 1>&2
    exit 1
}

function get_test_name() {
    echo ${1/.lox/}
}

function run_test() {
    local test_name=$1
    local out="$test_name.out"
    local in="$test_name.lox"

    if ! [ -f "$test_name.lox" ] ; then 
        error "test file '$test_name.lox' not found"
    fi

    local passed=0
    if [ -f "$test_name.out" ] ; then
        ../bin/clox "$in" > "$TMP_FILE"
        diff "$out" "$TMP_FILE" > /dev/null
        passed=$?
    else
        ../bin/clox "$in" > /dev/null 2> "$TMP_FILE"
        [ $? -eq 0 ] && passed=1
    fi

    [ $passed -eq 0 ] && echo -n "\033[0;32mPASSED" || echo -n "\033[0;31mFAILED"
    echo "\033[0m $test_name"
    return $passed
}

function run_all_test() {
    local errors=0
    local test_count=0
    for file in *.lox ; do 
        run_test $(get_test_name $file) || ((errors++))
        ((test_count++))
    done

    local passed=$((test_count - errors))
    echo "ran $test_count tests where $passed passed and $errors failed"
}

function main() {
    case "$1" in 
        --help|'') echo $USAGE ;;
        --all) run_all_test ;;
        -*) error "Error: invalid option '$1'\n$USAGE" ;;
        *) 
            local test_name=$(get_test_name $1)
            local out="$test_name.out"
            if ! run_test "$test_name" && [ -f "$out" ]; then
                vimdiff $TMP_FILE "$out"
            fi
    esac
}

main $*
