#!/bin/sh
set -e
. ./config.sh

mkdir -p "$SYSROOT"

for PROJECT in $SYSTEM_HEADER_PROJECTS; do
  (cd "$PROJECT" && DESTDIR="$SYSROOT" $MAKE install-headers)
done

for PROJECT in $PROJECTS; do
  (cd "$PROJECT" && DESTDIR="$SYSROOT" $MAKE install)
done

mkdir -p isodir/boot/grub

cp sysroot/boot/os.kernel isodir/boot/os.kernel

cat >isodir/boot/grub/grub.cfg <<EOF
menuentry "HDH" {
	multiboot /boot/os.kernel
}
EOF

grub-mkrescue -o HDH.iso isodir

qemu-system-$(./arch.sh $HOST) -cdrom HDH.iso