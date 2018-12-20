#include "Airport.h"
#include <fstream>
#include "Log.h"
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cassert>
#include <iostream>


AirportStorage::AirportStorage() {
	m_db = nullptr;
	m_insertStatement = nullptr;
}

AirportStorage::~AirportStorage() {
	this->close();
}



bool AirportStorage::initialize(const std::string& name)
{
	int status = sqlite3_initialize();
	if (status == SQLITE_OK) {
		status = sqlite3_open_v2(name.c_str(), &m_db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
		if (status == 0) {
			static char* createStatement = "CREATE TABLE IF NOT EXISTS APT_DATA("\
				"APT_CODE TEXT PRIMARY KEY NOT NULL,"\
				"DATA BLOB"\
				")";
			char* errorMessage = nullptr;
			status = sqlite3_exec(m_db, createStatement, nullptr, 0, &errorMessage);
			if (status != SQLITE_OK) {
				Logger::Log(Logger::ERROR, errorMessage);
				sqlite3_free(errorMessage);
			}
		}
		else if (m_db) {
			Logger::Log(Logger::ERROR, sqlite3_errmsg(m_db));
		}
	}
	else {
		Logger::Log(Logger::ERROR, "Error Initializing DB");
	}
	return (status == SQLITE_OK);
}

void AirportData::insertStatement(std::stringstream& stream, std::stringstream* datastream) {
	if (datastream == nullptr) {
		datastream = new std::stringstream();
		(*datastream) << data << "\n";
		for (std::vector<AirportData*>::iterator i = children.begin(); i != children.end(); i++) {
			(*i)->insertStatement(stream, datastream);
		}
		std::string _data = datastream->str();
		/*stream << "INSERT OR REPLACE INTO APT_DATA VALUES ('"
			<< airportCode.c_str()
			<< "','"
			<< _data.c_str()
			<< "');\n";
			*/
		stream << _data;
		delete datastream;
	}
	else {
		(*datastream) << data << "\n";
		for (std::vector<AirportData*>::iterator i = children.begin(); i != children.end(); i++) {
			(*i)->insertStatement(stream, datastream);
		}
	}
}

sqlite3_stmt* AirportStorage::getInsertStatement() {
	if (m_insertStatement == nullptr) {
		int status = sqlite3_prepare(m_db, "INSERT OR REPLACE INTO APT_DATA VALUES (?,?)", -1, &m_insertStatement, 0);
		if (status != SQLITE_OK) {
			Logger::Log(Logger::ERROR, "Error setting insert statements.");
		}
	}
	return m_insertStatement;
}

bool AirportStorage::persist(AirportData* data) {
	int status;
	bool result = false;
	if (m_db) {
		std::stringstream stream;
		data->insertStatement(stream, nullptr);
		std::string statement = stream.str();
		sqlite3_stmt * stmt = this->getInsertStatement();
		if (stmt) {
			if (status == SQLITE_OK)
				status = sqlite3_bind_text(stmt, 1, data->airportCode.c_str(), data->airportCode.length(), SQLITE_STATIC);
			if (status == SQLITE_OK)
				status = sqlite3_bind_blob(stmt, 2, statement.c_str(), statement.length(), SQLITE_STATIC);
			if (status != SQLITE_OK)
				Logger::Log(Logger::ERROR, "Error setting insert statements.");
			status = sqlite3_step(stmt);
			if (status != SQLITE_DONE) {
				Logger::Log(Logger::ERROR, "Error inserting");
				result = false;
			}
			else {
				result = true;
			}
		}
	}
	return result;
}

bool AirportStorage::close() {
	if (m_db) {
		if (m_insertStatement) {
			sqlite3_finalize(m_insertStatement);
			m_insertStatement = nullptr;
		}
		sqlite3_close(m_db);
		m_db = nullptr;
	}
	return false;
}


static std::istream& safeGetline(std::istream &is, std::string &t);

#define IS_SPACE(x) (((x) == ' ') || ((x) == '\t'))

static inline void ltrim(std::string &s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
		return !IS_SPACE(ch);
	}));
}

static inline void rtrim(std::string &s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
		return !IS_SPACE(ch);
	}).base(), s.end());
}

static inline void trim(std::string &s) {
	ltrim(s);
	rtrim(s);
}

static bool isNumber(const std::string& s)
{
	std::string::const_iterator it = s.begin();
	while (it != s.end() && std::isdigit(*it)) ++it;
	return !s.empty() && it == s.end();
}

static int tokenize(const std::string& line, std::vector<std::string>& tokens) {
	std::stringstream stream(line);
	std::string token;
	tokens.clear();
	while (std::getline(stream, token, ' ')) {
		ltrim(token);
		if (token.empty() == false) {
			tokens.push_back(token);
		}
	}
	return (tokens.size());
}

static int selectCallback(void* _data, int argc, char **argv, char **azColName) {
	AirportData** data = (AirportData**)_data;
	if (argc == 1) {
		std::string line = argv[0];
		(*data) = AirportStorage::build(line);
	}
	return 0;
}

