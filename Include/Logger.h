/* 
 * This file is part of the QuantServer (https://github.com/geneidy/QuantServer).
 * Copyright (c) 2017 geneidy.
 * 
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
// Logger.h
#pragma once

#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <mutex>

// Definition of a multithread safe singleton logger class
class Logger
{
public:
	static const std::string Debug;
	static const std::string Info;
	static const std::string Error;

	// Returns a reference to the singleton Logger object
	static Logger& instance();

	// Logs a single message at the given log level
	void log(const std::string& inMessage, 
		const std::string& inLogLevel);

	// Logs a vector of messages at the given log level
	void log(const std::vector<std::string>& inMessages, 
		const std::string& inLogLevel);

protected:
	// Static variable for the one-and-only instance  
	static Logger* pInstance;

	// Constant for the filename
	static const char* const  kLogFileName;

	// Data member for the output stream
	std::fstream mOutputStream;

	// Embedded class to make sure the single Logger
	// instance gets deleted on program shutdown.
	friend class Cleanup;
	class Cleanup
	{
	public:
		~Cleanup();
	};

	// Logs message. The thread should own a lock on sMutex
	// before calling this function.
	void logHelper(const std::string& inMessage, 
		const std::string& inLogLevel);

private:
	Logger();
	virtual ~Logger();
	Logger(const Logger&);
	Logger& operator=(const Logger&);
	static std::mutex sMutex;
};


