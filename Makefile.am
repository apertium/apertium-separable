pkgconfigdir = $(libdir)/pkgconfig
dist_pkgconfig_DATA = apertium-separable.pc

SUBDIRS = src

EXTRA_DIST=autogen.sh

test:
	(cd tests; ./run_tests.py)
