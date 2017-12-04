#!/bin/sh
HERE="$(dirname "$(readlink -f "${0}")")"
export LD_LIBRARY_PATH="${HERE}/usr/lib/x86_64-linux-gnu":"${HERE}":$LD_LIBRARY_PATH

"${HERE}/fwupgrader" "$@"
