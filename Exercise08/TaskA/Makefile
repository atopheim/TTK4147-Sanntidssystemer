BUILDROOTDIR := /home/student/Desktop/buildroot
TARGETDIR := /export/nfs
CC := $(BUILDROOTDIR)/output/staging/usr/bin/avr32-linux-gcc

all: hello.c
	$(CC) -o output hello.c 

copy:
	sudo cp output $(TARGETDIR)
	
clean:
	rm rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions
