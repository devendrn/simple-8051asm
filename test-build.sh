#!/bin/bash

BUILD=./build/asm51

if [ ! -f "$BUILD" ]; then
	echo "ERROR: $BUILD not found"
	exit 1
fi

# test files will be in name.asm, name.hex pairs
TEST_DIR=./test/
HEX_DIR=${TEST_DIR}hex/
TEST_FILES=$TEST_DIR*.asm

echo "TEST DIR: $TEST_DIR"

for TEST_ASM in $TEST_FILES; do
	if [ -f "$TEST_ASM" ]; then

    TEST_FILE_NAME=${TEST_ASM##*/}

		#TEST_HEX_ORG=${TEST_ASM%.asm}.hex
		TEST_HEX_ORG=${HEX_DIR}${TEST_FILE_NAME%.asm}.hex
		TEST_HEX_GEN=${TEST_ASM%.asm}-temp.hex

		# line
		printf '%*s\n' "${COLUMNS:-$(tput cols)}" '' | tr ' ' -

		echo "CHECKING: ${TEST_ASM/$TEST_DIR}"

		# assemble into hex
		OUT=$($BUILD $TEST_ASM -o $TEST_HEX_GEN)

		if [[ -z "$OUT" ]]; then
			echo "ASSEMBLE: done"
		else
			echo "ASSEMBLE: failed"
			echo "$OUT"
			continue
		fi
	
		# compare correct hex file and generated hex file
		DIFF=$(diff $TEST_HEX_GEN $TEST_HEX_ORG)

		if [[ -z "$DIFF" ]]; then
			echo "VERIFY  : done"
		else
			echo "VERIFY  : failed"
			echo "$DIFF"
		fi

	fi
done

rm $TEST_DIR*.hex
