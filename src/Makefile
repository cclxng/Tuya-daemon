EXT_DIR := ../tuya-iot-core-sdk
INC_DIRS := $(EXT_DIR)/include $(EXT_DIR)/interface $(EXT_DIR)/utils $(EXT_DIR)/middleware ../include
CFLAGS := $(foreach DIR, $(INC_DIRS), -I$(DIR))
SRC := main.c tuya_connect.c system_info.c report.c args.c fork_daemon.c signals.c reporter.c

EXT_LIB_DIR := $(EXT_DIR)/build/lib
LDLIBS := -llink_core -lmiddleware_implementation -lplatform_port -lutils_modules
LDFLAGS := -g -L$(EXT_LIB_DIR) -Wl,-rpath='$$ORIGIN/$(EXT_LIB_DIR)'

tuyad: $(SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) $(SRC) $(LDLIBS) -o tuyad
