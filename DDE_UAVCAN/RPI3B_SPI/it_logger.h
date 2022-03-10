#pragma once
#ifndef IT_LOGGER_H
#define IT_LOGGER_H


#include <iostream>

namespace IT
{
	class Logger {
	public:
		enum Level {
			INFO,   // Everything.
			ERROR,  // An error in the code itself.
			FATAL   // What is caused by a runtime exception.
		};

		// Set Logger to an output stream. Default is std::cout.
		//
		explicit Logger(std::ostream &ostr);

		// Set the stream we want to log to. Default is std::cout.
		//
		static std::ostream &setStream(std::ostream *ostr = 0);

		// Set the reporting level. Default is INFO.
		//
		static void setLevel(const IT::Logger::Level = INFO);

		// The logging function. This is wrapped in the LOG macro below.
		//
		static void log(const Level, const std::string fileName, const int line, const std::string msg);
	};
}

// Generate a logger entry. Macros are evil, but this is a necessary evil.
//
#define LOG(level, string) \
    IT::Logger::log((IT::Logger::level), __FILE__, __LINE__, (string));

#endif