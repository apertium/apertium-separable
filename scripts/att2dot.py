import sys ;

print('digraph G {');
print('rankdir=LR');
print('node [shape=circle];');

for line in sys.stdin.readlines(): #{

	row = line.strip('\n').split('\t');

	if len(row) == 5: #{
		print('\t%s->%s [label="%s:%s"];' % (row[0], row[1], row[2], row[3]));
	elif len(row) == 1: #{
		print('\t%s [style=double];' % (row[0]));
	#}
#}

print('}');
