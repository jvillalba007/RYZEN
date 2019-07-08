COMMONS = commons
SUBDIRS = shared/Shared MEMORIA/Memoria LFS/Lfs KERNEL/Kernel

cinstall:
	$(MAKE) -C $(COMMONS) install;
compilar:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir all; \
	done
limpiar:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done
cuninstall:
	$(MAKE) -C $(COMMONS) uninstall;
