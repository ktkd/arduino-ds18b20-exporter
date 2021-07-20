MAC = 90:a2:da:0d:10:5a
IP = 192.168.1.41

ARDUINO_TOOLCHAIN_VER = 1.8.15
ARDUINO_TOOLCHAIN_NAME = arduino-$(ARDUINO_TOOLCHAIN_VER)
ARDUINO_TOOLCHAIN_URL = https://downloads.arduino.cc/$(ARDUINO_TOOLCHAIN_NAME)-linux64.tar.xz

ARDUINO_FQBN = arduino:avr:nano:cpu=atmega328

EXTRA_ARDUINO_BUILDER_OPTS =

CUSTOM_EXTRA_CFLAGS =
EXTRA_CFLAGS = -Wno-cpp -DUIP_CONF_UDP=0 $(CUSTOM_EXTRA_CFLAGS)

.PHONY: help
help:
	@echo "make fw               # Build firmware"
	@echo "make help             # Show this help message"
	@echo "make clean            # Remove compiled firmware HEX files"
	@echo "make clean-toolchain  # Remove downloaded toolchain"
	@echo "make clean-all        # Remove everything except of source code"

mac_fname = $(subst :,.,$(MAC))
fw_fname = arduino-ds18b20-exporter-$(mac_fname)-$(IP).hex

.PHONY: fw
fw: fw/$(fw_fname)
	@echo
	@echo "Here is your firmware:"
	@ls -lh "$(<)"

arduino_toolchain_tarball = $(notdir $(ARDUINO_TOOLCHAIN_URL))

# Download toolchain in a tarball.
toolchain/$(arduino_toolchain_tarball):
	mkdir -p "$(CURDIR)/toolchain"
	rm -f "$(CURDIR)/toolchain/.incomplete-dl"
	wget -O "$(CURDIR)/toolchain/.incomplete-dl" "$(ARDUINO_TOOLCHAIN_URL)"
	mv "$(CURDIR)/toolchain/.incomplete-dl" "$(@)"

# Untar toolchain.
toolchain/$(ARDUINO_TOOLCHAIN_NAME): toolchain/$(arduino_toolchain_tarball)
	rm -rf "$(CURDIR)/toolchain/.incomplete"
	mkdir -p "$(CURDIR)/toolchain/.incomplete"
	tar -C "$(CURDIR)/toolchain/.incomplete" -x -f "$(<)"
	touch "$(CURDIR)/toolchain/.incomplete/$(ARDUINO_TOOLCHAIN_NAME)"
	mv "$(CURDIR)/toolchain/.incomplete/$(ARDUINO_TOOLCHAIN_NAME)" "$(@)"

comma = ,
mac_c = 0x$(subst :,$(comma)0x,$(MAC))
ip_c = $(subst .,$(comma),$(IP))

# Build firmware. Cleans and reuilds each time because MAC/IP may change.
.PHONY: build/main.ino.hex
build/main.ino.hex: main.ino toolchain/$(ARDUINO_TOOLCHAIN_NAME)
	rm -rf "$(CURDIR)/build"
	mkdir -p "$(CURDIR)/build"
	toolchain/$(ARDUINO_TOOLCHAIN_NAME)/arduino-builder \
			-hardware "toolchain/$(ARDUINO_TOOLCHAIN_NAME)/hardware" \
			-tools "toolchain/$(ARDUINO_TOOLCHAIN_NAME)/hardware" \
			-libraries "toolchain/$(ARDUINO_TOOLCHAIN_NAME)/libraries" \
			-libraries "$(CURDIR)/libraries" \
			-build-path "$(CURDIR)/build" \
			-fqbn "$(ARDUINO_FQBN)" \
			-warnings "all" \
			-prefs "runtime.tools.ctags.path=$$(echo toolchain/$(ARDUINO_TOOLCHAIN_NAME)/tools-builder/ctags/*)" \
			-prefs "compiler.c.extra_flags=$(EXTRA_CFLAGS) -DMAC_STR=\"$(MAC)\" -DMAC=$(mac_c) -DIP=$(ip_c)" \
			-prefs "compiler.cpp.extra_flags=$(EXTRA_CFLAGS) -DMAC_STR=\"$(MAC)\" -DMAC=$(mac_c) -DIP=$(ip_c)" \
			-compile \
			-verbose \
			$(EXTRA_ARDUINO_BUILDER_OPTS) \
			"$(<)"

fw/$(fw_fname): build/main.ino.hex main.ino
	mkdir -p "$(CURDIR)/fw"
	cp "$(<)" "$(@)"

.PHONY: clean
clean:
	rm -rf "$(CURDIR)/fw"

.PHONY: clean-toolchain
clean-toolchain:
	rm -rf "$(CURDIR)/toolchain"

.PHONY: clean-build
clean-build:
	rm -rf "$(CURDIR)/build"

.PHONY: clean-all
clean-all: clean clean-toolchain clean-build
