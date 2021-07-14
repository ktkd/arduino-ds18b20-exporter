#include <SPI.h>
#include <UIPEthernet.h>
#include <DS18B20.h>

/*
 *  Configuration:
 */

// Comment this line to disable debug logging.
#define ENABLE_DEBUG_LOGGING

// Maximum number of sensors.
#define MAX_SENSORS 16

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

// Total number of discovered sensors.
uint8_t num_sensors;
// Information about discovered sensors.
struct {
	uint8_t address[8];
	uint8_t resolution;
	bool external_power;
} sensor_info[MAX_SENSORS];

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
	infoln(port);

	// Find all temperature sensors.
	for (num_sensors = 0; ds.selectNext(); num_sensors++) {
		if (num_sensors >= MAX_SENSORS) {
			infoln("error_halt too_many_sensors");
			for (;;);
		};

		info("found_sensor n=");
		info(num_sensors);

		ds.getAddress(sensor_info[num_sensors].address);
		info(" addr=");
		for (uint8_t i = 0; i < 8; i++) {
			if (i) {
				info('.');
			}
			info(sensor_info[num_sensors].address[i]);
		}

		info(" res=");
		sensor_info[num_sensors].resolution = ds.getResolution();
		info(sensor_info[num_sensors].resolution);

		sensor_info[num_sensors].external_power = ds.getPowerMode();
		if (sensor_info[num_sensors].external_power) {
			infoln(" pwr=external");
		} else {
			infoln(" pwr=parasite");
		}
	}

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

		// Number of consecutive newlines.
		uint8_t num_newlines = 0;

		debugln("request_begin");
		while (client.connected()) {
			char c = client.read();

			if (c >= 0) {
				// Skip all data until the end of HTTP request.
				switch (c) {
				case '\r':
					// Ignore \r.
					continue;

				case '\n':
					num_newlines++;
					break;

				default:
					// Reset counter.
					num_newlines = 0;
					break;
				}

				if (num_newlines == 2) {
					debugln("request_end");
					// Send answer.
					send_prometheus_response(client);
					break;
				}
				debug(c);
			}
		}

		infoln("closing_connection");
		client.stop();

		infoln();
	}
}

/*
 *  Format and send HTTP response for Prometheus.
 */
void send_prometheus_response(EthernetClient &client)
{
	infoln("sending_response");

	// Send a standard http response header.
	client.print("HTTP/1.1 200 OK\r\n");

	// Content-Type from https://github.com/siimon/prom-client/blob/master/lib/registry.js
	// 'text/plain; version=0.0.4; charset=utf-8'
	client.print("Content-Type: text/plain; version=0.0.4; charset=utf-8\r\n");
	client.print("Connnection: close\r\n\r\n");

	// Send Prometheus body.
	// We ask all sensors.
	for (uint8_t i = 0; i < num_sensors; i++) {
		debug("querying_sensor n=");
		debug(i);

		client.print("sensor{addr=\"");
		for (uint8_t j = 0; j < 8; j++) {
			if (j) {
				client.print('.');
			}
			client.print(sensor_info[i].address[j]);
		}

		// Select sensor by address.
		ds.select(sensor_info[i].address);

		client.print("\",res=\"");
		client.print(sensor_info[i].resolution);

		client.print("\",pwr=\"");
		if (sensor_info[i].external_power) {
			client.print("external");
		} else {
			client.print("parasite");
		}
		client.print("\"} ");

		const float temperature = ds.getTempC();
		client.print(temperature);
		client.print("\r\n");
		debug(" temperature=");
		debugln(temperature);
	}

	client.flush();
}