AirportData* AirportStorage::find(const std::string& code) {
	AirportData* data = nullptr;
	if (m_db) {
		char statement[512];
		char* errorMessage = nullptr;
		sprintf(statement, "SELECT DATA FROM APT_DATA where APT_CODE='%s'", code.c_str());
		int status = sqlite3_exec(m_db, statement, selectCallback, &data, &errorMessage);
		if (status != SQLITE_OK) {
			Logger::Log(Logger::ERROR, errorMessage);
			sqlite3_free(errorMessage);
		}
	}

	return data;
}

AirportData* AirportStorage::build(const std::string& content) {
	int lineNumber = 0;
	bool status = false;
	AirportData * currentAirport = nullptr;
	AirportData* currentTaxiway = nullptr;
	std::stringstream stream(content);

	std::string line;
	bool done = stream.peek() == -1;
	std::vector<std::string> tokens;

	while (!done) {
		safeGetline(stream, line);
		lineNumber++;
		trim(line);
		if (line.empty() == false) {
			std::size_t found = line.find_first_of(" \t");
			if (found < line.length()) {
				std::string stringCode = line.substr(0, found);
				int code = std::stoi(stringCode);
				switch (code) {
				case 1:
				case 16:
				case 17:
				{
					std::string airportCode;
					tokenize(line, tokens);
					if (tokens.size() >= 5) {
						airportCode = tokens[4];
						currentTaxiway = currentAirport = new AirportData(airportCode, code, line);
						Logger::Log(Logger::DEBUG, "%d code=%d acode=%s", lineNumber, code, airportCode.c_str());
					}
				}
				break;
				case 110:
				{
					assert(currentAirport != nullptr);
					currentTaxiway = new AirportData(currentAirport, code, line);
				}
				break;
				case 111:
				case 112:
				case 113:
				case 114:
				case 115:
				case 116:
				{
					if (currentTaxiway == nullptr) {
						Logger::Log(Logger::DEBUG, "%d code=%d data=%s", lineNumber, code, line.c_str());
					}
					assert(currentTaxiway != nullptr);
					new AirportData(currentTaxiway, code, line);
				}
				break;
				default:
				{
					new AirportData(currentAirport, code, line);
				}
				break;
				}
			}
		}
		done = stream.peek() == -1; // || (lineNumber > 100000);
	}
	return currentAirport;
}


int AirportStorage::persist(const std::string& filename) {
	int lineNumber = 0;
	bool status = false;
	std::ifstream stream(filename.c_str());
	AirportData * currentAirport = nullptr;
	AirportData* currentTaxiway = nullptr;
	if (stream.is_open())
	{
		std::string line;
		bool done = stream.peek() == -1;
		std::vector<std::string> tokens;

		while (!done) {
			safeGetline(stream, line);
			lineNumber++;
			trim(line);
			if (line.empty() == false) {
				std::size_t found = line.find_first_of(" \t");
				if (found < line.length()) {
					std::string stringCode = line.substr(0, found);
					int code = std::stoi(stringCode);
					switch (code) {
					case 1:
					case 16:
					case 17:
					{
						std::string airportCode;
						tokenize(line, tokens);
						if (tokens.size() >= 5) {
							airportCode = tokens[4];
							if (currentAirport) {
								this->persist(currentAirport);
								delete currentAirport;
							}
							currentTaxiway = currentAirport = new AirportData(airportCode, code, line);
							Logger::Log(Logger::DEBUG, "%d code=%d acode=%s", lineNumber, code, airportCode.c_str());
						}
					}
					break;
					case 110:
					{
						assert(currentAirport != nullptr);
						currentTaxiway = new AirportData(currentAirport, code, line);
					}
					break;
					case 111:
					case 112:
					case 113:
					case 114:
					case 115:
					case 116:
					{
						if (currentTaxiway == nullptr) {
							Logger::Log(Logger::DEBUG, "%d code=%d data=%s", lineNumber, code, line.c_str());
						}
						assert(currentTaxiway != nullptr);
						new AirportData(currentTaxiway, code, line);
					}
					break;
					default:
					{
						AirportData* data = new AirportData(currentAirport, code, line);
					}
					break;
					}
				}
			}
			done = stream.peek() == -1;
		}
	}
	if (currentAirport) {
		this->persist(currentAirport);
		delete currentAirport;
	}
	return lineNumber;
}


std::string join(const std::vector<std::string>& tokens, int startIndex) {
	std::stringstream stream;
	for (int i = startIndex; i < tokens.size(); i++) {
		if (i > startIndex) {
			stream << " ";
		}
		stream << tokens[i];
	}
	return stream.str();
}


static std::istream& safeGetline(std::istream &is, std::string &t) {
	t.clear();
	std::istream::sentry se(is, true);
	std::streambuf *sb = is.rdbuf();

	if (se) {
		for (;;) {
			int c = sb->sbumpc();
			switch (c) {
			case '\n':
				return is;
			case '\r':
				if (sb->sgetc() == '\n') sb->sbumpc();
				return is;
			case EOF:
				if (t.empty()) is.setstate(std::ios::eofbit);
				return is;
			default:
				t += static_cast<char>(c);
			}
		}
	}

	return is;
}