#!/usr/bin/env sh

set -e

pkg update
pkg install git ninja nasm python312 sdl2 ccache jq llvm20 uv 

ln -sf /usr/local/llvm20/lib/clang/20 /usr/lib/clang/20
