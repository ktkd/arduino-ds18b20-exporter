ARDUINO_HARDWARE_DIR = /usr/share/arduino/hardware
ARDUINO_FQBN = arduino:avr:nano:cpu=atmega328

EXTRA_ARDUINO_BUILDER_OPTS =

CUSTOM_EXTRA_CFLAGS =
EXTRA_CFLAGS = -Wno-cpp $(CUSTOM_EXTRA_CFLAGS)

.PHONY: all
all: build/main.ino.hex

build/main.ino.hex: main.ino
	[ -d "$(CURDIR)/build" ] \
			&& rm -r "$(CURDIR)/build" \
			|| true
	mkdir "$(CURDIR)/build"
	arduino-builder \
			-hardware "$(ARDUINO_HARDWARE_DIR)" \
			-tools "$(CURDIR)/packages" \
			-libraries "$(CURDIR)/libraries" \
			-build-path "$(CURDIR)/build" \
			-fqbn "$(ARDUINO_FQBN)" \
			-warnings "all" \
			-prefs "compiler.c.extra_flags=$(EXTRA_CFLAGS)" \
			-prefs "compiler.cpp.extra_flags=$(EXTRA_CFLAGS)" \
			-compile \
			-verbose \
			$(EXTRA_ARDUINO_BUILDER_OPTS) \
			main.ino

.PHONY: clean
clean:
	[ -e "$(CURDIR)/build" ] \
			&& rm -r "$(CURDIR)/build" \
			|| true
