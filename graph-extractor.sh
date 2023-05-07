#!/bin/bash

set -euo pipefail

WD=$(realpath $(dirname $0))
TEST_DIR="${WD}/tests"

install_prerequisites() {
    sudo apt install cpanminus
    sudo cpanm Graph::Easy
}

# process_file ${TEST_FILE}
process_file() {
    local TEST_FILE="${1}"
    local TEST_NAME="$(echo ${TEST_FILE} | sed -E 's@^.*/(\w+)_test.cpp$@\1@g')"
    
    local TEST_LINES=()
    for L in $(grep -no "TEST(.*,.*)" ${TEST_FILE} | grep -Eo "^([0-9]+)"); do
        TEST_LINES+=( "${L}" )
    done

    local OUT_DIR=".dot/${TEST_NAME}"
    mkdir -p ${OUT_DIR}

    for L in ${!TEST_LINES[@]}; do
        local TEST_START=${TEST_LINES[${L}]}
        local TEST_END=${TEST_LINES[$(( $L + 1 ))]-"$"}
        local OUT_FILE_DOT="${OUT_DIR}/${TEST_LINES[${L}]}.dot"
        local OUT_FILE_ASCII="${OUT_DIR}/${TEST_LINES[${L}]}.txt"

        echo "graph ${TEST_NAME}_${TEST_LINES[${L}]} {" > ${OUT_FILE_DOT}
        sed -n ${TEST_START},${TEST_END}p ${TEST_FILE} | sed -En 's@.*SetSuccessors\((\w+), (.*)\).*@\1 -> \2@p' | sed 's@,@ @g' >> ${OUT_FILE_DOT}
        echo "}" >> ${OUT_FILE_DOT}

        echo "/*" > ${OUT_FILE_ASCII}
        cat ${OUT_FILE_DOT} | graph-easy --from=dot --as_ascii >> ${OUT_FILE_ASCII}
        echo "*/" >> ${OUT_FILE_ASCII}
    done
}

process_files() {
    for F in $(find ${TEST_DIR} -name "*_test.cpp"); do
        echo ${F}
        process_file "${F}"
    done
}

install_prerequisites
process_files
