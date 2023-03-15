BIN := handcar_test.elf
all:${BIN}
CFLAGS := -MMD -O0 -g3
LDLIBS := -ldl -lstdc++
LDFLAGS := -rdynamic

SRC := $(wildcard *.cc)
OBJ := $(patsubst %.cc,%.o,${SRC})
DEP := $(OBJ:.o=.d)
-include ${DEP}

${BIN}:${OBJ}
%.elf:
	gcc ${CFLAGS} -o $@ $^ ${LDFLAGS} ${LDLIBS}

run:${BIN}
	./${BIN}

clean:
	rm -f ${BIN} ${OBJ} ${DEP}

.PHONY: run clean
