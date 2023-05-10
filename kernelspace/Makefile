#obj-m	+=	/source/main.o
#main-objs := ./source/utils.o ./source/infectivity.o
#all:
#	make	-C	/lib/modules/$(shell uname -r)/build	M=$(PWD) modules
#	make	-C	/lib/modules/5.10.176/build	M=$(PWD) modules
#clean:
#	make	-C	/lib/modules/$(shell uname -r)/build	M=$(PWD) clean
#	make	-C	/lib/modules/5.10.176/build	M=$(PWD) clean

# Set the kernel version
KERNEL_VER := $(shell uname -r)

# Set the name of the kernel module
MODULE_NAME := infectivity-test

# Set the path to the kernel source directory
KERNEL_SRC := /lib/modules/$(KERNEL_VER)/build

# ACT_DIR = $(shell pwd)
# SOURCE_FILES:=$(shell ../get_source-files.sh ./source ;../get_source-files.sh ./common | cat |sed -e "s/\.c/\.o/"| tr '\n' ' ')
# $(info ls is $(SOURCE_FILES))
# Set the object files to be built
obj-m := $(MODULE_NAME).o
$(MODULE_NAME)-objs := ./source/infectivity.o ./source/interceptor.o ./source/main.o ./source/utils.o ./source/communicator.o

# List the source files
#$(MODULE_NAME)-y := ./source/main.c ./source/utils.c ./source/infectivity.c

# List the header files
EXTRA_CFLAGS += -I$(PWD)/headers
#$(MODULE_NAME)-y += /headers/utils.h /headers/infectivity.h

# Build the module
all:
#	make -C $(KERNEL_SRC) M=$(PWD) modules
	make	-C	/lib/modules/$(shell uname -r)/build	M=$(PWD) modules

# Clean the build directory
clean:
#	make -C $(KERNEL_SRC) M=$(PWD) clean
	make	-C	/lib/modules/$(shell uname -r)/build	M=$(PWD) clean
	# rm *.o *.mod *.mod*
	# rm source/*.o