#include <string>
#include <iostream>
#include "Log.h"
#include "Airport.h"

//PATH=%PATH%;D:\dev\projects\xplane-dev\VFS\external\SDL2-2.0.9\lib\x64;D:\dev\projects\xplane-dev\VFS\external\glew-2.1.0\bin\Release\x64

int main(int argc, char** argv) {
	Logger* logger = Logger::createInstance();
	AirportStorage storage;
	storage.initialize("D:\\dev\\projects\\xplane-dev\\apt.db");
	//storage.persist("D:\\dev\\projects\\xplane-dev\\apt.dat");
	
	
	AirportData* data = storage.find("KTKI");
	if (data) {
		data->log();
		delete data;
	}
	data = storage.find("KADS");
	if (data) {
		//data->log();
		delete data;
	}
	storage.close();

	Logger::close();
	return 0;
}