obj-y := start_elevator.o issue_request.o stop_elevator.o
obj-m += elevator.o

PWD := `pwd`
KERNELDIR := /lib/modules/`uname -r`/build
default:
        $(MAKE) -C $(KERNELDIR) M=$(PWD) modules
clean:
        rm -f *.ko *.o Module* *mod*
