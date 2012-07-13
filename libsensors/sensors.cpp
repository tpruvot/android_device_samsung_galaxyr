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

#define LOG_TAG "Sensors"

#include <hardware/sensors.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <math.h>
#include <poll.h>
#include <pthread.h>
#include <stdlib.h>

#include <linux/input.h>

#include <utils/Atomic.h>
#include <utils/Log.h>

#include "sensors.h"

#include "MPLSensor.h"

#include "LightSensor.h"
#include "ProximitySensor.h"
#include "AkmSensor.h"

/*****************************************************************************/

#define DELAY_OUT_TIME 0x7FFFFFFF

#define LIGHT_SENSOR_POLLTIME    2000000000


#define SENSORS_ACCELERATION     (1<<ID_A)
#define SENSORS_MAGNETIC_FIELD   (1<<ID_M)
#define SENSORS_ORIENTATION      (1<<ID_O)
#define SENSORS_LIGHT            (1<<ID_L)
#define SENSORS_PROXIMITY        (1<<ID_P)
#define SENSORS_GYROSCOPE        (1<<ID_GY)
#define SENSORS_GRAVITY          (1<<ID_GR)
#define SENSORS_LINEAR_ACCEL     (1<<ID_LA)
#define SENSORS_ROTATION_VECTOR  (1<<ID_RV)

#define SENSORS_ACCELERATION_HANDLE     (ID_A)
#define SENSORS_MAGNETIC_FIELD_HANDLE   (ID_M)
#define SENSORS_ORIENTATION_HANDLE      (ID_O)
#define SENSORS_LIGHT_HANDLE            (ID_L)
#define SENSORS_PROXIMITY_HANDLE        (ID_P)
#define SENSORS_GYROSCOPE_HANDLE        (ID_GY)
#define SENSORS_GRAVITY_HANDLE          (ID_GR)
#define SENSORS_LINEAR_ACCEL_HANDLE     (ID_LA)
#define SENSORS_ROTATION_VECTOR_HANDLE  (ID_RV)

#define AKM_FTRACE 0
#define AKM_DEBUG 0
#define AKM_DATA 0

// akm 8975 is handled by mpl too
#define AKM_RAW 1

/*****************************************************************************/

#define LOCAL_SENSORS (2 + AKM_RAW)

/* The SENSORS Module */
static struct sensor_t sSensorList[LOCAL_SENSORS + MPLSensor::numSensors] = {
#if AKM_RAW
        { "AK8975 3-axis Magnetic field sensor",
          "Asahi Kasei Microdevices",
          1, SENSORS_MAGNETIC_FIELD_HANDLE,
          SENSOR_TYPE_MAGNETIC_FIELD, 2000.0f, CONVERT_M, 6.8f, 16667, { } },
#endif
        { "CM3663 Light sensor",
          "Capella Microsystems",
          1, SENSORS_LIGHT_HANDLE,
          SENSOR_TYPE_LIGHT, 10240.0f, 1.0f, 0.75f, 0, { } },
        { "CM3663 Proximity sensor",
          "Capella Microsystems",
          1, SENSORS_PROXIMITY_HANDLE,
          SENSOR_TYPE_PROXIMITY, 5.0f, 5.0f, 0.75f, 0, { } },
};
static int numSensors = LOCAL_SENSORS;

static int open_sensors(const struct hw_module_t* module, const char* id,
                        struct hw_device_t** device);


static int sensors__get_sensors_list(struct sensors_module_t* module,
                                     struct sensor_t const** list) 
{
        *list = sSensorList;
        return numSensors;
}

static struct hw_module_methods_t sensors_module_methods = {
        open: open_sensors
};

struct sensors_module_t HAL_MODULE_INFO_SYM = {
        common: {
                tag: HARDWARE_MODULE_TAG,
                version_major: 1,
                version_minor: 0,
                id: SENSORS_HARDWARE_MODULE_ID,
                name: "Samsung Sensor module",
                author: "Samsung Electronic Company",
                methods: &sensors_module_methods,
        },
        get_sensors_list: sensors__get_sensors_list,
};

struct sensors_poll_context_t {
    struct sensors_poll_device_t device; // must be first

