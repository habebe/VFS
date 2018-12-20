#include "Log.h"
#include <ctime>
#include <cstdarg>
#include <vector>
#include <iostream>
Logger* Logger::s_instance = nullptr;

Logger* Logger::getInstance() {
	return s_instance;
}

Logger* Logger::createInstance(const char* filename) {
	if (s_instance == nullptr) {
		s_instance = new Logger();
		s_instance->m_file = fopen(filename,"w");
	}
	return s_instance;
}

Logger* Logger::createInstance() {
	if (s_instance == nullptr) {
		s_instance = new Logger();
		s_instance->m_file = stdout;
	}
	return s_instance;
}

void Logger::close() {
	if (s_instance != nullptr) {
		if (s_instance->m_file != stdout) {
			fclose(s_instance->m_file);
		}
		delete s_instance;
		s_instance = nullptr;
	}
}

void Logger::Log(Type type, const char* fmt, ...) {
	static char* TYPE[] = {"DEBUG","INFO","WARN","ERROR"};
	if (s_instance) {
		std::time_t t = std::time(0);
		std::tm* now = std::localtime(&t);
		static char timeBuffer[64];
		static char buffer[4096];
		std::strftime(timeBuffer, sizeof timeBuffer, "%D %T", std::gmtime(&t));
		sprintf(buffer,"%s %s ", TYPE[type],timeBuffer);
		va_list args;
		va_start(args, fmt);
		vsprintf(buffer + strlen(buffer), fmt, args);
		va_end(args);
		s_instance->log(buffer);
	
	}
}

void Logger::log(const char* buffer) {
	fprintf(m_file,buffer);
	fprintf(m_file, "\n");

/*
	std::time_t t = std::time(0);
	std::tm* now = std::localtime(&t);
	char time_buf[100];
	std::strftime(time_buf, sizeof time_buf, "%D %T", std::gmtime(&t));
	Type type = DEBUG;
	switch (type) {
	case DEBUG:
		fprintf(m_file,"DEBUG [%s] ", time_buf);
		break;
	case INFO:
		fprintf(m_file, "INFO [%d:%d:%d] ", now->tm_hour, now->tm_min, now->tm_sec);
		break;
	case WARN:
		fprintf(m_file, "WARN [%d:%d:%d] ", now->tm_hour, now->tm_min, now->tm_sec);
		break;
	case ERROR:
		fprintf(m_file, "ERROR [%d:%d:%d] ", now->tm_hour, now->tm_min, now->tm_sec);
		break;
	}
	std::cerr << fmt << "\n";
	char buffer[1024];
	va_list args;
	va_start(args, fmt);
	vsprintf(buffer, fmt, args);
	va_end(args);

	std::cerr << buffer << "\n";

	

	va_start(args, fmt);
	
	//vfprintf(m_file, fmt, args);
	va_end(args);

	fprintf(m_file, "\n");
	*/
}

