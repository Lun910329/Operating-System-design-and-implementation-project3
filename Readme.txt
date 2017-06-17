*************************** MINIX  VERSION3.4 **************************************
1.Source codes(because we modified servers and libs and sys etc. so you probably need download all minix from my github, because I am not sure whether you can compile correctly if somewhere missing.)
The whole image link is: https://drive.google.com/a/iit.edu/file/d/0Bw0d-ZkX59ijNDdqb3Y5YUFXY2c/view?usp=sharing

2. To compile minix, it may take serveral minutes to finish compile:
# cd /usr/src
# make build

3. Imcrement update(not suggested):
# cd /usr/src/
# make build MKUPDATE=yes

4. Modify logs(may not complete).
./include:
minix  unistd.h

./include/minix:
callnr.h  vfsif.h

./minix:
fs  lib  servers

./minix/fs:
mfs

./minix/fs/mfs:
Makefile  proto.h  syscl_mfs.c  table.c

./minix/lib:
libc  libfsdriver

./minix/lib/libc:
sys

./minix/lib/libc/sys:
lib_vfs2mfs.c  Makefile.inc

./minix/lib/libfsdriver:
call.c  fsdriver.h  table.c

./minix/servers:
vfs

./minix/servers/vfs:
Makefile  proto.h  request.c  table.c  vfs_vfs2mfs.c
	

5. Test cases(/root)       
a                    inodeBitmap.c           zoneBitmap.c
damageInodeBitmap    inodeBitmap_recovery    zoneBitmap_recovery
damageInodeBitmap.c  inodeBitmap_recovery.c  zoneBitmap_recovery.c
damageZoneBitmap     inodeBitmap.txt         zoneBitmap.txt
damageZoneBitmap.c   inodeWalker             zoneWalker
dir.c                inodeWalker.c           zoneWalker.c
directoryWalker      syscl_fs.h
inodeBitmap          zoneBitmap

./a:
b

./a/b:
aaa.txt
