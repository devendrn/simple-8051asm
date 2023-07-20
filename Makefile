ASM_VERSION=0.2.7
CC=gcc

BUILD_DIR=build
TARGET_EXEC=$(BUILD_DIR)/asm51

SRC_DIR=src
SRCS=$(wildcard $(SRC_DIR)/*.c)

all:
	$(CC) $(SRCS) -o $(TARGET_EXEC) -DVERSION=\"$(ASM_VERSION)\"

clean:
	rm $(TARGET_EXEC)
