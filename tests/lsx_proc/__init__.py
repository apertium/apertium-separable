#!/usr/bin/env python3
import unittest
from proctest import ProcTest, ProcTestNoFlush
from subprocess import Popen, PIPE
from tempfile import mkdtemp
from shutil import rmtree


class nullFlushTest(unittest.TestCase, ProcTest):
    procdix = "data/short-example.dix"
    procflags = ["-z"]
    inputs = [
        "[1] ^take<vblex><pres>$ ^it<prn><obj>$ ^out<adv>$",
        "[2] ^take<vblex><pres>$ ^me<prn><obj>$ ^out<adv>$",
        # out of LU characters test
        "[3] ^the<det>$ !!^Aragonese<n><sg>$ ;^take<vblex><pres>$ ;.^it<prn><obj>$   !;^out<adv>$ ^a<det><sg>$",
        # normal blanks test
        "[4] ^the<det>$ [<x>]^Aragonese<n><sg>$[</x>] [<y>]^take<vblex><pres>$ [<z>]^it<prn><obj>$ [</y></z>]^out<adv>$ [<a>]^a<det><sg>$[</a>]",
        # wordbound blank tests
        "[5] ^the<det>$ [[t:b:123456]]^Aragonese<n><sg>$ [[t:s:abc123]]^take<vblex><pres>$ [[t:b:xyz567]]^it<prn><obj>$ [[t:p:yui124]]^out<adv>$ [[t:b:uvw674]]^a<det><sg>$",
        "[6] ^the<det>$ [[t:b:123456]]^Aragonese<n><sg>$ [[t:s:abc123; t:p:hgb650]]^take<vblex><pres>$ ^it<prn><obj>$ [[t:p:yui124]]^out<adv>$ [[t:b:uvw674]]^a<det><sg>$",
        "[7] ^the<det>$ [[t:b:123456]]^Aragonese<n><sg>$ ^take<vblex><pres>$ [[t:b:xyz567]]^it<prn><obj>$ ^out<adv>$ [[t:b:uvw674]]^a<det><sg>$",
        "[8] ^the<det>$ [[t:b:123456]]^Aragonese<n><sg>$ ^take<vblex><pres>$ ^it<prn><obj>$ [[t:p:yui124]]^out<adv>$ [[t:b:uvw674]]^a<det><sg>$",
        "[9] ^the<det>$ [[t:b:123456]]^Aragonese<n><sg>$ [[t:s:abc123; t:p:hgb650]]^take<vblex><pres>$ ^it<prn><obj>$ [[t:p:yui124; t:x:puhbj23]]^out<adv>$ [[t:b:uvw674]]^a<det><sg>$"
    ]

    expectedOutputs = [
        "[1] ^take# out<vblex><pres>$ ^it<prn><obj>$",
        "[2] ^take# out<vblex><pres>$ ^me<prn><obj>$",
        "[3] ^the<det>$ !!^Aragonese<n><sg>$ ;^take# out<vblex><pres>$ ;.^it<prn><obj>$   !; ^a<det><sg>$",
        "[4] ^the<det>$ [<x>]^Aragonese<n><sg>$[</x>] [<y>]^take# out<vblex><pres>$ [<z>]^it<prn><obj>$ [</y></z>] [<a>]^a<det><sg>$[</a>]",
        "[5] ^the<det>$ [[t:b:123456]]^Aragonese<n><sg>$ [[t:s:abc123; t:p:yui124]]^take# out<vblex><pres>$ [[t:b:xyz567]]^it<prn><obj>$ [[t:b:uvw674]]^a<det><sg>$",
        "[6] ^the<det>$ [[t:b:123456]]^Aragonese<n><sg>$ [[t:s:abc123; t:p:hgb650; t:p:yui124]]^take# out<vblex><pres>$ ^it<prn><obj>$ [[t:b:uvw674]]^a<det><sg>$",
        "[7] ^the<det>$ [[t:b:123456]]^Aragonese<n><sg>$ ^take# out<vblex><pres>$ [[t:b:xyz567]]^it<prn><obj>$ [[t:b:uvw674]]^a<det><sg>$",
        "[8] ^the<det>$ [[t:b:123456]]^Aragonese<n><sg>$ [[t:p:yui124]]^take# out<vblex><pres>$ ^it<prn><obj>$ [[t:b:uvw674]]^a<det><sg>$",
        "[9] ^the<det>$ [[t:b:123456]]^Aragonese<n><sg>$ [[t:s:abc123; t:p:hgb650; t:p:yui124; t:x:puhbj23]]^take# out<vblex><pres>$ ^it<prn><obj>$ [[t:b:uvw674]]^a<det><sg>$"
    ]

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

