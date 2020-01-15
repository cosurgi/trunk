/*************************************************************************
*  2006-2008 © Václav Šmilauer                                           *
*  2019      © Janek Kozicki, rewritten using boost::log the version     *
*              which was removed in 2008 (git revision 014b11496)        *
*                                                                        *
*  This program is free software; it is licensed under the terms of the  *
*  GNU General Public License v2 or later. See file LICENSE for details. *
*************************************************************************/

// hint: follow changes in d067b0696a8 to add new modules.

#include <lib/base/AliasNamespaces.hpp>
#include <lib/base/Logging.hpp>
#include <lib/pyutil/doc_opts.hpp>
#include <core/Omega.hpp>
#include <string>

CREATE_CPP_LOCAL_LOGGER("_log.cpp");

namespace yade { // Cannot have #include directive inside.

#ifdef YADE_BOOST_LOG

int getDefaultLogLevel() { return Logging::instance().getDefaultLogLevel(); }

void setDefaultLogLevel(int level) { Logging::instance().setNamedLogLevel("Default", level); }

// accepted streams: "clog", "cerr", "cout", "filename"
// It is possible to set different levels per log file, see notes about that in Logging::setOutputStream(…)
void setOutputStream(std::string streamName, bool reset)
{
	Logging::instance().setOutputStream(streamName, reset);
	if (reset) {
		LOG_INFO("Log output stream has been set to " << streamName << ". Other output streams were removed.");
	} else {
		LOG_INFO("Additional output stream has been set to " << streamName << ".");
	}
}

void resetOutputStream()
{
	Logging::instance().setOutputStream("clog", true);
	LOG_INFO("Log output stream has been reset to std::clog. File sinks are not removed.");
}

void setLevel(std::string className, int level)
{
	Logging::instance().setNamedLogLevel(className, level);
	LOG_INFO("filter log level for " << className << " has been set to " << Logging::instance().getNamedLogLevel(className));
}

void unsetLevel(std::string className)
{
	Logging::instance().unsetNamedLogLevel(className);
	LOG_INFO("filter log level for " << className << " has been unset to " << Logging::instance().getNamedLogLevel(className));
}

py::dict getAllLevels()
{
	py::dict ret {};
	for (const auto& a : Logging::instance().getClassLogLevels()) {
		ret[a.first] = a.second;
	}
	return ret;
}

py::dict getUsedLevels()
{
	py::dict ret {};
	for (const auto& a : Logging::instance().getClassLogLevels()) {
		if (a.second != -1) {
			ret[a.first] = a.second;
		}
	}
	return ret;
}

void setUseColors(bool use) { Logging::instance().setUseColors(use); }

void readConfigFile(std::string fname) { Logging::instance().readConfigFile(fname); }

void saveConfigFile(std::string fname) { Logging::instance().saveConfigFile(fname); }

std::string defaultConfigFileName() { return Logging::instance().defaultConfigFileName(); }

int getMaxLevel() { return Logging::instance().maxLogLevel; }

#else

void printNoBoostLogWarning()
{
	std::cerr
	        << "\nWarning: yade was compiled with cmake option -DENABLE_LOGGER=OFF, any attempts to manipulate log filter levels will not have effect.\n\n";
}

int      getDefaultLogLevel() { return std::min(MAX_LOG_LEVEL, MAX_HARDCODED_LOG_LEVEL); }
void     setDefaultLogLevel(int) { printNoBoostLogWarning(); }
void     setOutputStream(std::string, bool) { printNoBoostLogWarning(); }
void     resetOutputStream() { printNoBoostLogWarning(); }
void     setLevel(std::string, int) { printNoBoostLogWarning(); }
void     unsetLevel(std::string) { printNoBoostLogWarning(); }
py::dict getAllLevels()
{
	printNoBoostLogWarning();
	return {};
}
py::dict getUsedLevels()
{
	printNoBoostLogWarning();
	return {};
}
void        setUseColors(bool) { printNoBoostLogWarning(); }
void        readConfigFile(std::string) { printNoBoostLogWarning(); }
void        saveConfigFile(std::string) { printNoBoostLogWarning(); }
std::string defaultConfigFileName()
{
	printNoBoostLogWarning();
	return "";
}
int getMaxLevel() { return std::min(MAX_LOG_LEVEL, MAX_HARDCODED_LOG_LEVEL); }

#endif

void testAllLevels()
{
	int                testInt = 0;
	std::string        testStr = "test string";
	Real               testReal(11);
	Vector3r           testVec(1, 2, 3);
	Matrix3r           testMat = (Matrix3r() << 1, 2, 3, 4, 5, 6, 7, 8, 9).finished();
	std::complex<Real> testComplex(-1, 1);

	LOG_0_NOFILTER("Test log level: LOG_0_NOFILTER, test int: " << testInt++);
	LOG_1_FATAL("Test log level: LOG_1_FATAL, test int: " << testInt++);
	LOG_2_ERROR("Test log level: LOG_2_ERROR, test int: " << testInt++);
	LOG_3_WARN("Test log level: LOG_3_WARN, test int: " << testInt++);
	LOG_4_INFO("Test log level: LOG_4_INFO, test int: " << testInt++);
	LOG_5_DEBUG("Test log level: LOG_5_DEBUG, test int: " << testInt++);
	LOG_6_TRACE("Test log level: LOG_6_TRACE, test int: " << testInt++);

	LOG_0_NOFILTER("Below 6 variables are printed at filter level TRACE, then macro TRACE; is used");
	TRVAR1(testInt);
	TRVAR2(testInt, testStr);
	TRVAR3(testInt, testStr, testReal);
	TRVAR4(testInt, testStr, testReal, testVec);
	TRVAR5(testInt, testStr, testReal, testVec, testMat);
	TRVAR6(testInt, testStr, testReal, testVec, testMat, testComplex);

	TRACE;
}

} // namespace yade

