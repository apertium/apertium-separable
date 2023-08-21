pkgconfigdir = $(libdir)/pkgconfig
dist_pkgconfig_DATA = apertium-separable.pc

SUBDIRS = src locales

EXTRA_DIST=autogen.sh

test:
	(cd tests; ./run_tests.py)

export LTTB_I18N_DATA=$(datadir)/lttoolbox/lttoolbox.dat
export LOCALES_DIR=$(datadir)/$(PACKAGE_NAME)
export APSP_I18N_DATA=$(LOCALES_DIR)/apsp.dat
