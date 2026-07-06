# BBB LDR Light Monitor

A C project for the BeagleBone Black that reads a light-dependent resistor (LDR)
via the onboard ADC and serves a live web page showing the current light level.

---

## Project Structure

```
bbb-ldr-monitor/
├── src/
│   ├── main.c          # Entry point — main loop, config constants
│   ├── adc.c / .h      # Low-level ADC sysfs reader
│   ├── sensor.c / .h   # LDR reading + data struct (SensorReading)
│   └── webserver.c/.h  # Minimal HTTP server (single-threaded)
└── README.md
```

---

## Wiring the LDR

Use a **voltage divider** so the LDR's resistance is converted to a voltage:

```
1.8V ──┬── LDR (e.g. 10kΩ in light) ──┬── AIN0 (P9 pin 39)
       │                               │
       └── fixed 10kΩ resistor ────────┴── GND (P9 pin 34)
```

- **IMPORTANT:** BBB ADC max input is **1.8V**. Never connect 3.3V or 5V directly to AIN pins.
- AIN0 = P9 pin 39, GND = P9 pin 34, 1.8V ref = P9 pin 32

---

## Device Tree Overlay (run once on BBB)

The ADC cape overlay must be loaded before the program will work:

```bash
sudo sh -c "echo 'BB-ADC' > /sys/devices/platform/bone_capemgr/slots"
```

To make it permanent, add to `/boot/uEnv.txt`:
```
cape_enable=bone_capemgr.enable_partno=BB-ADC
```

Verify it worked:
```bash
ls /sys/bus/iio/devices/iio:device0/
# Should show: in_voltage0_raw, in_voltage1_raw, etc.
```

---

## Building

**Syntax check on your PC:**
```bash
make
```

compile natively on the BBB:
```bash
# On BBB:
sudo apt install build-essential
make check   # uses regular gcc
./bin/ldr_monitor_pc
```

---

## Running

```bash
./ldr_monitor_bbb
```

Output:
```
=== BBB LDR Light Monitor ===
ADC channel : AIN0
Web server  : http://0.0.0.0:8080
Poll rate   : 3s

[WEB] Listening on http://0.0.0.0:8080
[MAIN] Open http://192.168.7.2:8080 in your browser.

[14:22:01] Light:  72% | Bright     | raw=2949 | 1.295V
[14:22:04] Light:  38% | Moderate   | raw=1557 | 0.684V
```

Open `http://192.168.7.2:8080` in your browser to see the live dashboard.

---

## Configuration (in main.c)

| Constant          | Default | Description                        |
|-------------------|---------|------------------------------------|
| `LDR_ADC_CHANNEL` | `0`     | AIN channel (0–6)                  |
| `HTTP_PORT`       | `8080`  | Web server port                    |
| `POLL_INTERVAL_S` | `3`     | Seconds between sensor reads       |

---

## Known Limitations / Next Steps

- **Single-threaded:** The web server blocks on `accept()`, so sensor reads
  only happen when a browser connects. Adding `select()` or a second thread
  would allow continuous polling independent of web traffic.
- **No history:** Only the latest reading is shown. A future enhancement
  could log to a CSV file and render a chart.
- **No authentication:** Anyone on the local network can see the page.


## BBB Checklist
1. Boot and connect
bashssh debian@192.168.7.2
2. Install GCC
bashsudo apt install build-essential
3. Load the ADC overlay
bashsudo sh -c "echo 'BB-ADC' > /sys/devices/platform/bone_capemgr/slots"
4. Copy your source files from PC to BBB
bashscp -r bbb-ldr-monitor/src/ debian@192.168.7.2:/home/debian/bbb-ldr-monitor/
5. Compile directly on the BBB with GCC
bashcd bbb-ldr-monitor
gcc -Wall -o ldr_monitor src/main.c src/sensor.c src/adc.c src/webserver.c
6. Run it
bash./ldr_monitor
Then open http://192.168.7.2:8080 in your browser.



# =========================================================
# Makefile — BBB LDR Light Monitor
#
# Targets:
#   make          → syntax-check build on your PC (won't run)
#   make check    → same as above, explicit
#   make bbb      → cross-compile for BeagleBone Black (ARM)
#   make clean    → remove build artifacts
# =========================================================

CC_HOST  = gcc
CC_BBB   = arm-linux-gnueabihf-gcc

CFLAGS   = -Wall -Wextra -Wpedantic -std=c99 -I./src
SRCS     = src/main.c src/sensor.c src/adc.c src/webserver.c
TARGET   = ldr_monitor

.PHONY: all check bbb clean

# Default: compile for PC to catch syntax/type errors
# Will not run correctly (no /sys/bus/iio/... on PC)
all: check

check:
	@mkdir -p bin
	$(CC_HOST) $(CFLAGS) $(SRCS) -o bin/$(TARGET)_pc
	@echo ""
	@echo "✓ Syntax check passed — binary is for PC only (will not run correctly)"
	@echo "  Use 'make bbb' to build for BeagleBone Black"

# Cross-compile for BBB (ARM Cortex-A8, hard-float ABI)
bbb:
	@mkdir -p bin
	$(CC_BBB) $(CFLAGS) $(SRCS) -o bin/$(TARGET)_bbb
	@echo ""
	@echo "✓ BBB binary ready: bin/$(TARGET)_bbb"
	@echo "  Copy to BBB: scp bin/$(TARGET)_bbb debian@192.168.7.2:/home/debian/"

clean:
	rm -rf bin/# Beagle_Bone_Black-LDR-Monitor
