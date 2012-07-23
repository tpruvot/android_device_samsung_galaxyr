# This script is included in releasetools/addons
# It is the final build step (after OTA package)

# To remember :
# DEVICE_OUT=$ANDROID_BUILD_TOP/out/target/product/galaxyr
# DEVICE_TOP=$ANDROID_BUILD_TOP/device/samsung/galaxyr
# VENDOR_TOP=$ANDROID_BUILD_TOP/vendor/samsung/galaxyr

echo "addons.sh: $1"

if [ -z "$1" ]; then
    echo "addons: missing addon type"
fi

if [ "$1" = "recovery" ]; then
	cat $DEVICE_TOP/releasetools/updater-addons-recovery > $REPACK/ota/META-INF/com/google/android/updater-script
	rm -rf $REPACK/ota/system
	cp $DEVICE_OUT/recovery.img $REPACK/ota/
	OUTFILE=$OUT/recovery-$CM_BUILD.zip
fi

if [ "$1" = "kernel" ]; then
        cat $DEVICE_TOP/releasetools/updater-addons-kernel > $REPACK/ota/META-INF/com/google/android/updater-script
        cp $DEVICE_OUT/boot.img $REPACK/ota/
        OUTFILE=$OUT/kernel-$CM_BUILD.zip
fi

