ASM_VERSION=0.2.10
CC=gcc
MD=mkdir

BUILD_DIR=build
TARGET_EXEC=$(BUILD_DIR)/asm51
TEST_SH=./test-build.sh

SRC_DIR=src
SRCS=$(wildcard $(SRC_DIR)/*.c)

ASM_VERSION:='"$(ASM_VERSION) $(shell uname -ms)"'

all:
	$(MD) -p $(BUILD_DIR)
	$(CC) $(SRCS) -o $(TARGET_EXEC) -DVERSION=$(ASM_VERSION)

clean:
	rm $(TARGET_EXEC)
	
check:
	$(TEST_SH) 
