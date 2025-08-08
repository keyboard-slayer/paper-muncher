#!/usr/bin/env sh

set -e

if [ "$(id -u)" -eq 0 ]; then
    echo "Please do not run this script as root."

    if [ "$CUTEKIT_ALLOW_ROOT" == "1" ]; then
        echo "CUTEKIT_ALLOW_ROOT is set, continuing..."
    else
        echo "If you know what you are doing, set CUTEKIT_ALLOW_ROOT=1 and try again."
        echo "Aborting."
        exit 1
    fi
fi

. ./meta/scripts/setup-any.sh
meta/scripts/cutekit-wrapper.py $@
