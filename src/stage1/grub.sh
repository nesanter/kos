#!/bin/bash

echo "menuentry "$OS_NAME" {
    multiboot /boot/kernel.bin $BOOT_CMDLINE" > $OUT/grub.cfg

for M in $BOOT_MODULES ; do
    MNAME=$(sed 's/\//_/g' <<< $M)
    if [ -e src/$M/cmdline.sh ] ; then
        echo "    module /boot/modules/m_$MNAME.bin $(./src/$M/cmdline.sh)" >> $OUT/grub.cfg
    else
        echo "    module /boot/modules/m_$MNAME.bin" >> $OUT/grub.cfg
    fi
done

echo "}" >> $OUT/grub.cfg
