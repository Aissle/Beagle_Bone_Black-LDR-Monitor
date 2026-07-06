#include <time.h>
#include "sensor.h"
#include "adc.h"

SensorReading sensor_read_ldr(int adc_channel) {
    SensorReading r;
    int raw = adc_read(adc_channel);

    r.timestamp = time(NULL);

    if (raw < 0) {
        r.raw     = -1;
        r.voltage = -1.0f;
        r.percent = -1;
        r.valid   = 0;
    } else {
        r.raw     = raw;
        r.voltage = adc_to_voltage(raw);
        r.percent = adc_to_percent(raw);
        r.valid   = 1;
    }

    return r;
}

const char *sensor_label(int percent) {
    if (percent < 0)  return "Error";
    if (percent < 10) return "Dark";
    if (percent < 30) return "Dim";
    if (percent < 60) return "Moderate";
    if (percent < 85) return "Bright";
    return "Very Bright";
}
