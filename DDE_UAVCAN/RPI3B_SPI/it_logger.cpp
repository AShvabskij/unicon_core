#include <cctype>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <sstream>
#include <strings.h>

#include "it_logger.h"

// All the code in this anonymous namespace block is executed once and only once
// during application startup. This is a "silent" new for Logging to make sure
// it is properly configured and sane.
//
namespace
{
	// Check to see where logging output will go. Default is standard out.
	//
	// If the user defines the environmental variable ITLOGFILE, then that
	// will be the logging output. The file will be opened relative to the
	// home directory of the user running the application. For example, if
	// the user, using bash, performs 'export ITLOGFILE=foo.txt', then runs
	// the application, all logging will go to $HOME/foo.txt.
	//
	// Failures to open are silent, and the default on failure is standard out.
	//
	std::ostream* checkLoggingOutput(void) {
		std::ostream *logout = &std::cout;
		std::stringstream logfilename;
		char *home = getenv("HOME");
		char *env = "LOG_RPI3B_SPI.txt";// getenv("ITLOGFILE");

		if ((home != NULL) && (env != NULL)) {
			logfilename << home << "/" << env;
			std::ofstream *ofs =
				new std::ofstream(
					logfilename.str().c_str(), std::ofstream::out | std::ofstream::app);

			if (ofs != NULL && ofs->is_open()) {
				logout = ofs;
			}
		}

		return logout;
	}

	std::ostream *outPtr = checkLoggingOutput();

	// Check what logging level to use. Default is IT::Logger::ERROR.
	//
	// If the user defines the environmental variable ITLOGLEVEL, then that
	// will be the logging output. Levels are INFO, ERROR, and FATAL.
	// Logging level is set, using bash as an example,
	// by 'export ITLOGLEVEL=INFO' (if you want INFO level logging or higher).
	// The three levels are case insensitive (info is the same as INFO, etc).
	//
	// Failures due to misspellings are silent. Default level is ERROR.
	//
	const IT::Logger::Level checkLoggingLevel(void) {
		IT::Logger::Level level = IT::Logger::ERROR;
		char *env = getenv("ITLOGLEVEL");

		if (env != NULL) {
			if (strcasecmp(env, "INFO") == 0) {
				level = IT::Logger::INFO;
			}
			else if (strcasecmp(env, "ERROR") == 0) {
				level = IT::Logger::ERROR;
			}
			else if (strcasecmp(env, "FATAL") == 0) {
				level = IT::Logger::FATAL;
			}
		}

		return level;
	}

	IT::Logger::Level levelFilter = checkLoggingLevel();

	std::ostream &setStream(std::ostream *ostr) {
		outPtr = ostr != NULL ? ostr : &std::cout;
		return *ostr;
	}

	static const char* lname[] = {
		"-  - INFO  : ",
		"!!!! ERROR : ",
		"**** FATAL : "
	};

	const char* levelToString(const IT::Logger::Level level) {
		return (level >= 0 && level < sizeof(lname)) ? lname[level] : "UNKNOWN: ";
	}
}
// end anonymous namespace

IT::Logger::Logger(std::ostream &ostr) {
	setStream(&ostr);
}

std::ostream &IT::Logger::setStream(std::ostream *ostr) {
	return setStream(ostr);
}

void IT::Logger::setLevel(const IT::Logger::Level level) {
	::levelFilter = level;
}

// A LOG macro is wrapped around this specific function call.
// The level is one of the three IT::Logger::Levels defined in the header.
// The filename is the source file name in which this was invoked via the macro.
// The line number is the source file line number, i.e. where you
// would expect to find when you open up the source file in a text editor.
// The message is the explicit message written with the macro.
// Logging is timestamped with the system's current local time.

void IT::Logger::log(
	const Level level,
	const std::string filename,
	const int line,
	const std::string message) {

	if (level >= ::levelFilter) {
		char atime[80];
		time_t rawtime = time(NULL);
		tm *curtime = localtime(&rawtime);
		strftime(atime, sizeof(atime), "%c %Z : ", curtime);
		*outPtr << levelToString(level) << atime;
		*outPtr << filename << " : " << line << " : " << message << std::endl;
	}
}