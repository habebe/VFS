#pragma once
#include <fstream>

class Logger {
private:
	static Logger* s_instance;
	


public:
	enum Type {
		DEBUG,
		INFO,
		WARN,
		ERROR
	};
	
	static Logger* getInstance();
	static Logger* createInstance(const char* filename);
	static Logger* createInstance();
	static void close();

	void log(const char* buffer);
	static void Log(Type type,const char* fmt,...);


private:
	FILE * m_file;

};