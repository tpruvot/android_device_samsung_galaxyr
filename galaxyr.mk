#
# This file is the build configuration for a full Android
# build for sapphire hardware. This cleanly combines a set of
# device-specific aspects (drivers) with a device-agnostic
# product configuration (apps).

## Inherit from those products. Most specific first.
$(call inherit-product, $(SRC_TARGET_DIR)/product/languages_full.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/full.mk)

#DEVICE_PACKAGE_OVERLAYS := device/samsung/cooper/overlay

# proprietary side of the device
#$(call inherit-product-if-exists, vendor/samsung/galaxyr/device-vendor.mk)

# Discard inherited values and use our own instead.
PRODUCT_NAME := galaxyr
PRODUCT_DEVICE := galaxyr
PRODUCT_MODEL := GT-I9103

ifeq ($(TARGET_PREBUILT_KERNEL),)
	LOCAL_KERNEL := device/samsung/galaxyr/kernel
else
	LOCAL_KERNEL := $(TARGET_PREBUILT_KERNEL)
endif

PRODUCT_COPY_FILES += \
    $(LOCAL_KERNEL):kernel

PRODUCT_PACKAGES += \
    LiveWallpapers \
    LiveWallpapersPicker \
    VisualizationWallpapers \
    MagicSmokeWallpapers \
    VisualizationWallpapers \
    HoloSpiralWallpaper \
    Gallery3d \
    SpareParts \
    Development \
    Term \
    libOmxCore \
    libOmxVidEnc \
    sec_touchscreen.kcm \
    dexpreopt \
    com.android.future.usb.accessory \
    galaxyrSettings \
    SamsungServiceMode \
    Torch \
    Galaxy4 \
    NoiseField \
    PhaseBeam \
    librs_jni

# Filesystem management tools
PRODUCT_PACKAGES += \
    static_busybox \
    make_ext4fs \
    setup_fs

PRODUCT_PACKAGES += \
    sensors.n1 \
    sensors.tegra \
    lights.tegra \
    gps.n1 \
    gralloc.tegra \
    overlay.tegra

PRODUCT_PACKAGES += RomUpdater DroidSSHd

# Set true if you want .odex files
DISABLE_DEXPREOPT := false

# INIT-scripts
PRODUCT_COPY_FILES += \
    device/samsung/galaxyr/init.n1.rc:root/init.n1.rc \
    device/samsung/galaxyr/ueventd.n1.rc:root/ueventd.n1.rc

# Prebuilt modules
PRODUCT_COPY_FILES += \
    device/samsung/galaxyr/prebuilt/dhd.ko:root/lib/modules/dhd.ko \
    device/samsung/galaxyr/prebuilt/bthid.ko:root/lib/modules/bthid.ko \
    device/samsung/galaxyr/prebuilt/scsi_wait_scan.ko:root/lib/modules/scsi_wait_scan.ko \
    device/samsung/galaxyr/prebuilt/Si4709_driver.ko:root/lib/modules/Si4709_driver.ko \
    device/samsung/galaxyr/prebuilt/modules.dep:root/lib/modules/modules.dep

# Vold and Storage
PRODUCT_COPY_FILES += \
    device/samsung/galaxyr/configs/vold.fstab:system/etc/vold.fstab

# Wifi
PRODUCT_COPY_FILES += \
    device/samsung/galaxyr/configs/wifi.conf:system/etc/wifi/wifi.conf \
    device/samsung/galaxyr/configs/wpa_supplicant.conf:system/etc/wifi/wpa_supplicant.conf

# GPS
PRODUCT_COPY_FILES += \
    device/samsung/galaxyr/configs/gps.conf:system/etc/gps.conf \
    device/samsung/galaxyr/configs/sirfgps.conf:system/etc/sirfgps.conf

# Media
PRODUCT_COPY_FILES += \
	device/samsung/galaxyr/configs/media_profiles.xml:system/etc/media_profiles.xml

