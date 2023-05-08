CC = gcc

TARGET_EXEC = asm51

BUILD_DIR = build
SRC_DIR = src

SRCS = $(wildcard $(SRC_DIR)/*.c)

all:
	$(CC) $(SRCS) -o $(BUILD_DIR)/$(TARGET_EXEC)

clean:
	rm $(BUILD_DIR)/$(TARGET_EXEC)
