#!/bin/sh

AUTOMAKE_OPTIONS="--add-missing"

check_tool() {
    $1 --version > /dev/null 2>&1
    if [ $? -ne 0 ]; then
        echo "Error: $1 is required to run $0"
        exit 1
    fi
}

run_tool() {
    echo "Running $1..."
    eval $1 \$`echo $1 | tr '[[:lower:]]' '[[:upper:]]'`_OPTIONS
}

RUN_TOOLS="aclocal autoconf autoheader automake"

for tool in $RUN_TOOLS; do
    check_tool $tool
done

for tool in $RUN_TOOLS; do
    run_tool $tool
done
