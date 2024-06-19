#include "DatabaseManager.h"
#include <iostream>

DatabaseManager::DatabaseManager() : driver(nullptr), con(nullptr) {}

DatabaseManager::DatabaseManager(const std::string& server, const std::string& username, const std::string& password, const std::string& db) {
    try {
        driver = get_driver_instance();
        con = driver->connect(server, username, password);
        con->setSchema(db);
    }
    catch (sql::SQLException& e) {
        std::cerr << "Wyj¹tek SQL: " << e.what() << std::endl;
        std::cerr << "Kod b³êdu MySQL: " << e.getErrorCode() << std::endl;
        std::cerr << "SQLState: " << e.getSQLState() << std::endl;
    }
    catch (std::exception& e) {
        std::cerr << "Standardowy wyj¹tek: " << e.what() << std::endl;
    }
    catch (...) {
        std::cerr << "Z³apano nieznany wyj¹tek" << std::endl;
    }
}

DatabaseManager::~DatabaseManager() {
    delete con;
}

sql::Connection* DatabaseManager::getConnection() {
    return con;
}
