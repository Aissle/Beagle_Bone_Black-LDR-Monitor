#ifndef SENSOR_H
#define SENSOR_H

#include <time.h>

/**
 * sensor.h - Shared sensor data structure
 *
 * SensorReading is the central data type passed between
 * the ADC reader, the web server, and main.
 */

typedef struct {
    int   raw;          /* Raw ADC value: 0–4095 */
    float voltage;      /* Converted voltage: 0.0–1.8V */
    int   percent;      /* Light level percentage: 0–100 */
    time_t timestamp;   /* Unix timestamp of the reading */
    int   valid;        /* 1 if reading succeeded, 0 if error */
} SensorReading;

/**
 * sensor_read_ldr() - Read LDR value from the given ADC channel.
 * Populates and returns a SensorReading struct.
 */
SensorReading sensor_read_ldr(int adc_channel);

/**
 * sensor_label() - Return a human-readable light level label.
 * e.g. "Dark", "Dim", "Moderate", "Bright", "Very Bright"
 */
const char *sensor_label(int percent);

#endif /* SENSOR_H */
