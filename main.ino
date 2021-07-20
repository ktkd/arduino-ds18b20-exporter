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
static const uint8_t ds_pin = 3;

// Ethernet Shield MAC address, use value on back of shield.
static const uint8_t mac[] = {MAC};
// IP address, will depend on your local network.
static const IPAddress ip(IP);
// TCP port for HTTP server.
static const uint16_t port = 80;

// How long we should wait for request from client.
static const unsigned long request_timeout = 2000;  /* milliseconds */

/*
 *  Global objects:
 */

// Initialize the sensors library.
static DS18B20 ds(ds_pin);

// Initialize the Ethernet server library.
static EthernetServer server(port);

// Total number of discovered sensors.
static uint8_t num_sensors;
// Information about discovered sensors.
static struct {
	uint8_t address[8];
	uint8_t resolution;
	uint8_t power_mode;
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
static void setup()
{
	// Open Serial communications and wait for port to open.
	Serial.begin(115200);
	infoln();

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

		sensor_info[num_sensors].power_mode = ds.getPowerMode();
		if (sensor_info[num_sensors].power_mode) {
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
static void loop()
{
	// Listen for incoming clients.
	EthernetClient client = server.accept();

	if (client) {
		info("client_connected remote_ip=");
		infoln(client.remoteIP());

		// Number of bytes read.
		uint8_t num_bytes = 0;
		// Number of consecutive newlines.
		uint8_t num_newlines = 0;
		// Time of connection start.
		const unsigned long start = millis();

		debugln("request_begin");
		while (client.connected()) {
			Ethernet.maintain();
			char c = client.read();
			#ifdef ENABLE_DEBUG_LOGGING
			char last_printed = '\n';
			#endif

			if (c >= 0) {
				num_bytes++;
				if (!num_bytes) {
					// uint8_t overflow => more than 255 bytes.
					#ifdef ENABLE_DEBUG_LOGGING
					if (last_printed != '\n') {
						debugln();
					}
					#endif
					infoln("error request_too_long");
					break;
				}

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

				#ifdef ENABLE_DEBUG_LOGGING
				if (((c < ' ') || (c > '~')) && (c != '\n')) {
					// Replace non-printable characters with dots.
					c = '.';
				}
				debug(c);
				last_printed = c;
				#endif
			}

			if (millis() - start > request_timeout) {
				#ifdef ENABLE_DEBUG_LOGGING
				if (last_printed != '\n') {
					debugln();
				}
				#endif
				infoln("error client_timeout");
				break;
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
static void send_prometheus_response(EthernetClient &client)
{
	infoln("sending_response");

	// Send a standard http response header.
	client.print("HTTP/1.1 200 OK\r\n");
	client.print("Content-Type: text/plain; charset=utf-8\r\n");
	client.print("Connnection: close\r\n\r\n");

	// Send Prometheus body.
	const unsigned long uptime = millis();
	client.print("sensor_exporter_uptime{mac=\"" MAC_STR "\"} ");
	client.print(uptime);
	client.print("\r\n");
	debug("uptime milliseconds=");
	debugln(uptime);

	// We ask all sensors.
	for (uint8_t i = 0; i < num_sensors; i++) {
		debug("querying_sensor n=");
		debug(i);

		client.print("sensor{mac=\"" MAC_STR "\",addr=\"");
		for (uint8_t j = 0; j < 8; j++) {
			if (j) {
				client.print('.');
			}
			client.print(sensor_info[i].address[j]);
		}

		client.print("\",res=\"");
		client.print(sensor_info[i].resolution);

		client.print("\",pwr=\"");
		if (sensor_info[i].power_mode) {
			client.print("external");
		} else {
			client.print("parasite");
		}
		client.print("\"} ");

		const float temperature = ds.getTempC(
				sensor_info[i].address,
				sensor_info[i].resolution,
				sensor_info[i].power_mode);
		client.print(temperature);
		client.print("\r\n");
		debug(" temperature=");
		debugln(temperature);
	}

	client.flush();
}