class SpaceManipulation1(unittest.TestCase, ProcTest):
    procdir = "lr"
    procdix = "data/spaces.lsx"

    inputs = ["^a<ex>$^,<cm>$^b<ir>$",
              "^a<ex>$ ^,<cm>$ ^b<ir>$",
              "^a<ex>$ ^,<cm>$ _ ^b<ir>$",
              "^a<ex>$_^,<cm>$^b<ir>$"]
    expectedOutputs = ["^c<ex>$ ^d<ir>$",
                       "^c<ex>$ ^d<ir>$",
                       "^c<ex>$ ^d<ir>$ _ ",
                       "^c<ex>$_^d<ir>$"]

class SpaceManipulation2(unittest.TestCase, ProcTest):
    procdir = "rl"
    procdix = "data/spaces.lsx"

    inputs = ["^c<ex>$ ^d<ir>$",
              "^c<ex>$^d<ir>$",
              "^c<ex>$ _ ^d<ir>$",
              "^c<ex>$_^d<ir>$"]
    expectedOutputs = ["^a<ex>$^,<cm>$ ^b<ir>$",
                       "^a<ex>$^,<cm>$ ^b<ir>$",
                       "^a<ex>$^,<cm>$ _ ^b<ir>$",
                       "^a<ex>$^,<cm>$_^b<ir>$"]


class Empty(unittest.TestCase, ProcTest):
    procdir = "lr"
    procdix = "data/empty.lsx"
    inputs = ["^c<ex>$ ^d<ir>$"]
    expectedOutputs = ["^c<ex>$ ^d<ir>$"]

class Variant1(unittest.TestCase, ProcTest):
    procdix = "data/variants.dix"
    compflags = ['-v', 'abc']
    inputs = ['^take<vblex><pres>$ ^up<adv>$',
              '^take<vblex><pres>$ ^out<adv>$',
              '^take<vblex><pres>$ ^over<adv>$']
    expectedOutputs = ['^take# up<vblex><pres>$',
                       '^take# out<vblex><pres>$',
                       '^take<vblex><pres>$ ^over<adv>$']

class Variant2(unittest.TestCase, ProcTest):
    procdix = "data/variants.dix"
    compflags = ['-v', 'xyz']
    inputs = ['^take<vblex><pres>$ ^up<adv>$',
              '^take<vblex><pres>$ ^out<adv>$',
              '^take<vblex><pres>$ ^over<adv>$']
    expectedOutputs = ['^take# up<vblex><pres>$',
                       '^take<vblex><pres>$ ^out<adv>$',
                       '^take# over<vblex><pres>$']


class Weights(unittest.TestCase, ProcTest):
    procdix = "data/weights.lsx"
    inputs = ['^turn<vblex><pres>$ ^off<adv>$',
              '^turn<vblex><pres>$ ^things<n>$ ^off<adv>$',
              '^turn<vblex><pres>$ ^the<det>$ ^thing<n>$ ^off<adv>$',
              '^turn<vblex><pres>$ ^me<prn><pers>$ ^off<adv>$',
              ]
    expectedOutputs = ['^deactivate<vblex><pres>$',
                       '^deactivate<vblex><pres>$ ^things<n>$',
                       '^deactivate<vblex><pres>$ ^the<det>$ ^thing<n>$',
                       '^alienate<vblex><pres>$ ^me<prn><pers>$',
                       ]

