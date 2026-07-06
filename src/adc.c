#include <stdio.h>
#include "adc.h"

#define ADC_SYSFS_PATH "/sys/bus/iio/devices/iio:device0/in_voltage%d_raw"
#define ADC_MAX_RAW    3950 //4095
#define ADC_MAX_VOLTS  1.8f

int adc_read(int channel) {
    char path[64];
    FILE *f;
    int raw = -1;

    if (channel < 0 || channel > 6) {
        fprintf(stderr, "[ADC] Invalid channel %d (must be 0–6)\n", channel);
        return -1;
    }

    snprintf(path, sizeof(path), ADC_SYSFS_PATH, channel);

    f = fopen(path, "r");
    if (!f) {
        perror("[ADC] Failed to open sysfs ADC file");
        fprintf(stderr, "[ADC] Path: %s\n", path);
        fprintf(stderr, "[ADC] Hint: ensure the BB-ADC device tree overlay is loaded.\n");
        return -1;
    }

    if (fscanf(f, "%d", &raw) != 1) {
        fprintf(stderr, "[ADC] Failed to read value from %s\n", path);
        fclose(f);
        return -1;
    }

    fclose(f);
    return raw;
}

float adc_to_voltage(int raw) {
    if (raw < 0) return -1.0f;
    return ((float)raw / ADC_MAX_RAW) * ADC_MAX_VOLTS;
}

int adc_to_percent(int raw) {
    if (raw < 0) return -1;
    /* Invert if needed depending on your LDR wiring:
     * If LDR is between AIN and GND (voltage rises with more light): no invert
     * If LDR is between VCC and AIN (voltage drops with more light): invert */
    return (raw * 100) / ADC_MAX_RAW;
}
