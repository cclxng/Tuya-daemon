EXT_DIR := ./tuya-iot-core-sdk

.PHONY: all src clean ext

all: src
	ln -sf src/tuyad ./tuyad

src: ext
	$(MAKE) -C src

ext:
	cmake -S $(EXT_DIR) -B $(EXT_DIR)/build
	cmake --build $(EXT_DIR)/build

clean:
	$(MAKE) -C src clean
	$(RM) -rf $(EXT_DIR)/build ./tuyad