class Repeat(unittest.TestCase, ProcTest):
    procdix = "data/repeat.lsx"
    procflags = ['-z', '-r']
    inputs = ['[1] ^hot<adj>$ ^dog<n><sg>$',
              '[2] ^take<vblex><pres>$ ^the<det><def><sp>$ ^silver<adj>$ ^dollar<n><sg>$ ^out<adv>$',
              '[3] ^take<vblex><pres>$ ^the<det><def><sp>$ ^hot<adj>$ ^dog<n><sg>$ ^out<adv>$']
    expectedOutputs = ['[1] ^hot# dog<n><sg>$',
                       '[2] ^take# out<vblex><pres>$ ^the<det><def><sp>$ ^silver<adj>$ ^dollar<n><sg>$',
                       '[3] ^take# out<vblex><pres>$ ^the<det><def><sp>$ ^hot# dog<n><sg>$']


class Postgen(unittest.TestCase, ProcTest):
    procdix = 'data/postgen.lsx'
    procflags = ['-z', '-p']
    inputs = [
        '[1] ^aginh<n>/aginh$ ^tom<adj>/toma$',
        '[2] ^el<det><def><f><sg>/la$ ^anguilla<n><f><sg>/anguilla$',
        '[3] ^la<prn><pro><f><sg>/la$ ^dar<vblex><imv>/da$'
    ]
    expectedOutputs = [
        '[1] ^aginh<n>/agis$^tom<adj>/oma$',
        "[2] ^el<det><def><f><sg>/l'$^anguilla<n><f><sg>/anguilla$",
        '[3] ^la<prn><pro><f><sg>/la$ ^dar<vblex><imv>/da$'
    ]

class PostgenWblank(unittest.TestCase, ProcTest):
    procdix = 'data/postgen.lsx'
    procflags = ['-z', '-p']
    inputs = [
        '[1] [[x]]^aginh<n>/aginh$ [[y]]^tom<adj>/toma$',
        '[2] [[x]]^el<det><def><f><sg>/la$ [[y]]^anguilla<n><f><sg>/anguilla$',
        '[3] [[x]]^la<prn><pro><f><sg>/la$ [[y]]^dar<vblex><imv>/da$'
    ]
    expectedOutputs = [
        '[1] [[x; y]]^aginh<n>/agis$[[x; y]]^tom<adj>/oma$',
        "[2] [[x]]^el<det><def><f><sg>/l'$[[y]]^anguilla<n><f><sg>/anguilla$",
        '[3] [[x]]^la<prn><pro><f><sg>/la$ [[y]]^dar<vblex><imv>/da$'
    ]

class PostgenForms(unittest.TestCase, ProcTest):
    procdix = 'data/forms.lsx'
    procflags = ['-z', '-p']
    inputs = [
        '^A\/B-test/A\/B-test<n><m><sg><ind>$',
        '^lov/lov<n><nt><sg><ind>$ ^om/om<pr>$ ^frittståande/frittståande<adj><pst><un><pl><ind>$ ^sjiraffar/sjiraff<n><m><pl><ind>$',
        '^i/i<pr>$ ^lov/lov<n><nt><sg><ind>$ ^om/om<pr>$ ^frittståande/frittståande<adj><pst><un><pl><ind>$ ^sjiraffar/sjiraff<n><m><pl><ind>$',
        '^i\/ved/i\/ved<pr>$ ^lov/lov<n><nt><sg><ind>$ ^om/om<pr>$ ^frittståande/frittståande<adj><pst><un><pl><ind>$ ^sjiraffar/sjiraff<n><m><pl><ind>$',
    ]
    expectedOutputs = [
        '^A\/B-test/A\/B-test<n><m><sg><ind>$',
        '^lov/lov<n><nt><sg><ind>$ ^om/om<pr>$ ^frittståande/frittståande<adj><pst><un><pl><ind>$ ^sjiraffar/sjiraff<n><m><pl><ind>$',
        '^i/i<pr>$ ^lov om frittståande sjiraffar/lov om frittståande sjiraffar<np>$',
        '^i\/ved/i\/ved<pr>$ ^lov om frittståande sjiraffar/lov om frittståande sjiraffar<np>$',
    ]
