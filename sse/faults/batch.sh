LMBENCH_DIR=/home/pureza/lmbench3/bin/i686-pc-linux-gnu

$LMBENCH_DIR/lat_pipe
$LMBENCH_DIR/lat_ctx 10
$LMBENCH_DIR/lat_fs
$LMBENCH_DIR/lat_proc exec
$LMBENCH_DIR/lat_syscall read
$LMBENCH_DIR/lmdd count=100000000

wget ftp://ftp.dei.uc.pt/pub/linux/gentoo/experimental/x86/vserver/stage1-x86-20060317.tar.bz2 -O stage1 &> /dev/null
md5sum stage1 | cut -d" " -f 1
rm stage1
