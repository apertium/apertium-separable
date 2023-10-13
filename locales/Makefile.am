asp.dat: root.txt en.txt
	genrb -d . root.txt en.txt
	echo root.res en.res > package_list.txt
	pkgdata -p asp --mode archive -d . package_list.txt

localesdir = $(LOCALES_DIR)
dist_locales_DATA = asp.dat
