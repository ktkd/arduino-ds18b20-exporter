# arduino-ds18b20-exporter
Little sketch with prometheus exporter for arduino nano and multiple Dallas temperature sensors.

Parts:
 * 1x ds18b20 sensor
 * 1x arduino nano
 * 1x enc28j80 ethernet shield

# how it work's
Just curl http://192.168.1.41 :
```
sensor_exporter_uptime{mac="90:a2:da:0d:10:5a"} 1908564
sensor{mac="90:a2:da:0d:10:5a",addr="40.28.168.7.0.0.128.63",res="12",pwr="parasite"} 21.94
sensor{mac="90:a2:da:0d:10:5a",addr="40.129.18.124.6.0.0.87",res="9",pwr="external"} 24.00
sensor{mac="90:a2:da:0d:10:5a",addr="40.203.37.123.6.0.0.190",res="9",pwr="external"} 36.00
sensor{mac="90:a2:da:0d:10:5a",addr="40.255.47.100.102.20.2.94",res="12",pwr="external"} 8.75
```

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
