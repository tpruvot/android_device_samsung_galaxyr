/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//#define LOG_NDEBUG 0

#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <poll.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/select.h>
#include <cutils/log.h>

#include "GyroSensor.h"

#include "kernel/mpu.h"

#define FETCH_FULL_EVENT_BEFORE_RETURN 0
#define IGNORE_EVENT_TIME 350000000

#define TAG "[MPU] "

/*****************************************************************************/

GyroSensor::GyroSensor()
    : SensorBase("/dev/mpu", "mpu-accel"),
      mEnabled(0),
      mInputReader(6),
      mHasPendingEvent(false),
      mEnabledTime(0),
      mDelay_ns(IGNORE_EVENT_TIME),
      mInitialTestDone(false)
{
    mPendingEvent.version = sizeof(sensors_event_t);
    mPendingEvent.sensor = ID_GY;
    mPendingEvent.type = SENSOR_TYPE_GYROSCOPE;
    memset(mPendingEvent.data, 0, sizeof(mPendingEvent.data));

    if (data_fd) {
        strcpy(input_sysfs_path, "/sys/class/input/");
        strcat(input_sysfs_path, input_name);
        strcat(input_sysfs_path, "/device/");
        input_sysfs_path_len = strlen(input_sysfs_path);
        enable(0, 1);
    }
}

GyroSensor::~GyroSensor() {
    if (mEnabled) {
        enable(0, 0);
    }
}

int GyroSensor::setInitialState() {
    struct input_absinfo absinfo_x;
    struct input_absinfo absinfo_y;
    struct input_absinfo absinfo_z;
    float value;
    unsigned char dmp_cfg;

    if (!mInitialTestDone)
        sec_runtest();

    LOGV(TAG "%s: data_fd=%d", __FUNCTION__, data_fd);

    if (!ioctl(data_fd, MPU_GET_DMP_CFG1, &dmp_cfg)) {
        LOGV(TAG "ioctl success : %d", dmp_cfg);
    }

    if (!ioctl(data_fd, EVIOCGABS(EVENT_TYPE_GYRO_X), &absinfo_x) &&
        !ioctl(data_fd, EVIOCGABS(EVENT_TYPE_GYRO_Y), &absinfo_y) &&
        !ioctl(data_fd, EVIOCGABS(EVENT_TYPE_GYRO_Z), &absinfo_z)) {
        value = absinfo_x.value;
        mPendingEvent.data[0] = value * CONVERT_GYRO_X;
        value = absinfo_y.value;
        mPendingEvent.data[1] = value * CONVERT_GYRO_Y;
        value = absinfo_z.value;
        mPendingEvent.data[2] = value * CONVERT_GYRO_Z;
        mHasPendingEvent = true;
    }
    return 0;
}

int GyroSensor::sec_power(int en) {
    int flags = en ? 1 : 0;
    if (flags != mEnabled) {
        int fd;
        char sec_sysfs[] = "/sys/class/sec/sec_mpu3050/gyro_power_on";

        LOGV(TAG "%s(%d) %s", __FUNCTION__, en, sec_sysfs);
        fd = open(input_sysfs_path, O_RDWR);
        if (fd >= 0) {
            char buf[2];
            int err;
            buf[1] = 0;
            if (flags) {
                buf[0] = '1';
            } else {
                buf[0] = '0';
            }
            err = write(fd, buf, sizeof(buf));
            close(fd);

            mEnabled = flags;
            return 0;
        }
        LOGE(TAG "%s failed, err %d", __FUNCTION__, errno);
        return -1;
    }
    return 0;
}

/* beware, slow, 2 seconds */
int GyroSensor::sec_runtest()
{
    struct input_absinfo bias_x;
    struct input_absinfo bias_y;
    struct input_absinfo bias_z;

    struct input_absinfo rms_x;
    struct input_absinfo rms_y;
    struct input_absinfo rms_z;

    if (!mEnabled)
	enable(0, 1);

    if (mEnabled) {
        int nr;
        FILE *fd;

        LOGV(TAG "%s()", __FUNCTION__);

        fd = fopen("/sys/class/sec/sec_mpu3050/gyro_selftest", "r");
        if (fd) {
            char buf[32]="", pfx[3]="";

            fgets(&buf[0], sizeof(buf), fd);
            fclose(fd);

            nr = sscanf(buf, "%2s, %d, %d, %d, %d, %d, %d", pfx,
                        &bias_x.value, &bias_y.value, &bias_z.value,
			&rms_x.value, &rms_y.value, &rms_z.value);

            LOGV(TAG "read %s : %d/%d/%d - rms : %d/%d/%d", pfx,
		 bias_x.value, bias_y.value, bias_z.value,
		 rms_x.value, rms_y.value, rms_z.value);

            mInitialTestDone = true;
            return 0;
        }
        LOGE(TAG "%s failed, err %d", __FUNCTION__, errno);
        return -1;
    }
    return 0;
}

