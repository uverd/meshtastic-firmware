#!/usr/bin/env bash

set -e

echo "This script requires https://jpa.kapsi.fi/nanopb/download/ version 0.4.9 to be located in the"
echo "firmware root directory if the following step fails, you should download the correct"
echo "prebuilt binaries for your computer into nanopb-0.4.9"

# the nanopb tool seems to require that the .options file be in the current directory!
#cd protobufs
#../nanopb-0.4.9/generator-bin/protoc --experimental_allow_proto3_optional "--nanopb_out=-S.cpp -v:../src/mesh/generated/" -I=../protobufs meshtastic/*.proto



echo "→ locating script and repo root"
# Resolve the directory this script lives in, then back up to the firmware root\ n
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
REPO_ROOT="$SCRIPT_DIR/.."

echo "→ cleaning out old generated files"
rm -f "$REPO_ROOT/src/mesh/generated/"*.pb.c \
      "$REPO_ROOT/src/mesh/generated/"*.pb.h
rm -f "$REPO_ROOT/src/mesh/generated/"*.pb.{c,h,cpp}
rm -rf "$REPO_ROOT/src/mesh/generated/meshtastic"
mkdir -p "$REPO_ROOT/src/mesh/generated"

echo "→ entering protobufs submodule"
cd "$REPO_ROOT/protobufs"

echo "→ regenerating nanopb sources into src/mesh/generated/"
PROTOC="$REPO_ROOT/nanopb-0.4.9/generator-bin/protoc"
PLUGIN="$REPO_ROOT/nanopb-0.4.9/generator-bin/protoc-gen-nanopb"

$PROTOC \
  --plugin=protoc-gen-nanopb="$PLUGIN" \
  --experimental_allow_proto3_optional \
  --nanopb_out=strip_import_prefix=meshtastic:../src/mesh/generated \
  -I . \
  meshtastic/*.proto

echo "→ fixing include paths in generated files"
find "$REPO_ROOT/src/mesh/generated" -type f \( -name '*.pb.c' -o -name '*.pb.h' -o -name '*.pb.cpp' \) \
  -exec sed -i 's|#include "meshtastic/|#include "|g' {} +

echo "✅ done!"
