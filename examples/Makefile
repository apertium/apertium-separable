all:
	lt-comp lr example.dix example.bin
	lt-print example.bin | python3 convert-mwe-att.py > example.att
#	cat example.att | sed 's/ /@_SPACE_@/g' | hfst-txt2fst -e ε -o example.hfst
	cat example.2.att | sed 's/ /@_SPACE_@/g' | hfst-txt2fst -e ε -o example.2.hfst

# echo "pick<vblex><ANY_TAG><$><ANY_CHAR><ANY_CHAR><ANY_CHAR><ANY_CHAR><det><ANY_TAG><ANY_TAG><$><ANY_CHAR><ANY_CHAR><ANY_CHAR><n><ANY_TAG><ANY_TAG><$>up<adv>" | hfst-lookup -p example.2.hfst
