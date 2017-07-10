#python reap_multiwords.py ../../apertium-en-es/apertium-en-es.en.metadix 

#!/usr/bin/env python
import sys, re

dix_file = sys.argv[1]
sep_words = []

with open(dix_file) as input_file:
    for line in input_file:
        line = line.strip()
        if len(line) > 2 and line.strip().count('<e lm=') > 0 and ('__sep' in line or 'sa="sep"' in line): # and 'sa="sep"' in line:
            lm = re.findall('"([^"]*)"', line)[0]
            sep_words.append(lm)
for word in sep_words:
    print word
