#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>

#include "sensor.h"
#include "webserver.h"

/* ----------------------------------------------------------------------- *
 * Configuration — change these to match your wiring
 * ----------------------------------------------------------------------- */
#define LDR_ADC_CHANNEL  0       /* AIN0 on P9 header pin 39 */
#define HTTP_PORT        8080    /* Visit http://<BBB-IP>:8080 in browser  */
#define POLL_INTERVAL_S  3       /* Seconds between sensor reads           */

/* ----------------------------------------------------------------------- */

static volatile int running = 1;

static void handle_signal(int sig) {
    (void)sig;
    printf("\n[MAIN] Shutting down...\n");
    running = 0;
}

int main(void) {
    int server_fd;
    SensorReading latest;

    /* Graceful shutdown on Ctrl+C */
    signal(SIGINT,  handle_signal);
    signal(SIGTERM, handle_signal);

    printf("=== BBB LDR Light Monitor ===\n");
    printf("ADC channel : AIN%d\n", LDR_ADC_CHANNEL);
    printf("Web server  : http://100.64.0.4:%d\n", HTTP_PORT);
    printf("Poll rate   : %ds\n\n", POLL_INTERVAL_S);

    /* Take an initial reading before accepting connections */
    latest = sensor_read_ldr(LDR_ADC_CHANNEL);
    if (!latest.valid) {
        fprintf(stderr, "[MAIN] Warning: initial sensor read failed. "
                        "Check wiring and device tree overlay.\n");
    }

    /* Start HTTP server */
    server_fd = webserver_start(HTTP_PORT);
    if (server_fd < 0) {
        fprintf(stderr, "[MAIN] Could not start web server. Exiting.\n");
        return EXIT_FAILURE;
    }

    printf("[MAIN] Open http://<BBB-IP>:%d in your browser.\n", HTTP_PORT);
    printf("[MAIN] Press Ctrl+C to stop.\n\n");

    /*
     * Main loop:
     *   1. Read sensor
     *   2. Print to terminal
     *   3. Handle one incoming HTTP request (blocks until browser connects)
     *
     * Note: because webserver_handle_request() blocks on accept(), the
     * sensor is only re-read when a client connects.  The HTML page
     * auto-refreshes every 5s, so in practice readings update at roughly
     * max(POLL_INTERVAL_S, browser_refresh_rate).
     *
     * For a non-blocking version, use select()/poll() or threads —
     * a good next step once you're comfortable with this code.
     */
    while (running) {
        /* --- Read sensor --- */
        latest = sensor_read_ldr(LDR_ADC_CHANNEL);

        /* --- Print to terminal --- */
        char timebuf[20];
        struct tm *tm_info = localtime(&latest.timestamp);
        strftime(timebuf, sizeof(timebuf), "%H:%M:%S", tm_info);

        if (latest.valid) {
            printf("[%s] Light: %3d%% | %s | raw=%d | %.3fV\n",
                   timebuf,
                   latest.percent,
                   sensor_label(latest.percent),
                   latest.raw,
                   latest.voltage);
        } else {
            printf("[%s] Sensor read FAILED\n", timebuf);
        }

        /* --- Serve one HTTP request --- */
        webserver_handle_request(server_fd, &latest);

        /* --- Wait before next poll --- */
        if (running) sleep(POLL_INTERVAL_S);
    }

    webserver_stop(server_fd);
    printf("[MAIN] Done.\n");
    return EXIT_SUCCESS;
}
