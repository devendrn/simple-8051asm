#!/bin/bash

BUILD=build/asm51

if [ ! -f "$BUILD" ]; then
	echo "ERROR: $BUILD not found"
	exit 1
fi

# test files will be in name.asm, name.hex pairs
TEST_DIR=test
EXAMPLE_DIR=examples
HEX_DIR=$TEST_DIR/hex
TEST_FILES="$TEST_DIR/*.asm $EXAMPLE_DIR/*.asm"

echo "TEST DIRS: $TEST_DIR/ $EXAMPLE_DIR/"

FAILED=false

for TEST_ASM in $TEST_FILES; do
	if [ -f "$TEST_ASM" ]; then

    TEST_FILE_NAME=${TEST_ASM##*/}
    TEST_FILE_NAME=${TEST_FILE_NAME%.asm}

		TEST_HEX_ORG=$HEX_DIR/$TEST_FILE_NAME.hex
		TEST_HEX_GEN=$TEST_DIR/$TEST_FILE_NAME-temp.hex

		# line
		printf '%*s\n' "${COLUMNS}" '' | tr ' ' -

		echo "CHECKING: $TEST_ASM"

		# assemble into hex
		OUT=$($BUILD $TEST_ASM -o $TEST_HEX_GEN)

		if [[ -z "$OUT" ]]; then
			echo "ASSEMBLE: done"
		else
      FAILED=true
			echo "ASSEMBLE: failed"
			echo "$OUT"
			continue
		fi
	
		# compare correct hex file and generated hex file
		DIFF=$(diff $TEST_HEX_GEN $TEST_HEX_ORG)

		if [[ -z "$DIFF" ]]; then
			echo "VERIFY  : done"
		else
      FAILED=true
			echo "VERIFY  : failed"
			echo "$DIFF"
		fi

	fi
done

rm $TEST_DIR/*.hex

if $FAILED; then
	exit 1
fi
