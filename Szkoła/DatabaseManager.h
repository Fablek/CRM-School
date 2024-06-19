#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <mysql_connection.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <string>

class DatabaseManager {
private:
    sql::Driver* driver;
    sql::Connection* con;

public:
    DatabaseManager();
    DatabaseManager(const std::string& server, const std::string& username, const std::string& password, const std::string& db);
    ~DatabaseManager();
    sql::Connection* getConnection();
};

#endif
