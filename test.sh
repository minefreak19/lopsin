#!/bin/sh

set -xe

cd examples/lopasm
LOPASMFLAGS=$* make -B
cd ../..
