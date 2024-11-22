TARGET = metrics

CC = gcc

SRC_DIR = src
INCLUDE_DIR = include
PROMETHEUS_REPO = https://github.com/digitalocean/prometheus-client-c
PROMETHEUS_DIR = prometheus-client-c
BUILD_DIR_PROM = $(PROMETHEUS_DIR)/prom/build
BUILD_DIR_PROMHTTP = $(PROMETHEUS_DIR)/promhttp/build
PROMETHEUS_LIB_DIR = /usr/local/lib
MICROHTTPD_INCLUDE_DIR = /usr/include

SRCS = $(SRC_DIR)/main.c $(SRC_DIR)/expose_metrics.c $(SRC_DIR)/metrics.c

CFLAGS = -I$(PROMETHEUS_DIR) -I$(MICROHTTPD_INCLUDE_DIR) -I$(INCLUDE_DIR) -I/usr/include/cjson
LDFLAGS = -L$(PROMETHEUS_LIB_DIR) -lprom -pthread -lpromhttp -lcjson

export LD_LIBRARY_PATH := $(PROMETHEUS_LIB_DIR):$(LD_LIBRARY_PATH)

check_dependencies:
	sudo apt-get update
	sudo apt-get install -y libmicrohttpd-dev libcjson-dev

clone_prometheus: check_dependencies
	git clone $(PROMETHEUS_REPO) $(PROMETHEUS_DIR)

modify_source:
	sed -i '32s/int/enum MHD_Result/' $(PROMETHEUS_DIR)/promhttp/src/promhttp.c
	sed -i '64s/MHD_AccessHandlerCallback/((MHD_AccessHandlerCallback)promhttp/' $(PROMETHEUS_DIR)/promhttp/src/promhttp.c
	sed -i '64s/(MHD_AccessHandlerCallback)/(MHD_AccessHandlerCallback)promhttp/' $(PROMETHEUS_DIR)/promhttp/src/promhttp.c

build_prometheus: clone_prometheus modify_source
	mkdir -p $(BUILD_DIR_PROM)
	cd $(BUILD_DIR_PROM) && cmake ../ && make && sudo make install
	mkdir -p $(BUILD_DIR_PROMHTTP)
	cd $(BUILD_DIR_PROMHTTP) && cmake ../ && make && sudo make install

all: build_prometheus $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(SRCS) -o $(TARGET) $(CFLAGS) $(LDFLAGS)

clean:
	rm -f $(TARGET)
	rm -rf $(PROMETHEUS_DIR)