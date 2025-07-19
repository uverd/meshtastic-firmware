#!/usr/bin/env bash
set -e
echo "This script requires https://jpa.kapsi.fi/nanopb/download/ version 0.4.9 to be located in the"
echo "firmware root directory if the following step fails, you should download the correct"
echo "prebuilt binaries for your computer into nanopb-0.4.9"

# Go into the protobuf directory
cd ../protobufs

# Generate the Meshtastic protos
../nanopb-0.4.9/generator/protoc \
    --experimental_allow_proto3_optional \
    "--nanopb_out=-S.cpp -v:../src/mesh/generated/" \
    -I=. \
    meshtastic/*.proto

# Go back to the original directory
cd - > /dev/null

echo "Protobuf generation complete."
