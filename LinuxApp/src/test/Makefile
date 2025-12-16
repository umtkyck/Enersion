# Enersion Test Application Makefile
# Target: MYIR STM32MP257

CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -O2 -I../hal -I../protocol
LDFLAGS = 

# Source files
HAL_SRCS = ../hal/rs485_serial.c ../hal/rs485_gpio.c
PROTO_SRCS = ../protocol/enersion_crc.c ../protocol/enersion_protocol.c
APP_SRCS = enersion_test.c

SRCS = $(APP_SRCS) $(HAL_SRCS) $(PROTO_SRCS)

# Output
TARGET = enersion_test

.PHONY: all clean install run

all: $(TARGET)

$(TARGET): $(SRCS)
	@echo "Compiling Enersion Test Application..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo ""
	@echo "=========================================="
	@echo " Build successful: $(TARGET)"
	@echo "=========================================="
	@echo ""
	@echo "Run with: ./$(TARGET)"
	@echo ""

clean:
	rm -f $(TARGET)

install: $(TARGET)
	cp $(TARGET) /usr/local/bin/
	@echo "Installed to /usr/local/bin/$(TARGET)"

run: $(TARGET)
	./$(TARGET)