        sensors_poll_context_t();
        ~sensors_poll_context_t();
    int activate(int handle, int enabled);
    int setDelay(int handle, int64_t ns);
    int pollEvents(sensors_event_t* data, int count);

private:
    enum {
        mpl = 0,          //all mpl entries must be consecutive and in this order
        mpl_accel,
        mpl_timer,
        light,
        proximity,
#if AKM_RAW
        akm,
#endif
        numSensorDrivers, // wake pipe goes here
        mpl_power,        // special handle for MPL pm interaction
        numFds,
    };

    static const size_t wake = numFds - 1;
    static const char WAKE_MESSAGE = 'W';
    struct pollfd mPollFds[numFds];
    int mWritePipeFd;
    SensorBase* mSensors[numSensorDrivers];

    int handleToDriver(int handle) const {
        switch (handle) {
            case ID_RV:
            case ID_LA:
            case ID_GR:
            case ID_GY:
            case ID_A:
            case ID_O:
                return mpl;
            case ID_M:
#if AKM_RAW
                return akm;
#else
                return mpl;
#endif
            case ID_P:
                return proximity;
            case ID_L:
                return light;
            default:
                LOGW(LOG_TAG ": unknown sensor handle %d", handle);
        }
        return -EINVAL;
    }
};

/*****************************************************************************/

sensors_poll_context_t::sensors_poll_context_t()
{
    MPLSensor* p_mplsen = new MPLSensor();
    setCallbackObject(p_mplsen); //setup the callback object for handing mpl callbacks

    numSensors = LOCAL_SENSORS;

    numSensors += p_mplsen->populateSensorList(sSensorList + LOCAL_SENSORS,
        sizeof(sSensorList[0]) * (ARRAY_SIZE(sSensorList) - LOCAL_SENSORS));

    mSensors[mpl] = p_mplsen;
    mPollFds[mpl].fd = mSensors[mpl]->getFd();
    mPollFds[mpl].events = POLLIN;
    mPollFds[mpl].revents = 0;

    mSensors[mpl_accel] = mSensors[mpl];
    mPollFds[mpl_accel].fd = ((MPLSensor*)mSensors[mpl])->getAccelFd();
    mPollFds[mpl_accel].events = POLLIN;
    mPollFds[mpl_accel].revents = 0;

    mSensors[mpl_timer] = mSensors[mpl];
    mPollFds[mpl_timer].fd = ((MPLSensor*)mSensors[mpl])->getTimerFd();
    mPollFds[mpl_timer].events = POLLIN;
    mPollFds[mpl_timer].revents = 0;

    mSensors[light] = new LightSensor();
    mPollFds[light].fd = mSensors[light]->getFd();
    mPollFds[light].events = POLLIN;
    mPollFds[light].revents = 0;

    mSensors[proximity] = new ProximitySensor();
    mPollFds[proximity].fd = mSensors[proximity]->getFd();
    mPollFds[proximity].events = POLLIN;
    mPollFds[proximity].revents = 0;

#if AKM_RAW
    mSensors[akm] = new AkmSensor();
    mPollFds[akm].fd = mSensors[akm]->getFd();
    mPollFds[akm].events = POLLIN;
    mPollFds[akm].revents = 0;
#endif

    int wakeFds[2];
    int result = pipe(wakeFds);
    LOGE_IF(result<0, "error creating wake pipe (%s)", strerror(errno));
    fcntl(wakeFds[0], F_SETFL, O_NONBLOCK);
    fcntl(wakeFds[1], F_SETFL, O_NONBLOCK);
    mWritePipeFd = wakeFds[1];

    mPollFds[wake].fd = wakeFds[0];
    mPollFds[wake].events = POLLIN;
    mPollFds[wake].revents = 0;

    //setup MPL pm interaction handle
    mPollFds[mpl_power].fd = ((MPLSensor*)mSensors[mpl])->getPowerFd();
    mPollFds[mpl_power].events = POLLIN;
    mPollFds[mpl_power].revents = 0;
}

sensors_poll_context_t::~sensors_poll_context_t() {
    for (int i=0 ; i<numSensorDrivers ; i++) {
        delete mSensors[i];
    }
    close(mPollFds[wake].fd);
    close(mWritePipeFd);
}

