.PHONY: clean upload

# Board type (arduino-cli board listall will show all boards)
BOARD = esp32:esp32:lolin32

# Serial port (usually an USB port, use arduino-cli board list)
SERIAL = /dev/ttyUSB0

# Build dir is usually `pwd`/build/board/*.ino.bin with the board
# name colons replaced by dots.
build_dir = $(subst :,.,$(BOARD))
bin = $(PWD)/build/$(build_dir)/*.ino.bin
src = $(wildcard *.ino *.h)

# Default target
${bin}: Makefile ${src}
	arduino-cli compile --warnings all --fqbn ${BOARD} $(PWD)

# Upload to board
upload: ${bin}
	arduino-cli upload -p ${SERIAL} --fqbn ${BOARD} $(PWD)

# Cleanup
clean:
	rm -rf build
