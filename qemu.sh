#!/bin/sh
set -e
. ./iso.sh

qemu-system-$(./arch.sh $HOST) -cdrom HDH.iso