#ifndef USER_H
#define USER_H

#include <mysql_connection.h>
#include <cppconn/prepared_statement.h>
#include <string>
#include <iostream>

void pressEnterToContinue();
void clearConsole();

class User {
protected:
    sql::Connection* con;

public:
    User(sql::Connection* connection);
    virtual bool login() = 0;
};

#endif
