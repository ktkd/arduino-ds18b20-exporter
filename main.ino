#include <SPI.h>
#include <UIPEthernet.h>
#include <DS18B20.h>

// Use Digital pin3 for sensor
DS18B20 ds(3);


// Ethernet Shield MAC address, use value on back of shield.
byte mac[] = { 0x90, 0xA2, 0xDA, 0x0D, 0x10, 0x5A };

// IP address, will depend on your local network.
IPAddress ip(192,168,12,41);

// Initialize the Ethernet server library, use port 80 for HTTP.
EthernetServer server(80);

// String for reading from client
String request = String(100);
String parsedRequest = "";

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


void loop()
{
  // Listen for incoming clients.
  EthernetClient client = server.available();

  if (client)
  {
    Serial.println("Client connected");

    // An http request ends with a blank line.
    boolean currentLineIsBlank = true;

    while (client.connected())
    {
      if (client.available())
      {
        char c = client.read();

        // Read http request.
        if (request.length() < 100)
        {
          request += c;
        }
        if (c == '\n' && currentLineIsBlank)
        {
          Serial.println("Finished reading request.");
          Serial.println("http request: '" + request + "'");

          Serial.println("Reading sensor.");

          String error = "";
          // Send command to all the sensors for temperature conversion
          sendPrometheusResponse(client, error);

          break;
        }

        if (c == '\n')
        {
          // Character is a new line.
          currentLineIsBlank = true;
        }
        else if (c != '\r')
        {
          // Character is not a new line or a carriage return.
          currentLineIsBlank = false;
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
  parsedRequest = "";
}


/*
 *
 */
void sendPrometheusResponse(EthernetClient client, String error)
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


  }
  else
  {
    Serial.println("Sending Prometheus error response.");

    // Send a standard http response header.
    client.println("HTTP/1.1 500 Internal Server Error");
    client.println("Connnection: close");
    client.println();
  }
}
