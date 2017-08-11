import sys;

src = open(sys.argv[1]);   # input to MWE module from POS tagger
ref = open(sys.argv[2]);   # desired output from MWE module
tst = open(sys.argv[3]);   # output from the MWE module

hit = 0.0;
miss = 0.0;

while True: #{

	src_l = src.readline();
	ref_l = ref.readline();
	tst_l = tst.readline();

	if src_l.strip() == '' or ref_l.strip() == '' or tst_l.strip() == '':  #{
		break;
	# }

	if tst_l == ref_l: #{
		hit += 1.0
	else: #{
		miss += 1.0
		if miss < 5:
			print (hit+miss, 'tst', tst_l)
			print (hit+miss, 'ref', ref_l)
	#}

#}

print('total: %d' % (hit+miss));
print('correct: %d' % hit);
print('accur: %.2f%%' % (hit/(hit+miss)*100.0));