int sensors_poll_context_t::activate(int handle, int enabled) {
    int index = handleToDriver(handle);
    if (index < 0) return index;
    int err =  mSensors[index]->enable(handle, enabled);
    if (enabled && !err) {
        const char wakeMessage(WAKE_MESSAGE);
        int result = write(mWritePipeFd, &wakeMessage, 1);
        LOGE_IF(result<0, "error sending wake message (%s)", strerror(errno));
    }
    return err;
}

int sensors_poll_context_t::setDelay(int handle, int64_t ns) {

    int index = handleToDriver(handle);
    if (index < 0) return index;
    return mSensors[index]->setDelay(handle, ns);
}

int sensors_poll_context_t::pollEvents(sensors_event_t* data, int count)
{
    int nbEvents = 0;
    int n = 0;

    do {
        // see if we have some leftover from the last poll()
        for (int i=0 ; count && i<numSensorDrivers ; i++) {
            SensorBase* const sensor(mSensors[i]);
            if ((mPollFds[i].revents & POLLIN) || (sensor->hasPendingEvents())) {
                int nb = sensor->readEvents(data, count);
                if (nb < count) {
                    // no more data for this sensor
                    mPollFds[i].revents = 0;
                }
                count -= nb;
                nbEvents += nb;
                data += nb;

                //special handling for the mpl, which has multiple handles
                if (i == mpl) {
                    i += 2; //skip accel and timer
                    mPollFds[mpl_accel].revents = 0;
                    mPollFds[mpl_timer].revents = 0;
                }
                if (i == mpl_accel) {
                    i += 1; //skip timer
                    mPollFds[mpl_timer].revents = 0;
                }
            }
        }

        if (count) {
            // we still have some room, so try to see if we can get
            // some events immediately or just wait if we don't have
            // anything to return
            n = poll(mPollFds, numFds, nbEvents ? 0 : -1);
            if (n<0) {
                LOGE("poll() failed (%s)", strerror(errno));
                return -errno;
            }
            if (mPollFds[wake].revents & POLLIN) {
                char msg;
                int result = read(mPollFds[wake].fd, &msg, 1);
                LOGE_IF(result<0, "error reading from wake pipe (%s)", strerror(errno));
                LOGE_IF(msg != WAKE_MESSAGE, "unknown message on wake queue (0x%02x)", int(msg));
                mPollFds[wake].revents = 0;
            }
            if (mPollFds[mpl_power].revents & POLLIN) {
                ((MPLSensor*)mSensors[mpl])->handlePowerEvent();
                mPollFds[mpl_power].revents = 0;
            }
        }
        // if we have events and space, go read them
    } while (n && count);

    return nbEvents;
}

/*****************************************************************************/

static int poll__close(struct hw_device_t *dev)
{
    sensors_poll_context_t *ctx = (sensors_poll_context_t *)dev;
    if (ctx) {
        delete ctx;
    }
    return 0;
}

static int poll__activate(struct sensors_poll_device_t *dev,
        int handle, int enabled) {
    sensors_poll_context_t *ctx = (sensors_poll_context_t *)dev;
    return ctx->activate(handle, enabled);
}

static int poll__setDelay(struct sensors_poll_device_t *dev,
        int handle, int64_t ns) {
    sensors_poll_context_t *ctx = (sensors_poll_context_t *)dev;
    return ctx->setDelay(handle, ns);
}

static int poll__poll(struct sensors_poll_device_t *dev,
        sensors_event_t* data, int count) {
    sensors_poll_context_t *ctx = (sensors_poll_context_t *)dev;
    return ctx->pollEvents(data, count);
}

/*****************************************************************************/

/** Open a new instance of a sensor device using name */
static int open_sensors(const struct hw_module_t* module, const char* id,
                        struct hw_device_t** device)
{
        int status = -EINVAL;
        sensors_poll_context_t *dev = new sensors_poll_context_t();

        memset(&dev->device, 0, sizeof(sensors_poll_device_t));

        dev->device.common.tag = HARDWARE_DEVICE_TAG;
        dev->device.common.version  = 0;
        dev->device.common.module   = const_cast<hw_module_t*>(module);
        dev->device.common.close    = poll__close;
        dev->device.activate        = poll__activate;
        dev->device.setDelay        = poll__setDelay;
        dev->device.poll            = poll__poll;

        *device = &dev->device.common;
        status = 0;

        return status;
}