# Keylayout
PRODUCT_COPY_FILES += \
    device/samsung/galaxyr/usr/keylayout/AVRCP.kl:system/usr/keylayout/AVRCP.kl \
    device/samsung/galaxyr/usr/keylayout/Broadcom_Bluetooth_HID.kl:system/usr/keylayout/Broadcom_Bluetooth_HID.kl \
    device/samsung/galaxyr/usr/keylayout/qwerty.kl:system/usr/keylayout/qwerty.kl \
    device/samsung/galaxyr/usr/keylayout/sec_jack.kl:system/usr/keylayout/sec_jack.kl \
    device/samsung/galaxyr/usr/keylayout/sec_key.kl:system/usr/keylayout/sec_key.kl \
    device/samsung/galaxyr/usr/keylayout/sec_touchscreen.kl:system/usr/keylayout/sec_touchscreen.kl

# Keychars
PRODUCT_COPY_FILES += \
    device/samsung/galaxyr/usr/keychars/Broadcom_Bluetooth_HID.kcm.bin:system/usr/keychars/Broadcom_Bluetooth_HID.kcm.bin \
    device/samsung/galaxyr/usr/keychars/qwerty2.kcm.bin:system/usr/keychars/qwerty2.kcm.bin \
    device/samsung/galaxyr/usr/keychars/qwerty.kcm.bin:system/usr/keychars/qwerty.kcm.bin \
    device/samsung/galaxyr/usr/keychars/sec_jack.kcm.bin:system/usr/keychars/sec_jack.kcm.bin \
    device/samsung/galaxyr/usr/keychars/sec_key.kcm.bin:system/usr/keychars/sec_key.kcm.bin \
    device/samsung/galaxyr/usr/keychars/sec_touchscreen.kcm.bin:system/usr/keychars/sec_touchscreen.kcm.bin

# Install the features available on this device.
PRODUCT_COPY_FILES += \
    frameworks/base/data/etc/handheld_core_hardware.xml:system/etc/permissions/handheld_core_hardware.xml \
    frameworks/base/data/etc/android.hardware.audio.low_latency.xml:system/etc/permissions/android.hardware.audio.low_latency.xml \
    frameworks/base/data/etc/android.hardware.camera.xml:system/etc/permissions/android.hardware.camera.xml \
    frameworks/base/data/etc/android.hardware.camera.autofocus.xml:system/etc/permissions/android.hardware.camera.autofocus.xml \
    frameworks/base/data/etc/android.hardware.camera.flash-autofocus.xml:system/etc/permissions/android.hardware.camera.flash-autofocus.xml \
    frameworks/base/data/etc/android.hardware.camera.front.xml:system/etc/permissions/android.hardware.camera.front.xml \
    frameworks/base/data/etc/android.hardware.location.gps.xml:system/etc/permissions/android.hardware.location.gps.xml \
    frameworks/base/data/etc/android.hardware.location.xml:system/etc/permissions/android.hardware.location.xml \
    frameworks/base/data/etc/android.hardware.sensor.proximity.xml:system/etc/permissions/android.hardware.sensor.proximity.xml \
    frameworks/base/data/etc/android.hardware.sensor.accelerometer.xml:system/etc/permissions/android.hardware.sensor.accelerometer.xml \
    frameworks/base/data/etc/android.hardware.sensor.compass.xml:system/etc/permissions/android.hardware.sensor.compass.xml \
    frameworks/base/data/etc/android.hardware.sensor.gyroscope.xml:system/etc/permissions/android.hardware.sensor.gyroscope.xml \
    frameworks/base/data/etc/android.hardware.sensor.light.xml:system/etc/permissions/android.hardware.sensor.light.xml \
    frameworks/base/data/etc/android.software.sip.voip.xml:system/etc/permissions/android.software.sip.voip.xml \
    frameworks/base/data/etc/android.software.sip.xml:system/etc/permissions/android.software.sip.xml \
    frameworks/base/data/etc/android.hardware.telephony.gsm.xml:system/etc/permissions/android.hardware.telephony.gsm.xml \
    frameworks/base/data/etc/android.hardware.telephony.cdma.xml:system/etc/permissions/android.hardware.telephony.cdma.xml \
    frameworks/base/data/etc/android.hardware.touchscreen.xml:system/etc/permissions/android.hardware.touchscreen.xml \
    frameworks/base/data/etc/android.hardware.touchscreen.multitouch.xml:system/etc/permissions/android.hardware.touchscreen.multitouch.xml \
    frameworks/base/data/etc/android.hardware.touchscreen.multitouch.distinct.xml:system/etc/permissions/android.hardware.touchscreen.multitouch.distinct.xml \
    frameworks/base/data/etc/android.hardware.touchscreen.multitouch.jazzhand.xml:system/etc/permissions/android.hardware.touchscreen.multitouch.jazzhand.xml \
    frameworks/base/data/etc/android.hardware.usb.accessory.xml:system/etc/permissions/android.hardware.usb.accessory.xml \
    frameworks/base/data/etc/android.hardware.wifi.xml:system/etc/permissions/android.hardware.wifi.xml

