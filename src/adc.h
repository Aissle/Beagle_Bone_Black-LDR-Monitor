#ifndef ADC_H
#define ADC_H

/**
 * adc.h - BeagleBone Black ADC interface
 *
 * The BBB exposes 7 ADC inputs (AIN0–AIN6) on the P9 header via sysfs:
 *   /sys/bus/iio/devices/iio:device0/in_voltage<N>_raw
 *
 * Returns a value between 0–4095 (12-bit resolution, 0–1.8V range).
 * NOTE: BBB ADC max is 1.8V — never exceed this on AIN pins!
 */

/**
 * adc_read() - Read raw ADC value from a given AIN channel (0–6)
 * Returns: 0–4095 on success, -1 on error
 */
int adc_read(int channel);

/**
 * adc_to_voltage() - Convert raw ADC value to voltage (0.0 – 1.8V)
 */
float adc_to_voltage(int raw);

/**
 * adc_to_percent() - Convert raw ADC value to light percentage (0–100%)
 * Higher value = more light (assumes standard LDR voltage divider wiring)
 */
int adc_to_percent(int raw);

#endif /* ADC_H */
