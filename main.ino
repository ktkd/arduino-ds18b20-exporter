#include <SPI.h>
#include <UIPEthernet.h>
#include <DS18B20.h>

/*
 *  Configuration:
 */

// Comment this line to disable debug logging.
#define ENABLE_DEBUG_LOGGING

// Digital pin for sensors.
const uint8_t ds_pin = 3;

// Ethernet Shield MAC address, use value on back of shield.
const uint8_t mac[] = {0x90, 0xa2, 0xda, 0x0d, 0x10, 0x5a};
// IP address, will depend on your local network.
const IPAddress ip(192, 168, 1, 41);
// TCP port for HTTP server.
const uint16_t port = 80;

/*
 *  Global objects:
 */

// Initialize the sensors library.
DS18B20 ds(ds_pin);

// Initialize the Ethernet server library.
EthernetServer server(port);

// String for reading from client
String request = String(100);

/*
 *  Logging macros:
 */
#ifdef ENABLE_DEBUG_LOGGING
#	define debug(...) Serial.print(__VA_ARGS__)
#	define debugln(...) Serial.println(__VA_ARGS__)
#else
#	define debug(...)
#	define debugln(...)
#endif

#define info(...) Serial.print(__VA_ARGS__)
#define infoln(...) Serial.println(__VA_ARGS__)

/*
 *  Initialization.
 */
void setup()
{
	// Open Serial communications and wait for port to open.
	Serial.begin(115200);

	// Start the Ethernet connection and the server.
	Ethernet.begin(mac, ip);
	server.begin();

	info("server_started ip=");
	info(ip);
	info(" port=");
	info(port);

	// Start the Temperature sensors
	info(" num_sensors=");
	infoln(ds.getNumberOfDevices());

	infoln();
}

/*
 *  Main loop.
 */
void loop()
{
	// Listen for incoming clients.
	EthernetClient client = server.available();

	if (client) {
		info("client_connected remote_ip=");
		infoln(client.remoteIP());

		// An http request ends with a blank line.
		boolean current_line_is_blank = true;

		while (client.connected()) {
			if (client.available()) {
				char c = client.read();

				// Read http request.
				if (request.length() < 100) {
					request += c;
				}

				if (c == '\n' && current_line_is_blank) {
					debug("got_request request='");
					debug(request);
					debugln('\'');

					infoln("reading_sensor");

					// Send command to all the sensors for temperature conversion
					send_prometheus_response(client);

					break;
				}

				if (c == '\n') {
					// Character is a new line.
					current_line_is_blank = true;
				} else if (c != '\r') {
					// Character is not a new line or a carriage return.
					current_line_is_blank = false;
				}
			}
		}

		// Give the web browser time to receive the data.
		delay(1);

		// Close the connection:
		client.stop();

		infoln("client_disconnected");
		infoln();
	}

	// Reset the request.
	request = "";
}

/*
 *  Format and send HTTP response for Prometheus.
 */
void send_prometheus_response(EthernetClient &client)
{
	infoln("sending_response");

	// Send a standard http response header.
	client.println("HTTP/1.1 200 OK");

	// Content-Type from https://github.com/siimon/prom-client/blob/master/lib/registry.js
	// 'text/plain; version=0.0.4; charset=utf-8'
	client.println("Content-Type: text/plain; version=0.0.4; charset=utf-8");
	client.println("Connnection: close");
	client.println();

	// Send Prometheus\serialport body.
	// we ask all sensors
	while (ds.selectNext()) {
		uint8_t address[8];
		ds.getAddress(address);
		client.print("sensor{addr=\"");
		infoln("sensor address=");
		for (uint8_t i = 0; i < 8; i++) {
			if (i) {
				info('.');
			}
			client.print(address[i]);
			info(address[i]);
		}
		infoln();
		client.print("\",res=\"");
		info("sensor resolution=");
		infoln(ds.getResolution());
		client.print(ds.getResolution());
		client.print("\",pwr=\"");
		info("sensor pwr=");
		if (ds.getPowerMode()) {
			client.print("external");
			infoln("external");
		} else {
			client.print("parasite");
			infoln("parasite");
		}
		client.print("\"} ");
		info("sensor temperature=");
		infoln(ds.getTempC());
		client.print(ds.getTempC());
		client.print("\n");
	}
}
