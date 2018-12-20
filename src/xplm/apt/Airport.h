#pragma once

#include <map>
#include <vector>
#include <sstream>
#include "Log.h"
#include <sqlite/sqlite3.h>

class AirportData {
public:
	AirportData():code(0) {}

	AirportData(const std::string _airportCode,
		int _code,
		const std::string& _data) :
		airportCode(_airportCode), code(_code), data(_data)
	{
	}

	AirportData(AirportData* parent,
		int _code,
		const std::string& _data) :
		code(_code), data(_data)
	{
		if (parent) {
			parent->children.push_back(this);
			airportCode = parent->airportCode;
		}
	}
	void insertStatement(std::stringstream& stream, std::stringstream* datastream);
	~AirportData() {
		for (std::vector<AirportData*>::iterator i = children.begin(); i != children.end(); i++) {
			delete (*i);
		}
	}
	void log(int level = 0) {
		std::stringstream stream;
		for (int i = 0; i < level; i++)
			stream << "\t";

		stream << "AirportItem [" << airportCode << "] " << " code=" << code << " data=" << data;
		std::string s = stream.str();
		Logger::Log(Logger::DEBUG, s.c_str());
		for (std::vector<AirportData*>::iterator i = children.begin(); i != children.end(); i++) {
			(*i)->log(level + 1);
		}
	}
	std::string airportCode;
	int code;
	std::string data;
	std::vector<AirportData*> children;
};

class AirportStorage {
public:
	AirportStorage();
	~AirportStorage();
	bool initialize(const std::string& name);
	bool close();
	int persist(const std::string& filename);
	AirportData* find(const std::string& code);
public:
	static AirportData* build(const std::string& content);
private:
	bool persist(AirportData* data);
	sqlite3_stmt* getInsertStatement();
private:
	sqlite3* m_db;
	sqlite3_stmt* m_insertStatement;

};