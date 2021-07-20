# arduino-ds18b20-exporter
Little sketch with prometheus exporter for arduino nano and multiple Dallas temperature sensors.

Parts:
 * 1x ds18b20 sensor
 * 1x arduino nano
 * 1x enc28j80 ethernet shield

## ⚠️ Warning ⚠️

Make sure that your Arduino is flashed with bootloader supporting AVR watchdog,
like [Optiboot](https://github.com/Optiboot/optiboot). Otherwise Arduino may
stuck in a boot loop.

Optiboot is already included in modern Arduino IDE versions. Connect external
programmer, select new bootloader and press "Burn Bootloader":

![How to burn bootloader from Arduino IDE](README.files/burn-bootloader.png)

## How to build

With default MAC and IP addresses (`90:a2:da:0d:10:5a`, `192.168.1.41`):

```bash
make fw
```

To build with custom MAC and IP addresses:

```bash
make MAC=12:34:56:78:90:ab IP=192.168.0.123 fw
```

## How to upload

```bash
avrdude -v -patmega328p -carduino -P/dev/$TTYDEV -b115200 -D -Uflash:w:fw/arduino-ds18b20-exporter-$MAC-$IP.hex:i
```

## How to test

```bash
curl -v http://$IP/
```

Debug log is on Arduino's USB Serial port (115200 baud).

Note that watchdog resets the device if there was no HTTP requests during
last 60 seconds.
