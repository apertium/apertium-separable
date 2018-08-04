import sys;

for line in sys.stdin.readlines(): #{

	if line.strip() == '': #{
		continue
	#}

	row = line.strip('\n').split('\t');

	# Final state
	if len(row) == 1: #{
		print(row[0]);
		continue;
	#}

	# Replace word boundary symbol with the special one
	if row[2] == '+': #{
		row[2] = '<$>';
	#}
	if row[3] == '+': #{
		row[3] = '<$>';
	#}

	# Replace ~ symbol with <ANY_TAG> and add a loop
	if row[2] == '~': #{
		row[2] = '<ANY_TAG>';
		#print('%s\t%s\t%s\t%s' % (row[1], row[1], row[2], 'ε'));
		print('%s\t%s\t%s\t%s' % (row[1], row[1], row[2], row[2]));
	#}
#	if row[3] == '~': #{
#		row[3] = '<ANY_TAG>';
#		print('%s\t%s\t%s\t%s' % (row[1], row[1], row[3], row[3]));
#		print('%s\t%s\t%s\t%s' % (row[1], row[1], 'ε', row[3]));
#	#}


	print('\t'.join(row));
#}
