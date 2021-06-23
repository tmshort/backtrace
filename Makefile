TOPDIR=$(PWD)

all: test_bt

openssl/Makefile:
	(cd openssl && ./config --prefix=$(TOPDIR)/usr/local -g)

.PHONY: openssl
openssl: openssl/Makefile
	make -C openssl -s -j 4
	make -C openssl install_sw

libunwind/Makefile: 
	(cd libunwind && ./autogen.sh && ./configure --enable-debug)

.PHONY: libunwind
libunwind: libunwind/Makefile
	make -C libunwind -s -j 4
	make -C libunwind install prefix=$(TOPDIR)/usr/local

test_bt: openssl libunwind test_bt.cpp
	gcc -g -o test_bt test_bt.cpp -l:libcrypto.a -l:libunwind.a -lstdc++ -lrt -llzma -L $(TOPDIR)/usr/local/lib -I $(TOPDIR)/usr/local/include

.PHONY: clean

clean:
	git clean -fdx
	(cd libunwind && git clean -fdx)
	(cd openssl && git clean -fdx)
