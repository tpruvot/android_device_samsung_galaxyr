#!/system/bin/sh

export PATH=/system/bin:$PATH

lpm_boot=`cat /sys/module/kernel/parameters/lpm_boot`

if [ "$lpm_boot" = "1" ]; then
    setprop sys.chargeonly.mode 1
fi

