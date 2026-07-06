#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "webserver.h"
#include "sensor.h"

#define BUFFER_SIZE 1024

/* These headers only exist on Linux (BBB).
 * On Windows we compile stub functions so Eclipse can build cleanly. */
#ifdef _WIN32

int webserver_start(int port) {
    printf("[WEB] Stub: webserver not available on Windows (port %d)\n", port);
    return -1;
}

void webserver_handle_request(int server_fd, const SensorReading *reading) {
    (void)server_fd;
    (void)reading;
    printf("[WEB] Stub: no request handling on Windows\n");
}

void webserver_stop(int server_fd) {
    (void)server_fd;
}

#else
/* ---- Linux / BBB build ---- */
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUFFER_SIZE 1024

/* -----------------------------------------------------------------------
 * HTML template — served on every GET request.
 * The page auto-refreshes every 5 seconds via the <meta> tag.
 * ----------------------------------------------------------------------- */
static void build_html(char *buf, size_t buflen, const SensorReading *r) {
    char timebuf[32];
    struct tm *tm_info = localtime(&r->timestamp);
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", tm_info);

    /* Choose a colour band to visualise light level */
    const char *colour = "#888";
    if (r->valid) {
        if      (r->percent < 10) colour = "#1a1a2e"; /* dark navy  */
        else if (r->percent < 30) colour = "#4a4e69"; /* muted blue */
        else if (r->percent < 60) colour = "#f2a65a"; /* amber      */
        else if (r->percent < 85) colour = "#ffe066"; /* yellow     */
        else                      colour = "#fff9c4"; /* pale white */
    }

    snprintf(buf, buflen,
        "<!DOCTYPE html>"
        "<html><head>"
        "<meta charset='utf-8'>"
        "<meta http-equiv='refresh' content='5'>"
        "<title>BBB Light Monitor</title>"
        "<style>"
        "  body { font-family: monospace; background: #111; color: #eee;"
        "         display: flex; justify-content: center; align-items: center;"
        "         height: 100vh; margin: 0; }"
        "  .card { background: #1e1e1e; border-radius: 12px; padding: 40px 60px;"
        "          text-align: center; box-shadow: 0 4px 20px rgba(0,0,0,0.5); }"
        "  .title { font-size: 1.2em; color: #aaa; margin-bottom: 10px; }"
        "  .label { font-size: 2.5em; font-weight: bold; color: %s; }"
        "  .pct   { font-size: 4em; margin: 10px 0; }"
        "  .meta  { font-size: 0.85em; color: #666; margin-top: 20px; }"
        "  .err   { color: #ff5555; }"
        "</style>"
        "</head><body>"
        "<div class='card'>"
        "  <div class='title'>BeagleBone Black — LDR Light Sensor</div>",
        colour
    );

    if (r->valid) {
        char extra[512];
        snprintf(extra, sizeof(extra),
            "  <div class='label'>%s</div>"
            "  <div class='pct'>%d%%</div>"
            "  <div class='meta'>Raw ADC: %d &nbsp;|&nbsp; Voltage: %.3fV</div>"
            "  <div class='meta'>Last read: %s</div>"
            "  <div class='meta'><small>Page refreshes every 5 seconds</small></div>",
            sensor_label(r->percent),
            r->percent,
            r->raw,
            r->voltage,
            timebuf
        );
        strncat(buf, extra, buflen - strlen(buf) - 1);
    } else {
        strncat(buf,
            "<div class='label err'>Sensor Error</div>"
            "<div class='meta'>Could not read from ADC.<br>"
            "Check wiring and device tree overlay.</div>",
            buflen - strlen(buf) - 1
        );
    }

    strncat(buf, "</div></body></html>", buflen - strlen(buf) - 1);
}

/* ----------------------------------------------------------------------- */

int webserver_start(int port) {
    int server_fd;
    struct sockaddr_in addr;
    int opt = 1;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("[WEB] socket()");
        return -1;
    }

    /* Allow re-use of port immediately after restart */
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons((uint16_t)port);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("[WEB] bind()");
        close(server_fd);
        return -1;
    }

    if (listen(server_fd, 5) < 0) {
        perror("[WEB] listen()");
        close(server_fd);
        return -1;
    }

    printf("[WEB] Listening on http://100.64.0.4:%d\n", port);
    return server_fd;
}

void webserver_handle_request(int server_fd, const SensorReading *reading) {
    int client_fd;
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char req_buf[BUFFER_SIZE];
    char html[4096];
    char response[5120];

    client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
    if (client_fd < 0) {
        perror("[WEB] accept()");
        return;
    }

    /* Read HTTP request (we don't need to parse it — just drain it) */
    ssize_t bytes = read(client_fd, req_buf, sizeof(req_buf) - 1);
    if (bytes > 0) req_buf[bytes] = '\0';

    /* Build HTML body */
    build_html(html, sizeof(html), reading);

    /* Build HTTP response */
    snprintf(response, sizeof(response),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html; charset=utf-8\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s",
        strlen(html), html
    );

    write(client_fd, response, strlen(response));
    close(client_fd);
}

void webserver_stop(int server_fd) {
    close(server_fd);
    printf("[WEB] Server stopped.\n");
}
#endif /* _WIN32 */
