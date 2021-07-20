# arduino-ds18b20-exporter
Little sketch with prometheus exporter for arduino nano and multiple Dallas temperature sensors.

Parts:
 * 1x ds18b20 sensor
 * 1x arduino nano
 * 1x enc28j80 ethernet shield

## How to build

```bash
make fw
```

To build with custom MAC and IP addresses:

```bash
make MAC=12:34:56:78:90:ab IP=192.168.0.123 fw
```
