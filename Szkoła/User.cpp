#include "User.h"

void pressEnterToContinue() {
    std::cout << "\n\nNaci�nij Enter, aby kontynuowa�...";
    std::cin.ignore();
    std::cin.get();
}

void clearConsole() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

User::User(sql::Connection* connection) : con(connection) {}
