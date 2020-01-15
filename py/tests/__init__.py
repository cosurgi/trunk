# encoding: utf-8
# 2009 © Václav Šmilauer <eudoxos@arcig.cz>
"""All defined functionality tests for yade."""
import unittest,inspect,sys

# add any new test suites to the list here, so that they are picked up by testAll
allTests=[
	  'dummyTest'
	, 'wrapper'
	, 'core'
	, 'pbc'
	, 'clump'
	, 'cohesive-chain'
	, 'engines'
	, 'utilsModule'
	, 'libVersions'
	#, 'testColliderByGiulia' # to investigate later maybe, my impression is that due to issue #7 the results are randomly different in that ill-posed periodic problem
]

# add any new yade module (ugly...)
import yade.export,yade.linterpolation,yade.log,yade.pack,yade.plot,yade.post2d,yade.timing,yade.utils,yade.ymport,yade.geom,yade.gridpfacet,yade.libVersions
allModules=(yade.export,yade.linterpolation,yade.log,yade.pack,yade.plot,yade.post2d,yade.timing,yade.utils,yade.ymport,yade.geom,yade.gridpfacet,yade.libVersions)
try:
	import yade.qt
	allModules+=(yade.qt,)
except ImportError: pass

# fully qualified module names
allTestsFQ=['yade.tests.'+test for test in allTests]

def testModule(module):
	"""Run all tests defined in the module specified, return TestResult object
	(http://docs.python.org/library/unittest.html#unittest.TextTestResult)
	for further processing.

	@param module: fully-qualified module name, e.g. yade.tests.wrapper
	"""
	suite=unittest.defaultTestLoader.loadTestsFromName(module)
	return unittest.TextTestRunner(stream=sys.stdout,verbosity=2).run(suite)

# https://docs.python.org/3/library/unittest.html
# https://stackoverflow.com/questions/10099491/how-does-pythons-unittest-module-detect-test-cases
# In short: to add a TestCase a class must be written inside python module, like this:
# class TestSomeYadeModule(unittest.TestCase):
#	……
def testAll():
	"""Run all tests defined in all yade.tests.* modules and return
	TestResult object for further examination."""
	suite=unittest.defaultTestLoader.loadTestsFromNames(allTestsFQ)
	import doctest
	for mod in allModules:
		suite.addTest(doctest.DocTestSuite(mod))
	return unittest.TextTestRunner(stream=sys.stdout,verbosity=2).run(suite)

