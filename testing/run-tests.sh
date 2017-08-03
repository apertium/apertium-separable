CORP=../corpus/
MODULE=../src/
LANGS=english

for lang in $LANGS; do
	cat $CORP"/"$lang.src | $MODULE./lsx-proc $MODULE"/"$lang.bin > $CORP"/"$lang.tst
	python3 evaluate.py $CORP"/"$lang.src $CORP"/"$lang.ref $CORP"/"$lang.tst
done

