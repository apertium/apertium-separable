
CORP=../corpus/
LANGS=english

for lang in $LANGS; do 
	# Just copy input to output, but this is where your module is going to go
	# you're going to do cat $CORP"/"$lang.src | your-module.py DATA > $CORP"/"$lang.tst

	cat $CORP"/"$lang.src > $CORP"/"$lang.tst

	# Run the evaluation
	python3 evaluate.py $CORP"/"$lang.src $CORP"/"$lang.ref $CORP"/"$lang.tst
done

