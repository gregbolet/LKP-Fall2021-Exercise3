## [M1: point 1]
#  Here we set the variable MODULE to hold the value 'ex3', which is the
#  name of our '.c' code file.
#  ...
MODULE = ex3

## [M2: point 1]
#  Here we add to the modules object list the name of our
#  object file that is to be generated
#  ...
obj-m += $(MODULE).o

## [M3: point 1]
#  Here we set the KERNELDIR to where the makefile for building
#  modules is located. The '?=' means to only set the variable
#  if it doesn't already have a value.
#  ...
KERNELDIR ?= /lib/modules/$(shell uname -r)/build

## [M4: point 1]
#  Here we grab the current directory that this makefile is located in
#  ...
PWD := $(shell pwd)

## [M5: point 1]
#  Here we set the trigger dependency of the 'all' rule to be 
#  the name of our '.c' code file.
#  ...
all: $(MODULE)


## [M6: point 1]
#  Here we print to the terminal what file we will comiple. We then
#  run the compilation of our '.c' code and have the output be an
#  object '.o' file. The '$<' means to grab the first dependency
#  of this rule (i.e: the '.c' file name). The "$@" means to grab the
#  target of this rule (i.e: the '.o' file name).
#  ...
%.o: %.c
	@echo "  CC      $<"
	@$(CC) -c $< -o $@

## [M7: point 1]
#  This sets the rule to build 'ex3' by calling the Makefile in the
#  KERNELDIR and telling it to work with the file in the current
#  directory (the M=$PWD). It triggers the 'modules' rule which will
#  build any modules in the current directory.
#  ...
$(MODULE):
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

## [M8: point 1]
#  This rule is similar to the previous rule, but it doesn't make any
#  modules. It instead deletes the files that were generated by
#  the 'make modules' call.
#  ...
clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean