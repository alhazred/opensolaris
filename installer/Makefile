includes = -I./lib/libti -I./lib/libtd
libs = -Llibs  -ltd -lti -lnvpair -lgen  
zfsinstall: disk.obj ti.obj install.obj
	cc disk.o ti_utils.o install.o ${libs} -o $@
disk.obj: disk.c
	cc -c disk.c ${includes}
ti.obj: ti_utils.c
	cc -c ti_utils.c ${includes}
install.obj: install.c
	cc -c install.c ${includes}
