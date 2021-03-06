#!/usr/bin/env python3
import unittest
from proctest import ProcTest, ProcTestNoFlush
from subprocess import Popen, PIPE
from tempfile import mkdtemp
from shutil import rmtree


class nullFlushTest(unittest.TestCase, ProcTest):
    procdix = "data/short-example.dix"
    procflags = ["-z"]
    inputs = ["^take<vblex><pres>$ ^it<prn><obj>$ ^out<adv>$",
    "^take<vblex><pres>$ ^me<prn><obj>$ ^out<adv>$",
     "^the<det>$ !!^Aragonese<n><sg>$ ;^take<vblex><pres>$ ;.^it<prn><obj>$   !;^out<adv>$ ^a<det><sg>$", #out of LU characters test
     "^the<det>$ [<x>]^Aragonese<n><sg>$[</x>] [<y>]^take<vblex><pres>$ [<z>]^it<prn><obj>$ [</y></z>]^out<adv>$ [<a>]^a<det><sg>$[</a>]", #normal blanks test
     "^the<det>$ [[t:b:123456]]^Aragonese<n><sg>$ [[t:s:abc123]]^take<vblex><pres>$ [[t:b:xyz567]]^it<prn><obj>$ [[t:p:yui124]]^out<adv>$ [[t:b:uvw674]]^a<det><sg>$", #wordbound blank tests
     "^the<det>$ [[t:b:123456]]^Aragonese<n><sg>$ [[t:s:abc123; t:p:hgb650]]^take<vblex><pres>$ ^it<prn><obj>$ [[t:p:yui124]]^out<adv>$ [[t:b:uvw674]]^a<det><sg>$",
     "^the<det>$ [[t:b:123456]]^Aragonese<n><sg>$ ^take<vblex><pres>$ [[t:b:xyz567]]^it<prn><obj>$ ^out<adv>$ [[t:b:uvw674]]^a<det><sg>$",
     "^the<det>$ [[t:b:123456]]^Aragonese<n><sg>$ ^take<vblex><pres>$ ^it<prn><obj>$ [[t:p:yui124]]^out<adv>$ [[t:b:uvw674]]^a<det><sg>$",
     "^the<det>$ [[t:b:123456]]^Aragonese<n><sg>$ [[t:s:abc123; t:p:hgb650]]^take<vblex><pres>$ ^it<prn><obj>$ [[t:p:yui124; t:x:puhbj23]]^out<adv>$ [[t:b:uvw674]]^a<det><sg>$"]
     
    expectedOutputs = ["^take# out<vblex><pres>$ ^it<prn><obj>$",
    "^take# out<vblex><pres>$ ^me<prn><obj>$",
     "^the<det>$ !!^Aragonese<n><sg>$ ;^take# out<vblex><pres>$ ;.^it<prn><obj>$   !; ^a<det><sg>$",
     "^the<det>$ [<x>]^Aragonese<n><sg>$[</x>] [<y>]^take# out<vblex><pres>$ [<z>]^it<prn><obj>$ [</y></z>] [<a>]^a<det><sg>$[</a>]",
     "^the<det>$ [[t:b:123456]]^Aragonese<n><sg>$ [[t:s:abc123; t:b:xyz567; t:p:yui124]]^take# out<vblex><pres>$ [[t:s:abc123; t:b:xyz567; t:p:yui124]]^it<prn><obj>$ [[t:b:uvw674]]^a<det><sg>$",
     "^the<det>$ [[t:b:123456]]^Aragonese<n><sg>$ [[t:s:abc123; t:p:hgb650; t:p:yui124]]^take# out<vblex><pres>$ [[t:s:abc123; t:p:hgb650; t:p:yui124]]^it<prn><obj>$ [[t:b:uvw674]]^a<det><sg>$",
     "^the<det>$ [[t:b:123456]]^Aragonese<n><sg>$ [[t:b:xyz567]]^take# out<vblex><pres>$ [[t:b:xyz567]]^it<prn><obj>$ [[t:b:uvw674]]^a<det><sg>$",
     "^the<det>$ [[t:b:123456]]^Aragonese<n><sg>$ [[t:p:yui124]]^take# out<vblex><pres>$ [[t:p:yui124]]^it<prn><obj>$ [[t:b:uvw674]]^a<det><sg>$",
     "^the<det>$ [[t:b:123456]]^Aragonese<n><sg>$ [[t:s:abc123; t:p:hgb650; t:p:yui124; t:x:puhbj23]]^take# out<vblex><pres>$ [[t:s:abc123; t:p:hgb650; t:p:yui124; t:x:puhbj23]]^it<prn><obj>$ [[t:b:uvw674]]^a<det><sg>$"]

class capitalizationTest(unittest.TestCase, ProcTest):
    procdix = "data/capitalization.dix"
    procflags = ["-z"]

    inputs = ["^Jun<num>$ ^Ajpu<np><ant><m>$",
              "^JUN<num>$ ^AJPU<np><ant><m>$",
              "^hargle<np>$ ^bargle<np>$",
              "^HaRgLe<np>$ ^BaRgLe<np>$"]

    expectedOutputs = ["^Jun Ajpu<np><ant><m>$",
                       "^JUN AJPU<np><ant><m>$",
                       "^hargle bargle<np>$",
                       "^Hargle bargle<np>$"]

class dictionaryCaseTest(unittest.TestCase, ProcTest):
    procdix = "data/capitalization.dix"
    procflags = ["-z", "-w"]

    inputs = ["^Jun<num>$ ^Ajpu<np><ant><m>$",
              "^JUN<num>$ ^AJPU<np><ant><m>$",
              "^hargle<np>$ ^bargle<np>$",
              "^HaRgLe<np>$ ^BaRgLe<np>$"]

    expectedOutputs = ["^Jun Ajpu<np><ant><m>$",
                       "^Jun Ajpu<np><ant><m>$",
                       "^hargle bargle<np>$",
                       "^hargle bargle<np>$"]

class splittingTest(unittest.TestCase, ProcTest):
    procdir = "rl"
    procdix = "data/short-example.dix"

    inputs = ["^take# out<vblex><pres>$ ^it<prn><obj>$",
    "^take# out<vblex><pres>$ ^me<prn><obj>$",
     "^the<det>$ !!^Aragonese<n><sg>$ ;^take# out<vblex><pres>$ ;.^it<prn><obj>$   !; ^a<det><sg>$"]

    expectedOutputs = ["^take<vblex><pres>$ ^it<prn><obj>$ ^out<adv>$",
    "^take<vblex><pres>$ ^me<prn><obj>$ ^out<adv>$",
     "^the<det>$ !!^Aragonese<n><sg>$ ;^take<vblex><pres>$ ;.^it<prn><obj>$ ^out<adv>$   !; ^a<det><sg>$"]


class issue26NoFlushTest(unittest.TestCase, ProcTestNoFlush):
    procdir = "lr"
    procdix = "data/short-example.dix"

    inputs = ["^.<sent>$"]
    expectedOutputs = ["^.<sent>$"]
