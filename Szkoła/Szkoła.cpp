#include <iostream>
#include <locale.h>
#include "DatabaseManager.h"
#include "User.h"
#include "Teacher.h"
#include "Admin.h"

int main() {
    setlocale(LC_CTYPE, "polish");

    const std::string server = "tcp://127.0.0.1:3306";
    const std::string username = "root";
    const std::string password = "";
    const std::string db = "schoolcrm";

    DatabaseManager dbManager(server, username, password, db);
    sql::Connection* con = dbManager.getConnection();

    if (con) {
        int userType;
        do {
            clearConsole();
            std::cout << "\n--- System Zarządzania Szkołą ---\n";
            std::cout << "1. Zaloguj jako nauczyciel\n";
            std::cout << "2. Zaloguj jako administrator\n";
            std::cout << "3. Wyjdź\n";
            std::cout << "Twój wybór: ";
            std::cin >> userType;

            switch (userType) {
            case 1: {
                Teacher teacher(con);
                if (teacher.login()) {
                    teacher.teacherMenu();
                }
                break;
            }
            case 2: {
                Admin admin(con);
                if (admin.login()) {
                    admin.adminMenu();
                }
                break;
            }
            case 3:
                std::cout << "Zamykanie programu..." << std::endl;
                break;
            default:
                std::cout << "Nieprawidłowy wybór. Spróbuj ponownie." << std::endl;
            }
        } while (userType != 3);
    }
    else {
        std::cout << "Nie udało się połączyć z bazą danych." << std::endl;
    }

    return 0;
}