obj-$(CONFIG_LUSTRE_FS) += libcfs.o

libcfs-linux-objs := linux-tracefile.o linux-debug.o
libcfs-linux-objs += linux-prim.o linux-cpu.o
libcfs-linux-objs += linux-tcpip.o
libcfs-linux-objs += linux-curproc.o
libcfs-linux-objs += linux-module.o
libcfs-linux-objs += linux-crypto.o
libcfs-linux-objs += linux-crypto-adler.o

libcfs-linux-objs := $(addprefix linux/,$(libcfs-linux-objs))

libcfs-all-objs := debug.o fail.o nidstrings.o module.o tracefile.o \
		   libcfs_string.o hash.o kernel_user_comm.o \
		   prng.o workitem.o libcfs_cpu.o \
		   libcfs_mem.o libcfs_lock.o

libcfs-objs := $(libcfs-linux-objs) $(libcfs-all-objs)
