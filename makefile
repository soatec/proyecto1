SOURCE_DIR  := ./src
INCLUDE_DIR := ./include
BUILD_DIR   := ./build

CC        := gcc
CFLAGS    := -Wall
LDFLAGS   := -lm -lpthread -lrt
INC_FLAGS := -I$(INCLUDE_DIR)

.PHONY: all
all: $(BUILD_DIR) $(BUILD_DIR)/creator $(BUILD_DIR)/producer $(BUILD_DIR)/consumer $(BUILD_DIR)/finalizer

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INC_FLAGS)

$(BUILD_DIR)/creator: $(BUILD_DIR)/creator.o $(BUILD_DIR)/creator_main.o $(BUILD_DIR)/buffer.o $(BUILD_DIR)/utils.o
	$(CC) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/producer: $(BUILD_DIR)/producer.o $(BUILD_DIR)/producer_main.o $(BUILD_DIR)/buffer.o $(BUILD_DIR)/utils.o
	$(CC) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/consumer: $(BUILD_DIR)/consumer.o $(BUILD_DIR)/consumer_main.o $(BUILD_DIR)/buffer.o $(BUILD_DIR)/utils.o
	$(CC) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/finalizer: $(BUILD_DIR)/finalizer.o $(BUILD_DIR)/finalizer_main.o $(BUILD_DIR)/buffer.o $(BUILD_DIR)/utils.o
	$(CC) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR):
	mkdir -p $@


.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
