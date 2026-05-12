CC = gcc
CFLAGS = -g -Wall

TARGET = gthr_demo
OBJS = gthr.o gthr_switch.o gthr_plat_linux.o main.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<


kernel/scheduler/gthr_switch.o: kernel/scheduler/gthr_switch.S
	nasm -f elf32 -o $@ $<

.PHONY: run clean
run: $(TARGET)
	./$(TARGET) LS

clean:
	rm -f *.o $(TARGET)