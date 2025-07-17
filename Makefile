CC = gcc
CFLAGS = -Wall -Wextra -O2 -fPIC -shared
LDFLAGS = -lpthread
TARGET = libgpio_rumble.so
SOURCE = libgpio_rumble.c

# Default target
all: $(TARGET)

# Build the shared library
$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# Clean build artifacts
clean:
	rm -f $(TARGET)

# Install to system (optional)
install: $(TARGET)
	sudo cp $(TARGET) /usr/local/lib/
	sudo ldconfig

# Uninstall from system
uninstall:
	sudo rm -f /usr/local/lib/$(TARGET)
	sudo ldconfig

# Test build (compile only, no link)
test:
	$(CC) -c $(SOURCE) -fPIC -Wall -Wextra

# Debug build with symbols
debug: CFLAGS += -g -DDEBUG
debug: $(TARGET)

# Release build (optimized, stripped)
release: CFLAGS += -O3 -s
release: $(TARGET)

# Show help
help:
	@echo "Available targets:"
	@echo "  all      - Build rumble.so (default)"
	@echo "  clean    - Remove build artifacts"
	@echo "  install  - Install to /usr/local/lib"
	@echo "  uninstall- Remove from /usr/local/lib"
	@echo "  test     - Test compilation"
	@echo "  debug    - Build with debug symbols"
	@echo "  release  - Optimized release build"
	@echo "  help     - Show this help"
	@echo ""
	@echo "Usage:"
	@echo "  make"
	@echo "  LD_PRELOAD=./libgpio_rumble.so your_game"

.PHONY: all clean install uninstall test debug release help
