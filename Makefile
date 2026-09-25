KCONFIG_MCONF = ./src/kconfig/output/bin/kconfig-mconf

$(KCONFIG_MCONF):
	@echo "Bygger lokalt Kconfig-verktyg..."
	cd src/kconfig && autoreconf -fi
	cd src/kconfig && ./configure --enable-conf --enable-mconf --disable-shared --enable-static --prefix=$(shell pwd)/src/kconfig/output
	$(MAKE) -C src/kconfig
	$(MAKE) -C src/kconfig install

menuconfig: $(KCONFIG_MCONF)
	$(KCONFIG_MCONF) Kconfig

all:
	mkdir -p output

clean:
	$(MAKE) -C src/kconfig clean
	rm -rf output/*