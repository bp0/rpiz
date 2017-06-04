
CFLAGS = -O2 -Wall -Wextra -Wmissing-prototypes -Wstrict-prototypes -Werror=implicit-function-declaration -Werror=missing-prototypes

objects = util.o fields.o arm_data.o cpu_arm.o x86_data.o cpu_x86.o cpu.o board_dt.o board_dmi.o board_rpi.o board.o

rpiz-cli : rpiz-cli.c $(objects)
	rm rpiz-cli
	cc -o rpiz-cli rpiz-cli.c $(objects)

rpiz-gtk : rpiz-gtk.c $(objects)
	rm rpiz-gtk
	cc -o rpiz-gtk rpiz-gtk.c $(objects) `pkg-config --cflags --libs gtk+-2.0`

util.o : util.h
fields.o : fields.h
arm_data.o : arm_data.h
cpu_arm.o : cpu_arm.h arm_data.o util.o fields.o
x86_data.o : x86_data.h
cpu_x86.o : cpu_x86.h x86_data.o util.o fields.o
cpu.o : cpu.h cpu_arm.o cpu_x86.o util.o fields.o
board_dt.o : board_dt.h util.o fields.o
board_dmi.o : board_dmi.h util.o fields.o
board_rpi.o : board_rpi.h board_dt.o util.o fields.o
board.o : board.h board_dt.o board_dmi.o board_rpi.o util.o fields.o

.PHONY : clean
clean :
	-rm rpiz-gtk rpiz-cli $(objects)