# Feature live wallpaper
PRODUCT_COPY_FILES += \
    packages/wallpapers/LivePicker/android.software.live_wallpaper.xml:system/etc/permissions/android.software.live_wallpaper.xml

# Overlay to set device specific parameters
DEVICE_PACKAGE_OVERLAYS := device/samsung/galaxyr/overlay

PRODUCT_PROPERTY_OVERRIDES += \
    ro.ril.enable.3g.prefix=1 \
    ro.ril.enable.a53=1 \
    ro.ril.enable.dtm=1 \
    ro.ril.enable.managed.roaming=1 \
    ro.ril.gprsclass=12 \
    ro.ril.hep=1 \
    ro.ril.hsdpa.category=8 \
    ro.ril.hsupa.category=5 \
    ro.ril.hsxpa=2 \
    rild.libpath=/system/lib/libsec-ril.so \
    rild.libargs="-d /dev/ttys0"

# The OpenGL ES API level that is natively supported by this device.
# This is a 16.16 fixed point number
PRODUCT_PROPERTY_OVERRIDES := \
    ro.opengles.version=131072

# These are the hardware-specific settings that are stored in system properties.
# Note that the only such settings should be the ones that are too low-level to
# be reachable from resources or other mechanisms.
PRODUCT_PROPERTY_OVERRIDES += \
    wifi.interface=eth0 \
    wifi.supplicant_scan_interval=30 \
    ro.board.platform=tegra \
    ro.sf.lcd_density=240 \
    ro.telephony.ril_class=samsung \
    ro.telephony.sends_barcount=1 \
    ro.com.android.dataroaming=false \
    mobiledata.interfaces=eth0,rmnet0,rmnet1,rmnet2 \
    dalvik.vm.heapsize=64m \
    persist.service.usb.setting=0 \
    dev.sfbootcomplete=0 \
    persist.sys.vold.switchexternal=1

# enable Google-specific location features,
# like NetworkLocationProvider and LocationCollector
PRODUCT_PROPERTY_OVERRIDES += \
    ro.com.google.locationfeatures=1 \
    ro.com.google.networklocation=1

# Extended JNI checks
# The extended JNI checks will cause the system to run more slowly, but they can spot a variety of nasty bugs 
# before they have a chance to cause problems.
# Default=true for development builds, set by android buildsystem.
PRODUCT_PROPERTY_OVERRIDES += \
    ro.kernel.android.checkjni=0 \
    dalvik.vm.checkjni=false

PRODUCT_PROPERTY_OVERRIDES += \
    ro.com.android.dateformat=yyyy-MM-dd \
    ro.setupwizard.enable_bypass=1 \
    ro.media.dec.jpeg.memcap=20000000 \
    dalvik.vm.lockprof.threshold=500 \
    dalvik.vm.dexopt-flags=m=y \
    dalvik.vm.heapsize=64m \
    dalvik.vm.execution-mode=int:jit \
    dalvik.vm.dexopt-data-only=1 \
    hwui.render_dirty_regions=false \
    ro.compcache.default=0 \
    media.stagefright.enable-player=false \
    media.stagefright.enable-meta=false \
    media.stagefright.enable-scan=false \
    media.stagefright.enable-http=true \
    media.stagefright.enable-rtsp=false \
    ro.tether.denied=false \
    ro.flash.resolution=1080

# we have enough storage space to hold precise GC data
PRODUCT_TAGS += dalvik.gc.type-precise

# Set default USB interface
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += \
    persist.sys.usb.config=mass_storage

PRODUCT_LOCALES += hdpi

# See comment at the top of this file. This is where the other
# half of the device-specific product definition file takes care
# of the aspects that require proprietary drivers that aren't
# commonly available

$(call inherit-product-if-exists, vendor/samsung/galaxyr/galaxyr-vendor-blobs.mk)
