# -*- coding: utf-8 -*-
from __future__ import unicode_literals

import sys
import unittest
from proctest import ProcTest

class nullFlushTest(unittest.TestCase, ProcTest):
    procdix = "data/short-example.dix"
    procflags = ["-z"]
    inputs = ["^take<vblex><pres>$ ^it<prn><obj>$ ^out<adv>$"]
    expectedOutputs = ["^take# out<vblex><pres>$ ^it<prn><obj>$"]

# These fail on some systems:
#from null_flush_invalid_stream_format import *