int GyroSensor::enable(int32_t, int en)
{
    int flags = en ? 1 : 0;
    if (flags != mEnabled) {
        int fd;
        strcpy(&input_sysfs_path[input_sysfs_path_len], "enable");

        LOGV(TAG "%s(%d) %s", __FUNCTION__, en, input_sysfs_path);
        fd = open(input_sysfs_path, O_RDWR);
        if (fd >= 0) {
            char buf[2];
            int err;
            buf[1] = 0;
            if (flags) {
//              sec_power(flags);
                buf[0] = '1';
                mEnabledTime = getTimestamp() + mDelay_ns;
            } else {
                buf[0] = '0';
            }
            err = write(fd, buf, sizeof(buf));
            close(fd);
            mEnabled = flags;
            sec_power(flags);
            setInitialState();
            return 0;
        }
        LOGE(TAG "%s enable failed, err %d", __FUNCTION__, errno);
        return -1;
    }
    return 0;
}

bool GyroSensor::hasPendingEvents() const {
    return mHasPendingEvent;
}

int GyroSensor::setDelay(int32_t handle, int64_t delay_ns)
{
    int fd;

    mDelay_ns = delay_ns;

    strcpy(&input_sysfs_path[input_sysfs_path_len], "poll_delay");
    fd = open(input_sysfs_path, O_RDWR);
    if (fd >= 0) {
        char buf[80];
        sprintf(buf, "%lld", delay_ns / 1000000);
        write(fd, buf, strlen(buf)+1);
        close(fd);

        LOGV(TAG "%s: %lld ms", __FUNCTION__, delay_ns / 1000000);
        return 0;
    }

    LOGW(TAG "%s: %lld ms, error", __FUNCTION__, delay_ns / 1000000);
    return -1;
}

int GyroSensor::readEvents(sensors_event_t* data, int count)
{
    if (count < 1)
        return -EINVAL;

    if (mHasPendingEvent) {
        mHasPendingEvent = false;
        mPendingEvent.timestamp = getTimestamp();
        *data = mPendingEvent;
        return mEnabled ? 1 : 0;
    }

    ssize_t n = mInputReader.fill(data_fd);
    if (n < 0)
        return n;

    int numEventReceived = 0;
    input_event const* event;

    int timeout = 64;
#if FETCH_FULL_EVENT_BEFORE_RETURN
again:
#endif
    while (count && mInputReader.readEvent(&event)) {
        int type = event->type;

        if (type == EV_REL) {
            float value = event->value;
            if (event->code == 0 /* EVENT_TYPE_GYRO_X */) {
                mPendingEvent.data[0] = value * CONVERT_GYRO_X;
            } else if (event->code == 1 /* EVENT_TYPE_GYRO_Y */) {
                mPendingEvent.data[1] = value * CONVERT_GYRO_Y;
            } else if (event->code == 2 /* EVENT_TYPE_GYRO_Z */) {
                mPendingEvent.data[2] = value * CONVERT_GYRO_Z;
            } else {
                LOGW(TAG "unhandled EV_REL code %d", event->code);
            }
        } else if (type == EV_SYN) {
            mPendingEvent.timestamp = timevalToNano(event->time);
            if (mEnabled) {
                if (mPendingEvent.timestamp >= mEnabledTime) {
                    *data++ = mPendingEvent;
                    numEventReceived++;
                    LOGV(TAG "event processed %1.2f/%1.2f/%1.2f",
			mPendingEvent.data[0],
			mPendingEvent.data[1],
			mPendingEvent.data[2]);
                }
                count--;
            }
        } else {
            LOGE(TAG "unknown event (type=0x%x, code=0x%x)",
                    type, event->code);
        }
        mInputReader.next();
    }

    usleep(10000);
#if FETCH_FULL_EVENT_BEFORE_RETURN
    /* if we didn't read a complete event, see if we can fill and
       try again instead of returning with nothing and redoing poll. */
    if (numEventReceived == 0 && mEnabled == 1 && --timeout > 0) {
        n = mInputReader.fill(data_fd);
        if (n)
            goto again;
    }
#endif

    mEnabledTime = getTimestamp() + mDelay_ns;

    return numEventReceived;
}

