wget ftp://ftp.dei.uc.pt/pub/linux/gentoo/experimental/x86/vserver/stage1-x86-20060317.tar.bz2 -O stage1 &> /dev/null
md5sum stage1 | cut -d" " -f 1
rm stage1
