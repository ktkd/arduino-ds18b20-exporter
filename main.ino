#include <SPI.h>
#include <UIPEthernet.h>
#include <DS18B20.h>

/*
 *  Configuration:
 */

// Digital pin for sensors.
const uint8_t ds_pin = 3;

// Ethernet Shield MAC address, use value on back of shield.
const uint8_t mac[] = {0x90, 0xa2, 0xda, 0x0d, 0x10, 0x5a};
// IP address, will depend on your local network.
const IPAddress ip(192, 168, 12, 41);
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
String parsed_request = "";

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
	Serial.print("Server is at ");
	Serial.println(Ethernet.localIP());
	// Start the Temperature sensors
	Serial.print("Devices: ");
	Serial.println(ds.getNumberOfDevices());
}

/*
 *  Main loop.
 */
void loop()
{
	// Listen for incoming clients.
	EthernetClient client = server.available();

	if (client) {
		Serial.println("Client connected");

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
					Serial.println("Finished reading request.");
					Serial.println("http request: '" + request + "'");

					Serial.println("Reading sensor.");

					String error = "";
					// Send command to all the sensors for temperature conversion
					send_prometheus_response(client, error);

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

		Serial.println("Client disonnected");
	}

	// Reset the request.
	request = "";
	parsed_request = "";
}

/*
 *  Format and send HTTP response for Prometheus.
 */
void send_prometheus_response(EthernetClient client, String error)
{
	if(error == "")
	{
		Serial.println("Sending Prometheus response.");

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
			Serial.print("Address:");
			for (uint8_t i = 0; i < 8; i++) {
				Serial.print(" ");
				client.print(address[i]);
				Serial.print(address[i]);
			}
			Serial.println();
			client.print("\",res=\"");
			Serial.print("Resolution: ");
			Serial.println(ds.getResolution());
			client.print(ds.getResolution());
			client.print("\",pwr=\"");
			Serial.print("Power Mode: ");
			if (ds.getPowerMode()) {
				client.print("external");
				Serial.println("external");
			} else {
				client.print("parasite");
				Serial.println("parasite");
			}
			client.print("\"} ");
			Serial.print("Temperature: ");
			Serial.print(ds.getTempC());
			client.print(ds.getTempC());
			client.print("\n");
		}
	} else {
		Serial.println("Sending Prometheus error response.");

		// Send a standard http response header.
		client.println("HTTP/1.1 500 Internal Server Error");
		client.println("Connnection: close");
		client.println();
	}
}
