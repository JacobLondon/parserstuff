CC = gcc

TARGET_T = tester

TARGET_O = \
./arg.o \
./tok.o \
./main.o

CFLAGS = -Wall -Wextra

.PHONY: clean

$(TARGET_T): $(TARGET_O)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -rf $(TARGET_O) $(TARGET_T)
