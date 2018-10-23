# program executable name
TARGET = exe.out

# compiler flags
CFLAGS = -g -std=gnu99 -D_GNU_SOURCE -Wall

# linker flags
LDFLAGS = -g -lpthread -lrt

# list of sources
SOURCES = $(shell find -name "*.c")

# dependencies
DEPS = time.h udp.h

BUILDROOTDIR := /home/student/Desktop/buildroot
TARGETDIR := /export/nfs
CC := $(BUILDROOTDIR)/output/staging/usr/bin/avr32-linux-gcc

# default rule, to compile everything
all: $(TARGET) copy

# define object files
OBJECTS = $(SOURCES:.c=.o)

# link programs
$(TARGET): $(OBJECTS)
	$(CC) $^ -o $@ $(LDFLAGS)
	
# compile
%.o : %.c $(DEPS)
	$(CC)  $(CFLAGS) -c -o $@ $<

copy:
	sudo cp exe.out $(TARGETDIR)

# cleaning
clean:
	rm -f $(TARGET) $(OBJECTS)
