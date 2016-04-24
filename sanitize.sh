#!/bin/sh

sanitize() {
    FILE=$(echo "$1" | rev | cut -d"/" -f1 | rev)
    cat $1 \
        | sed -e s/Σ/symbolCount/g  \
        | sed -e s/δ/delta/g        \
        | sed -e s/₀/0/g            \
        | sed -e s/ₛ/s/g            \
        | sed -e s/′/prime/g        \
        | sed -e s/≡/equivalence/g  \
        | sed -e s/⁻/minus/g        \
        | sed -e s/⁺/plus/g        \
        | sed -e s/τ/tau/g          \
        | sed -e s/σ/sigma/g        \
        > build/$FILE
}

sanitize_all() {
    while (( "$#" )); do
        sanitize $1
        shift
    done
}

sanitize_all $@
