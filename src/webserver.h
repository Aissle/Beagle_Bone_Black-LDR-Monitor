#ifndef WEBSERVER_H
#define WEBSERVER_H

/**
 * webserver.h - Minimal single-threaded HTTP server
 *
 * Listens on a TCP port and serves a single HTML page showing
 * the latest LDR sensor reading. Responds to GET / only.
 *
 * This is intentionally simple — no threading, no keep-alive,
 * one request at a time. Good enough for a local monitoring page.
 */

#include "sensor.h"

/**
 * webserver_start() - Bind and listen on the given port.
 * Returns the server socket fd on success, -1 on error.
 */
int webserver_start(int port);

/**
 * webserver_handle_request() - Accept one connection, serve response, close.
 * Pass a pointer to the latest SensorReading for live data.
 * Blocks until a client connects.
 */
void webserver_handle_request(int server_fd, const SensorReading *reading);

/**
 * webserver_stop() - Close the server socket.
 */
void webserver_stop(int server_fd);

#endif /* WEBSERVER_H */
