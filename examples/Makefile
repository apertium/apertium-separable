all:
	lt-comp lr example.dix example.bin
	lt-print example.bin | python3 convert-mwe-att.py > example.att

# cat example.att | sed 's/ /@_SPACE_@/g' | hfst-txt2fst -e ε -o example.hfst
