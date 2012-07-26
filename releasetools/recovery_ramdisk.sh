#!/bin/bash

DEVICE_OUT=$ANDROID_BUILD_TOP/out/target/product/galaxyr
DEVICE_TOP=$ANDROID_BUILD_TOP/device/samsung/galaxyr
VENDOR_TOP=$ANDROID_BUILD_TOP/vendor/samsung/galaxyr

# remove kernel modules, recovery size is limited to 5.2MB
rm -rf $DEVICE_OUT/recovery/root/lib/modules

# cleanup
rm -f $DEVICE_OUT/recovery/root/init.goldfish.rc
rm -f $DEVICE_OUT/recovery/root/ueventd.goldfish.rc