// BOOST_PYTHON_MODULE cannot be inside yade namespace, it has 'extern "C"' keyword, which strips it out of any namespaces.
BOOST_PYTHON_MODULE(_log)
try {
	using namespace yade; // 'using namespace' inside function keeps namespace pollution under control. Alernatively I could add y:: in front of function names below and put 'namespace y  = ::yade;' here.
	namespace py = ::boost::python;
	YADE_SET_DOCSTRING_OPTS;
	// We can use C++ string literal just like """ """ in python to write docstrings (see. https://en.cppreference.com/w/cpp/language/string_literal )
	// The """ is a custom delimeter, we could use    R"RAW( instead, or any other delimeter. This decides what will be the termination delimeter.
	// The docstrings can use syntax :param ……: ……… :return: ……. For details see https://thomas-cokelaer.info/tutorials/sphinx/docstring_python.html

	py::def("testAllLevels", testAllLevels, R"""(
This function prints test messages on all log levels. Can be used to see how filtering works and to what streams the logs are written.
	)""");

	// default level
	py::def("getDefaultLogLevel", getDefaultLogLevel, R"""(
:return: The current ``Default`` filter log level.
	)""");

	py::def("setDefaultLogLevel", setDefaultLogLevel, R"""(
:param int level: Sets the ``Default`` filter log level, same as calling ``log.setLevel("Default",level)``.
	)""");

	// output streams, files, cout, cerr, clog.
	py::def("setOutputStream", setOutputStream, R"""(
:param str streamName: sets the output stream, special names ``cout``, ``cerr``, ``clog`` use the ``std::cout``, ``std::cerr``, ``std::clog`` counterpart (``std::clog`` the is the default output stream). Every other name means that log will be written to a file with name provided in the argument.
:param bool reset: dictates whether all previously set output streams are to be removed. When set to false: the new output stream is set additionally to the current one.
	)""");

	py::def("resetOutputStream", resetOutputStream, R"""(
Resets log output stream to default state: all logs are printed on ``std::clog`` channel, which usually redirects to ``std::cerr``.
	)""");

	// filter levels
	py::def("setLevel", setLevel, R"""(
Set filter level (constants ``TRACE`` (6), ``DEBUG`` (5), ``INFO`` (4), ``WARN`` (3), ``ERROR`` (2), ``FATAL`` (1), ``NOFILTER`` (0)) for given logger.

:param str className: The logger name for which the filter level is to be set. Use name ``Default`` to change the default filter level.
:param int level: The filter level to be set.
.. warning:: setting ``Default`` log level higher than ``MAX_LOG_LEVEL`` provided during compilation will have no effect. Logs will not be printed because they are removed during compilation.
	)""");

	py::def("unsetLevel", unsetLevel, R"""(
:param str className: The logger name for which the filter level is to be unset, so that a ``Default`` will be used instead. Unsetting the ``Default`` level will change it to max level and print everything.
	)""");

	py::def("getAllLevels", getAllLevels, R"""(
:return: A python dictionary with all known loggers in yade. Those without a debug level set will have value -1 to indicate that ``Default`` filter log level is to be used for them.
	)""");

	py::def("getUsedLevels", getUsedLevels, R"""(
:return: A python dictionary with all used log levels in yade. Those without a debug level (value -1) are omitted.
	)""");

	py::def("getMaxLevel", getMaxLevel, R"""(
:return: the MAX_LOG_LEVEL of the current yade build.
	)""");

	// colors
	py::def("setUseColors", setUseColors, R"""(
Turn on/off colors in log messages. By default is on. If logging to a file then it is better to be turned off.
	)""");

	// config file
	py::def("readConfigFile", readConfigFile, R"""(
Loads the given configuration file.

:param str fname: the config file to be loaded.
	)""");
	py::def("saveConfigFile", saveConfigFile, R"""(
Saves log config to specified file.

:param str fname: the config file to be saved.
	)""");
	py::def("defaultConfigFileName", defaultConfigFileName, R"""(
:return: the default log config file, which is loaded at startup, if it exists.
	)""");

	py::scope().attr("TRACE")    = int(6);
	py::scope().attr("DEBUG")    = int(5);
	py::scope().attr("INFO")     = int(4);
	py::scope().attr("WARN")     = int(3);
	py::scope().attr("ERROR")    = int(2);
	py::scope().attr("FATAL")    = int(1);
	py::scope().attr("NOFILTER") = int(0);

} catch (...) {
	// How to work with python exceptions:
	//     https://www.boost.org/doc/libs/1_71_0/libs/python/doc/html/reference/high_level_components/boost_python_errors_hpp.html#high_level_components.boost_python_errors_hpp.example
	//     https://www.boost.org/doc/libs/1_71_0/libs/python/doc/html/tutorial/tutorial/embedding.html
	//     https://stackoverflow.com/questions/1418015/how-to-get-python-exception-text
	// If we wanted custom yade exceptions thrown to python:
	//     https://www.boost.org/doc/libs/1_71_0/libs/python/doc/html/tutorial/tutorial/exception.html
	//     https://www.boost.org/doc/libs/1_71_0/libs/python/doc/html/reference/high_level_components/boost_python_exception_translato.html
	LOG_FATAL("Importing this module caused an exception and this module is in an inconsistent state now.");
	PyErr_Print();
	PyErr_SetString(PyExc_SystemError, __FILE__);
	boost::python::handle_exception();
	throw;
}

