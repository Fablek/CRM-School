#include "Admin.h"

Admin::Admin(sql::Connection* connection) : User(connection) {}

bool Admin::login() {
    clearConsole();

    std::string adminUsername, adminPassword;
    std::cout << "Wpisz swoj¹ nazwê u¿ytkownika: ";
    std::cin >> adminUsername;
    std::cout << "Podaj has³o: ";
    std::cin >> adminPassword;

    try {
        sql::PreparedStatement* pstmt;
        pstmt = con->prepareStatement("SELECT id FROM Admin WHERE username=? AND password=?");
        pstmt->setString(1, adminUsername);
        pstmt->setString(2, adminPassword);
        sql::ResultSet* res = pstmt->executeQuery();

        if (res->next()) {
            std::cout << "Pomyœlnie zalogowano jako administrator." << std::endl;
            delete pstmt;
            delete res;
            pressEnterToContinue();
            clearConsole();
            return true;
        }
        else {
            std::cout << "Logowanie nie powiod³o siê. Niepoprawna nazwa u¿ytkownika lub has³o." << std::endl;
            delete pstmt;
            delete res;
            pressEnterToContinue();
            clearConsole();
            return false;
        }
    }
    catch (sql::SQLException& e) {
        std::cout << "B³¹d SQL: " << e.what() << std::endl;
        return false;
    }
}

void Admin::addTeacher() {
    clearConsole();

    std::string teacherName, teacherSurname, teacherDob, teacherPesel, teacherAddress, teacherPhone, teacherEmail, teacherUsername, teacherPassword;
    std::cout << "Wpisz imiê nauczyciela: ";
    std::cin >> teacherName;
    std::cout << "Wpisz nazwisko nauczyciela: ";
    std::cin >> teacherSurname;
    std::cout << "Wpisz datê urodzenia nauczyciela (RRRR-MM-DD): ";
    std::cin >> teacherDob;
    std::cout << "Wpisz PESEL nauczyciela: ";
    std::cin >> teacherPesel;
    std::cout << "Wpisz adres nauczyciela: ";
    std::cin.ignore();
    std::getline(std::cin, teacherAddress);
    std::cout << "Wpisz telefon nauczyciela: ";
    std::cin >> teacherPhone;
    std::cout << "Wpisz email nauczyciela: ";
    std::cin >> teacherEmail;
    std::cout << "Wpisz nazwê u¿ytkownika nauczyciela: ";
    std::cin >> teacherUsername;
    std::cout << "Wpisz has³o nauczyciela: ";
    std::cin >> teacherPassword;

    try {
        con->setAutoCommit(false); // Rozpoczyna transakcjê
        sql::PreparedStatement* pstmt = con->prepareStatement("INSERT INTO osoby (imie, nazwisko, data_urodzenia, pesel, adres, telefon, email) VALUES (?, ?, ?, ?, ?, ?, ?)");
        pstmt->setString(1, teacherName);
        pstmt->setString(2, teacherSurname);
        pstmt->setString(3, teacherDob);
        pstmt->setString(4, teacherPesel);
        pstmt->setString(5, teacherAddress);
        pstmt->setString(6, teacherPhone);
        pstmt->setString(7, teacherEmail);
        pstmt->executeUpdate();
        delete pstmt;

        pstmt = con->prepareStatement("SELECT id FROM osoby WHERE pesel=?");
        pstmt->setString(1, teacherPesel);
        sql::ResultSet* res = pstmt->executeQuery();

        int teacherId = 0;
        if (res->next()) {
            teacherId = res->getInt("id");
        }
        delete pstmt;
        delete res;

        if (teacherId > 0) {
            pstmt = con->prepareStatement("INSERT INTO nauczyciele (id, username, password) VALUES (?, ?, ?)");
            pstmt->setInt(1, teacherId);
            pstmt->setString(2, teacherUsername);
            pstmt->setString(3, teacherPassword);
            pstmt->executeUpdate();
            delete pstmt;

            con->commit(); // Zatwierdza transakcjê
            std::cout << "Nauczyciel zosta³ dodany pomyœlnie." << std::endl;
        }
        else {
            std::cout << "Nie znaleziono osoby o podanym PESEL w bazie danych." << std::endl;
            con->rollback(); // Wycofuje transakcjê
        }
    }
    catch (sql::SQLException& e) {
        std::cout << "B³¹d SQL: " << e.what() << std::endl;
        con->rollback(); // Wycofuje transakcjê w przypadku b³êdu
    }
    clearConsole();
}

void Admin::editTeacher() {
    clearConsole();

    try {
        sql::PreparedStatement* pstmt = con->prepareStatement("SELECT id, username FROM Nauczyciele");
        sql::ResultSet* res = pstmt->executeQuery();

        std::map<int, std::string> teachers;
        int teacherCounter = 1;
        std::cout << "Dostêpni nauczyciele:" << std::endl;
        while (res->next()) {
            int teacherId = res->getInt("id");
            std::string teacherUsername = res->getString("username");
            teachers[teacherCounter] = teacherUsername;
            std::cout << teacherCounter << ". " << teacherUsername << std::endl;
            teacherCounter++;
        }
        delete res;

        if (teachers.empty()) {
            std::cout << "Brak dostêpnych nauczycieli." << std::endl;
            delete pstmt;
            return;
        }

        int teacherChoice;
        std::cout << "Wybierz numer nauczyciela do edycji: ";
        std::cin >> teacherChoice;

        if (teachers.find(teacherChoice) != teachers.end()) {
            std::string teacherUsername = teachers[teacherChoice];

            int teacherId;
            pstmt = con->prepareStatement("SELECT id FROM Nauczyciele WHERE username = ?");
            pstmt->setString(1, teacherUsername);
            res = pstmt->executeQuery();
            if (res->next()) {
                teacherId = res->getInt("id");

                std::string teacherName, teacherSurname, teacherDob, teacherPesel, teacherAddress, teacherPhone, teacherEmail, newTeacherUsername, newTeacherPassword;
                std::cout << "Wpisz nowe imiê nauczyciela (pozostaw puste, aby nie zmieniaæ): ";
                std::cin.ignore();
                std::getline(std::cin, teacherName);
                std::cout << "Wpisz nowe nazwisko nauczyciela (pozostaw puste, aby nie zmieniaæ): ";
                std::getline(std::cin, teacherSurname);
                std::cout << "Wpisz now¹ datê urodzenia nauczyciela (pozostaw puste, aby nie zmieniaæ): ";
                std::getline(std::cin, teacherDob);
                std::cout << "Wpisz nowy PESEL nauczyciela (pozostaw puste, aby nie zmieniaæ): ";
                std::getline(std::cin, teacherPesel);
                std::cout << "Wpisz nowy adres nauczyciela (pozostaw puste, aby nie zmieniaæ): ";
                std::getline(std::cin, teacherAddress);
                std::cout << "Wpisz nowy telefon nauczyciela (pozostaw puste, aby nie zmieniaæ): ";
                std::getline(std::cin, teacherPhone);
                std::cout << "Wpisz nowy email nauczyciela (pozostaw puste, aby nie zmieniaæ): ";
                std::getline(std::cin, teacherEmail);
                std::cout << "Wpisz now¹ nazwê u¿ytkownika nauczyciela (pozostaw puste, aby nie zmieniaæ): ";
                std::getline(std::cin, newTeacherUsername);
                std::cout << "Wpisz nowe has³o nauczyciela (pozostaw puste, aby nie zmieniaæ): ";
                std::getline(std::cin, newTeacherPassword);

                if (!teacherName.empty()) {
                    pstmt = con->prepareStatement("UPDATE Osoby SET imie = ? WHERE id = ?");
                    pstmt->setString(1, teacherName);
                    pstmt->setInt(2, teacherId);
                    pstmt->executeUpdate();
                }
                if (!teacherSurname.empty()) {
                    pstmt = con->prepareStatement("UPDATE Osoby SET nazwisko = ? WHERE id = ?");
                    pstmt->setString(1, teacherSurname);
                    pstmt->setInt(2, teacherId);
                    pstmt->executeUpdate();
                }
                if (!teacherDob.empty()) {
                    pstmt = con->prepareStatement("UPDATE Osoby SET data_urodzenia = ? WHERE id = ?");
                    pstmt->setString(1, teacherDob);
                    pstmt->setInt(2, teacherId);
                    pstmt->executeUpdate();
                }
                if (!teacherPesel.empty()) {
                    pstmt = con->prepareStatement("UPDATE Osoby SET pesel = ? WHERE id = ?");
                    pstmt->setString(1, teacherPesel);
                    pstmt->setInt(2, teacherId);
                    pstmt->executeUpdate();
                }
                if (!teacherAddress.empty()) {
                    pstmt = con->prepareStatement("UPDATE Osoby SET adres = ? WHERE id = ?");
                    pstmt->setString(1, teacherAddress);
                    pstmt->setInt(2, teacherId);
                    pstmt->executeUpdate();
                }
                if (!teacherPhone.empty()) {
                    pstmt = con->prepareStatement("UPDATE Osoby SET telefon = ? WHERE id = ?");
                    pstmt->setString(1, teacherPhone);
                    pstmt->setInt(2, teacherId);
                    pstmt->executeUpdate();
                }
                if (!teacherEmail.empty()) {
                    pstmt = con->prepareStatement("UPDATE Osoby SET email = ? WHERE id = ?");
                    pstmt->setString(1, teacherEmail);
                    pstmt->setInt(2, teacherId);
                    pstmt->executeUpdate();
                }
                if (!newTeacherUsername.empty()) {
                    pstmt = con->prepareStatement("UPDATE Nauczyciele SET username = ? WHERE id = ?");
                    pstmt->setString(1, newTeacherUsername);
                    pstmt->setInt(2, teacherId);
                    pstmt->executeUpdate();
                }
                if (!newTeacherPassword.empty()) {
                    pstmt = con->prepareStatement("UPDATE Nauczyciele SET password = ? WHERE id = ?");
                    pstmt->setString(1, newTeacherPassword);
                    pstmt->setInt(2, teacherId);
                    pstmt->executeUpdate();
                }

                std::cout << "Dane nauczyciela zosta³y pomyœlnie zaktualizowane." << std::endl;
            }
            else {
                std::cout << "Nauczyciel o podanym u¿ytkowniku nie istnieje." << std::endl;
            }
            delete res;
        }
        else {
            std::cout << "Nieprawid³owy wybór nauczyciela." << std::endl;
        }

        if (pstmt != nullptr) {
            delete pstmt;
        }
    }
    catch (sql::SQLException& e) {
        std::cout << "B³¹d SQL: " << e.what() << std::endl;
    }

    pressEnterToContinue();
    clearConsole();
}

void Admin::removeTeacher() {
    clearConsole();

    try {
        sql::PreparedStatement* pstmt = nullptr;
        sql::ResultSet* res = nullptr;

        pstmt = con->prepareStatement("SELECT id, username FROM Nauczyciele");
        res = pstmt->executeQuery();

        std::map<int, std::string> teachers;
        int teacherCounter = 1;
        std::cout << "Dostêpni nauczyciele:" << std::endl;
        while (res->next()) {
            int teacherId = res->getInt("id");
            std::string teacherUsername = res->getString("username");
            teachers[teacherCounter] = teacherUsername;
            std::cout << teacherCounter << ". " << teacherUsername << std::endl;
            teacherCounter++;
        }
        delete res;

        if (teachers.empty()) {
            std::cout << "Brak dostêpnych nauczycieli." << std::endl;
            delete pstmt;
            return;
        }

        int teacherChoice;
        std::cout << "Wybierz numer nauczyciela do usuniêcia: ";
        std::cin >> teacherChoice;

        if (teachers.find(teacherChoice) == teachers.end()) {
            std::cout << "Nieprawid³owy wybór nauczyciela." << std::endl;
            delete pstmt;
            return;
        }

        std::string teacherUsername = teachers[teacherChoice];

        pstmt = con->prepareStatement("DELETE FROM Nauczyciele WHERE username = ?");
        pstmt->setString(1, teacherUsername);

        int updateCount = pstmt->executeUpdate();

        if (updateCount > 0) {
            std::cout << "Nauczyciel " << teacherUsername << " zosta³ usuniêty." << std::endl;
        }
        else {
            std::cout << "Nie uda³o siê usun¹æ nauczyciela." << std::endl;
        }

        if (pstmt != nullptr) {
            delete pstmt;
        }
    }
    catch (sql::SQLException& e) {
        std::cout << "B³¹d SQL: " << e.what() << std::endl;
    }

    pressEnterToContinue();
    clearConsole();
}

void Admin::addStudent() {
    clearConsole();

    std::string studentName, studentSurname, studentDob, studentPesel, studentAddress, studentPhone, studentEmail, studentUsername, studentPassword;
    std::cout << "Wpisz imiê ucznia: ";
    std::cin >> studentName;
    std::cout << "Wpisz nazwisko ucznia: ";
    std::cin >> studentSurname;
    std::cout << "Wpisz datê urodzenia ucznia (RRRR-MM-DD): ";
    std::cin >> studentDob;
    std::cout << "Wpisz PESEL ucznia: ";
    std::cin >> studentPesel;
    std::cout << "Wpisz adres ucznia: ";
    std::cin.ignore();
    std::getline(std::cin, studentAddress);
    std::cout << "Wpisz telefon ucznia: ";
    std::cin >> studentPhone;
    std::cout << "Wpisz email ucznia: ";
    std::cin >> studentEmail;
    std::cout << "Wpisz nazwê u¿ytkownika ucznia: ";
    std::cin >> studentUsername;
    std::cout << "Wpisz has³o ucznia: ";
    std::cin >> studentPassword;

    try {
        sql::PreparedStatement* pstmt;
        pstmt = con->prepareStatement("INSERT INTO Osoby (imie, nazwisko, data_urodzenia, pesel, adres, telefon, email) VALUES (?, ?, ?, ?, ?, ?, ?)");
        pstmt->setString(1, studentName);
        pstmt->setString(2, studentSurname);
        pstmt->setString(3, studentDob);
        pstmt->setString(4, studentPesel);
        pstmt->setString(5, studentAddress);
        pstmt->setString(6, studentPhone);
        pstmt->setString(7, studentEmail);
        pstmt->executeUpdate();
        delete pstmt;

        pstmt = con->prepareStatement("SELECT id FROM Osoby WHERE pesel=?");
        pstmt->setString(1, studentPesel);
        sql::ResultSet* res = pstmt->executeQuery();

        int studentId;
        if (res->next()) {
            studentId = res->getInt("id");
            pstmt = con->prepareStatement("INSERT INTO Uczniowie (id, username, password) VALUES (?, ?, ?)");
            pstmt->setInt(1, studentId);
            pstmt->setString(2, studentUsername);
            pstmt->setString(3, studentPassword);
            pstmt->executeUpdate();

            std::cout << "Uczeñ zosta³ dodany pomyœlnie." << std::endl;
        }

        delete pstmt;
        delete res;
    }
    catch (sql::SQLException& e) {
        std::cout << "B³¹d SQL: " << e.what() << std::endl;
    }
}

void Admin::editStudent() {
    clearConsole();

    try {
        sql::PreparedStatement* pstmt = con->prepareStatement("SELECT id, username FROM Uczniowie");
        sql::ResultSet* res = pstmt->executeQuery();

        std::map<int, std::string> students;
        int studentCounter = 1;
        std::cout << "Dostêpni uczniowie:" << std::endl;
        while (res->next()) {
            int studentId = res->getInt("id");
            std::string studentUsername = res->getString("username");
            students[studentCounter] = studentUsername;
            std::cout << studentCounter << ". " << studentUsername << std::endl;
            studentCounter++;
        }
        delete res;

        if (students.empty()) {
            std::cout << "Brak dostêpnych uczniów." << std::endl;
            delete pstmt;
            return;
        }

        int studentChoice;
        std::cout << "Wybierz numer ucznia do edycji: ";
        std::cin >> studentChoice;

        if (students.find(studentChoice) != students.end()) {
            std::string studentUsername = students[studentChoice];

            int studentId;
            pstmt = con->prepareStatement("SELECT id FROM Uczniowie WHERE username = ?");
            pstmt->setString(1, studentUsername);
            res = pstmt->executeQuery();
            if (res->next()) {
                studentId = res->getInt("id");

                std::string studentName, studentSurname, studentDob, studentPesel, studentAddress, studentPhone, studentEmail, newStudentUsername, newStudentPassword;
                std::cout << "Wpisz nowe imiê ucznia (pozostaw puste, aby nie zmieniaæ): ";
                std::cin.ignore();
                std::getline(std::cin, studentName);
                std::cout << "Wpisz nowe nazwisko ucznia (pozostaw puste, aby nie zmieniaæ): ";
                std::getline(std::cin, studentSurname);
                std::cout << "Wpisz now¹ datê urodzenia ucznia (pozostaw puste, aby nie zmieniaæ): ";
                std::getline(std::cin, studentDob);
                std::cout << "Wpisz nowy PESEL ucznia (pozostaw puste, aby nie zmieniaæ): ";
                std::getline(std::cin, studentPesel);
                std::cout << "Wpisz nowy adres ucznia (pozostaw puste, aby nie zmieniaæ): ";
                std::getline(std::cin, studentAddress);
                std::cout << "Wpisz nowy telefon ucznia (pozostaw puste, aby nie zmieniaæ): ";
                std::getline(std::cin, studentPhone);
                std::cout << "Wpisz nowy email ucznia (pozostaw puste, aby nie zmieniaæ): ";
                std::getline(std::cin, studentEmail);
                std::cout << "Wpisz now¹ nazwê u¿ytkownika ucznia (pozostaw puste, aby nie zmieniaæ): ";
                std::getline(std::cin, newStudentUsername);
                std::cout << "Wpisz nowe has³o ucznia (pozostaw puste, aby nie zmieniaæ): ";
                std::getline(std::cin, newStudentPassword);

                if (!studentName.empty()) {
                    pstmt = con->prepareStatement("UPDATE Osoby SET imie = ? WHERE id = ?");
                    pstmt->setString(1, studentName);
                    pstmt->setInt(2, studentId);
                    pstmt->executeUpdate();
                }
                if (!studentSurname.empty()) {
                    pstmt = con->prepareStatement("UPDATE Osoby SET nazwisko = ? WHERE id = ?");
                    pstmt->setString(1, studentSurname);
                    pstmt->setInt(2, studentId);
                    pstmt->executeUpdate();
                }
                if (!studentDob.empty()) {
                    pstmt = con->prepareStatement("UPDATE Osoby SET data_urodzenia = ? WHERE id = ?");
                    pstmt->setString(1, studentDob);
                    pstmt->setInt(2, studentId);
                    pstmt->executeUpdate();
                }
                if (!studentPesel.empty()) {
                    pstmt = con->prepareStatement("UPDATE Osoby SET pesel = ? WHERE id = ?");
                    pstmt->setString(1, studentPesel);
                    pstmt->setInt(2, studentId);
                    pstmt->executeUpdate();
                }
                if (!studentAddress.empty()) {
                    pstmt = con->prepareStatement("UPDATE Osoby SET adres = ? WHERE id = ?");
                    pstmt->setString(1, studentAddress);
                    pstmt->setInt(2, studentId);
                    pstmt->executeUpdate();
                }
                if (!studentPhone.empty()) {
                    pstmt = con->prepareStatement("UPDATE Osoby SET telefon = ? WHERE id = ?");
                    pstmt->setString(1, studentPhone);
                    pstmt->setInt(2, studentId);
                    pstmt->executeUpdate();
                }
                if (!studentEmail.empty()) {
                    pstmt = con->prepareStatement("UPDATE Osoby SET email = ? WHERE id = ?");
                    pstmt->setString(1, studentEmail);
                    pstmt->setInt(2, studentId);
                    pstmt->executeUpdate();
                }
                if (!newStudentUsername.empty()) {
                    pstmt = con->prepareStatement("UPDATE Uczniowie SET username = ? WHERE id = ?");
                    pstmt->setString(1, newStudentUsername);
                    pstmt->setInt(2, studentId);
                    pstmt->executeUpdate();
                }
                if (!newStudentPassword.empty()) {
                    pstmt = con->prepareStatement("UPDATE Uczniowie SET password = ? WHERE id = ?");
                    pstmt->setString(1, newStudentPassword);
                    pstmt->setInt(2, studentId);
                    pstmt->executeUpdate();
                }

                std::cout << "Dane ucznia zosta³y pomyœlnie zaktualizowane." << std::endl;
            }
            else {
                std::cout << "Uczeñ o podanym u¿ytkowniku nie istnieje." << std::endl;
            }
            delete res;
        }
        else {
            std::cout << "Nieprawid³owy wybór ucznia." << std::endl;
        }

        if (pstmt != nullptr) {
            delete pstmt;
        }
    }
    catch (sql::SQLException& e) {
        std::cout << "B³¹d SQL: " << e.what() << std::endl;
    }

    pressEnterToContinue();
    clearConsole();
}

void Admin::removeStudent() {
    clearConsole();

    try {
        sql::PreparedStatement* pstmt = nullptr;
        sql::ResultSet* res = nullptr;

        pstmt = con->prepareStatement("SELECT id, username FROM Uczniowie");
        res = pstmt->executeQuery();

        std::map<int, std::string> students;
        int studentCounter = 1;
        std::cout << "Dostêpni uczniowie:" << std::endl;
        while (res->next()) {
            int studentId = res->getInt("id");
            std::string studentUsername = res->getString("username");
            students[studentCounter] = studentUsername;
            std::cout << studentCounter << ". " << studentUsername << std::endl;
            studentCounter++;
        }
        delete res;

        if (students.empty()) {
            std::cout << "Brak dostêpnych uczniów." << std::endl;
            delete pstmt;
            return;
        }

        int studentChoice;
        std::cout << "Wybierz numer ucznia do usuniêcia: ";
        std::cin >> studentChoice;

        if (students.find(studentChoice) == students.end()) {
            std::cout << "Nieprawid³owy wybór ucznia." << std::endl;
            delete pstmt;
            return;
        }

        std::string studentUsername = students[studentChoice];

        pstmt = con->prepareStatement("DELETE FROM Uczniowie WHERE username = ?");
        pstmt->setString(1, studentUsername);

        int updateCount = pstmt->executeUpdate();

        if (updateCount > 0) {
            std::cout << "Uczeñ " << studentUsername << " zosta³ usuniêty." << std::endl;
        }
        else {
            std::cout << "Nie uda³o siê usun¹æ ucznia." << std::endl;
        }

        if (pstmt != nullptr) {
            delete pstmt;
        }
    }
    catch (sql::SQLException& e) {
        std::cout << "B³¹d SQL: " << e.what() << std::endl;
    }

    pressEnterToContinue();
    clearConsole();
}

void Admin::createClass() {
    clearConsole();

    std::string className;
    std::cout << "Wpisz nazwê klasy: ";
    std::cin.ignore();
    std::getline(std::cin, className);

    try {
        sql::PreparedStatement* pstmt;
        pstmt = con->prepareStatement("INSERT INTO Klasy (nazwa) VALUES (?)");
        pstmt->setString(1, className);
        pstmt->executeUpdate();

        std::cout << "Klasa zosta³a dodana pomyœlnie." << std::endl;

        delete pstmt;
    }
    catch (sql::SQLException& e) {
        std::cout << "B³¹d SQL: " << e.what() << std::endl;
    }

    pressEnterToContinue();
    clearConsole();
}

void Admin::editClass() {
    clearConsole();

    try {
        sql::PreparedStatement* pstmt = con->prepareStatement("SELECT id, nazwa FROM Klasy");
        sql::ResultSet* res = pstmt->executeQuery();

        std::map<int, std::string> classes;
        int classCounter = 1;
        std::cout << "Dostêpne klasy:" << std::endl;
        while (res->next()) {
            int classId = res->getInt("id");
            std::string className = res->getString("nazwa");
            classes[classCounter] = className;
            std::cout << classCounter << ". " << className << std::endl;
            classCounter++;
        }
        delete res;

        if (classes.empty()) {
            std::cout << "Brak dostêpnych klas." << std::endl;
            delete pstmt;
            return;
        }

        int classChoice;
        std::cout << "Wybierz numer klasy do edycji: ";
        std::cin >> classChoice;

        if (classes.find(classChoice) != classes.end()) {
            std::string className = classes[classChoice];

            int classId;
            pstmt = con->prepareStatement("SELECT id FROM Klasy WHERE nazwa = ?");
            pstmt->setString(1, className);
            res = pstmt->executeQuery();
            if (res->next()) {
                classId = res->getInt("id");

                std::string newClassName;
                std::cout << "Wpisz now¹ nazwê klasy (pozostaw puste, aby nie zmieniaæ): ";
                std::cin.ignore();
                std::getline(std::cin, newClassName);

                if (!newClassName.empty()) {
                    pstmt = con->prepareStatement("UPDATE Klasy SET nazwa = ? WHERE id = ?");
                    pstmt->setString(1, newClassName);
                    pstmt->setInt(2, classId);
                    pstmt->executeUpdate();

                    std::cout << "Nazwa klasy zosta³a pomyœlnie zaktualizowana." << std::endl;
                }
                else {
                    std::cout << "Brak zmian." << std::endl;
                }
            }
            else {
                std::cout << "Klasa o podanej nazwie nie istnieje." << std::endl;
            }
            delete res;
        }
        else {
            std::cout << "Nieprawid³owy wybór klasy." << std::endl;
        }

        if (pstmt != nullptr) {
            delete pstmt;
        }
    }
    catch (sql::SQLException& e) {
        std::cout << "B³¹d SQL: " << e.what() << std::endl;
    }

    pressEnterToContinue();
    clearConsole();
}

void Admin::removeClass() {
    clearConsole();

    try {
        sql::PreparedStatement* pstmt = nullptr;
        sql::ResultSet* res = nullptr;

        pstmt = con->prepareStatement("SELECT id, nazwa FROM Klasy");
        res = pstmt->executeQuery();

        std::map<int, std::string> classes;
        int classCounter = 1;
        std::cout << "Dostêpne klasy:" << std::endl;
        while (res->next()) {
            int classId = res->getInt("id");
            std::string className = res->getString("nazwa");
            classes[classCounter] = className;
            std::cout << classCounter << ". " << className << std::endl;
            classCounter++;
        }
        delete res;

        if (classes.empty()) {
            std::cout << "Brak dostêpnych klas." << std::endl;
            delete pstmt;
            return;
        }

        int classChoice;
        std::cout << "Wybierz numer klasy do usuniêcia: ";
        std::cin >> classChoice;

        if (classes.find(classChoice) == classes.end()) {
            std::cout << "Nieprawid³owy wybór klasy." << std::endl;
            delete pstmt;
            return;
        }

        std::string className = classes[classChoice];

        pstmt = con->prepareStatement("DELETE FROM Klasy WHERE nazwa = ?");
        pstmt->setString(1, className);

        int updateCount = pstmt->executeUpdate();

        if (updateCount > 0) {
            std::cout << "Klasa " << className << " zosta³a usuniêta." << std::endl;
        }
        else {
            std::cout << "Nie uda³o siê usun¹æ klasy." << std::endl;
        }

        if (pstmt != nullptr) {
            delete pstmt;
        }
    }
    catch (sql::SQLException& e) {
        std::cout << "B³¹d SQL: " << e.what() << std::endl;
    }

    pressEnterToContinue();
    clearConsole();
}

void Admin::addSubject() {
    clearConsole();

    std::string subjectName;
    std::cout << "Wpisz nazwê przedmiotu: ";
    std::cin.ignore();
    std::getline(std::cin, subjectName);

    try {
        sql::PreparedStatement* pstmt;
        pstmt = con->prepareStatement("INSERT INTO Przedmioty (nazwa) VALUES (?)");
        pstmt->setString(1, subjectName);
        pstmt->executeUpdate();

        std::cout << "Przedmiot zosta³ dodany pomyœlnie." << std::endl;

        delete pstmt;
    }
    catch (sql::SQLException& e) {
        std::cout << "B³¹d SQL: " << e.what() << std::endl;
    }

    pressEnterToContinue();
    clearConsole();
}

void Admin::editSubject() {
    clearConsole();

    try {
        sql::PreparedStatement* pstmt = con->prepareStatement("SELECT id, nazwa FROM Przedmioty");
        sql::ResultSet* res = pstmt->executeQuery();

        std::map<int, std::string> subjects;
        int subjectCounter = 1;
        std::cout << "Dostêpne przedmioty:" << std::endl;
        while (res->next()) {
            int subjectId = res->getInt("id");
            std::string subjectName = res->getString("nazwa");
            subjects[subjectCounter] = subjectName;
            std::cout << subjectCounter << ". " << subjectName << std::endl;
            subjectCounter++;
        }
        delete res;

        if (subjects.empty()) {
            std::cout << "Brak dostêpnych przedmiotów." << std::endl;
            delete pstmt;
            return;
        }

        int subjectChoice;
        std::cout << "Wybierz numer przedmiotu do edycji: ";
        std::cin >> subjectChoice;

        if (subjects.find(subjectChoice) != subjects.end()) {
            std::string subjectName = subjects[subjectChoice];

            int subjectId;
            pstmt = con->prepareStatement("SELECT id FROM Przedmioty WHERE nazwa = ?");
            pstmt->setString(1, subjectName);
            res = pstmt->executeQuery();
            if (res->next()) {
                subjectId = res->getInt("id");

                std::string newSubjectName;
                std::cout << "Wpisz now¹ nazwê przedmiotu (pozostaw puste, aby nie zmieniaæ): ";
                std::cin.ignore();
                std::getline(std::cin, newSubjectName);

                if (!newSubjectName.empty()) {
                    pstmt = con->prepareStatement("UPDATE Przedmioty SET nazwa = ? WHERE id = ?");
                    pstmt->setString(1, newSubjectName);
                    pstmt->setInt(2, subjectId);
                    pstmt->executeUpdate();

                    std::cout << "Nazwa przedmiotu zosta³a pomyœlnie zaktualizowana." << std::endl;
                }
                else {
                    std::cout << "Brak zmian." << std::endl;
                }
            }
            else {
                std::cout << "Przedmiot o podanej nazwie nie istnieje." << std::endl;
            }
            delete res;
        }
        else {
            std::cout << "Nieprawid³owy wybór przedmiotu." << std::endl;
        }

        if (pstmt != nullptr) {
            delete pstmt;
        }
    }
    catch (sql::SQLException& e) {
        std::cout << "B³¹d SQL: " << e.what() << std::endl;
    }

    pressEnterToContinue();
    clearConsole();
}

void Admin::removeSubject() {
    clearConsole();

    try {
        sql::PreparedStatement* pstmt = nullptr;
        sql::ResultSet* res = nullptr;

        pstmt = con->prepareStatement("SELECT id, nazwa FROM Przedmioty");
        res = pstmt->executeQuery();

        std::map<int, std::string> subjects;
        int subjectCounter = 1;
        std::cout << "Dostêpne przedmioty:" << std::endl;
        while (res->next()) {
            int subjectId = res->getInt("id");
            std::string subjectName = res->getString("nazwa");
            subjects[subjectCounter] = subjectName;
            std::cout << subjectCounter << ". " << subjectName << std::endl;
            subjectCounter++;
        }
        delete res;

        if (subjects.empty()) {
            std::cout << "Brak dostêpnych przedmiotów." << std::endl;
            delete pstmt;
            return;
        }

        int subjectChoice;
        std::cout << "Wybierz numer przedmiotu do usuniêcia: ";
        std::cin >> subjectChoice;

        if (subjects.find(subjectChoice) == subjects.end()) {
            std::cout << "Nieprawid³owy wybór przedmiotu." << std::endl;
            delete pstmt;
            return;
        }

        std::string subjectName = subjects[subjectChoice];

        pstmt = con->prepareStatement("DELETE FROM Przedmioty WHERE nazwa = ?");
        pstmt->setString(1, subjectName);

        int updateCount = pstmt->executeUpdate();

        if (updateCount > 0) {
            std::cout << "Przedmiot " << subjectName << " zosta³ usuniêty." << std::endl;
        }
        else {
            std::cout << "Nie uda³o siê usun¹æ przedmiotu." << std::endl;
        }

        if (pstmt != nullptr) {
            delete pstmt;
        }
    }
    catch (sql::SQLException& e) {
        std::cout << "B³¹d SQL: " << e.what() << std::endl;
    }

    pressEnterToContinue();
    clearConsole();
}

void Admin::assignTeacherToClass() {
    clearConsole();

    try {
        // Retrieve and display teachers
        sql::PreparedStatement* pstmt = con->prepareStatement("SELECT id, username FROM Nauczyciele");
        sql::ResultSet* res = pstmt->executeQuery();

        std::map<int, int> teacherMap;
        int teacherCounter = 1;
        std::cout << "Dostêpni nauczyciele:" << std::endl;
        while (res->next()) {
            int teacherId = res->getInt("id");
            std::string teacherUsername = res->getString("username");
            teacherMap[teacherCounter] = teacherId;
            std::cout << teacherCounter << ". " << teacherUsername << std::endl;
            teacherCounter++;
        }
        delete res;

        if (teacherMap.empty()) {
            std::cout << "Brak dostêpnych nauczycieli." << std::endl;
            delete pstmt;
            return;
        }

        int teacherChoice;
        std::cout << "Wybierz numer nauczyciela: ";
        std::cin >> teacherChoice;

        if (teacherMap.find(teacherChoice) == teacherMap.end()) {
            std::cout << "Nieprawid³owy wybór nauczyciela." << std::endl;
            delete pstmt;
            return;
        }

        int teacherId = teacherMap[teacherChoice];

        // Retrieve and display classes
        pstmt = con->prepareStatement("SELECT id, nazwa FROM Klasy");
        res = pstmt->executeQuery();

        std::map<int, int> classMap;
        int classCounter = 1;
        std::cout << "Dostêpne klasy:" << std::endl;
        while (res->next()) {
            int classId = res->getInt("id");
            std::string className = res->getString("nazwa");
            classMap[classCounter] = classId;
            std::cout << classCounter << ". " << className << std::endl;
            classCounter++;
        }
        delete res;

        if (classMap.empty()) {
            std::cout << "Brak dostêpnych klas." << std::endl;
            delete pstmt;
            return;
        }

        int classChoice;
        std::cout << "Wybierz numer klasy: ";
        std::cin >> classChoice;

        if (classMap.find(classChoice) == classMap.end()) {
            std::cout << "Nieprawid³owy wybór klasy." << std::endl;
            delete pstmt;
            return;
        }

        int classId = classMap[classChoice];

        // Assign teacher to class
        pstmt = con->prepareStatement("INSERT INTO Nauczyciele_Klasy (id_nauczyciela, id_klasy) VALUES (?, ?)");
        pstmt->setInt(1, teacherId);
        pstmt->setInt(2, classId);
        pstmt->executeUpdate();

        std::cout << "Nauczyciel zosta³ przypisany do klasy pomyœlnie." << std::endl;

        if (pstmt != nullptr) {
            delete pstmt;
        }
    }
    catch (sql::SQLException& e) {
        std::cout << "B³¹d SQL: " << e.what() << std::endl;
    }

    pressEnterToContinue();
    clearConsole();
}


void Admin::assignTeacherToSubject() {
    clearConsole();

    try {
        sql::PreparedStatement* pstmt = con->prepareStatement("SELECT id, username FROM Nauczyciele");
        sql::ResultSet* res = pstmt->executeQuery();

        std::map<int, int> teacherMap;
        int teacherCounter = 1;
        std::cout << "Dostêpni nauczyciele:" << std::endl;
        while (res->next()) {
            int teacherId = res->getInt("id");
            std::string teacherUsername = res->getString("username");
            teacherMap[teacherCounter] = teacherId;
            std::cout << teacherCounter << ". " << teacherUsername << std::endl;
            teacherCounter++;
        }
        delete res;

        if (teacherMap.empty()) {
            std::cout << "Brak dostêpnych nauczycieli." << std::endl;
            delete pstmt;
            return;
        }

        int teacherChoice;
        std::cout << "Wybierz numer nauczyciela: ";
        std::cin >> teacherChoice;

        if (teacherMap.find(teacherChoice) == teacherMap.end()) {
            std::cout << "Nieprawid³owy wybór nauczyciela." << std::endl;
            delete pstmt;
            return;
        }

        int teacherId = teacherMap[teacherChoice];

        pstmt = con->prepareStatement("SELECT id, nazwa FROM Przedmioty");
        res = pstmt->executeQuery();

        std::map<int, int> subjectMap;
        int subjectCounter = 1;
        std::cout << "Dostêpne przedmioty:" << std::endl;
        while (res->next()) {
            int subjectId = res->getInt("id");
            std::string subjectName = res->getString("nazwa");
            subjectMap[subjectCounter] = subjectId;
            std::cout << subjectCounter << ". " << subjectName << std::endl;
            subjectCounter++;
        }
        delete res;

        if (subjectMap.empty()) {
            std::cout << "Brak dostêpnych przedmiotów." << std::endl;
            delete pstmt;
            return;
        }

        int subjectChoice;
        std::cout << "Wybierz numer przedmiotu: ";
        std::cin >> subjectChoice;

        if (subjectMap.find(subjectChoice) == subjectMap.end()) {
            std::cout << "Nieprawid³owy wybór przedmiotu." << std::endl;
            delete pstmt;
            return;
        }

        int subjectId = subjectMap[subjectChoice];

        pstmt = con->prepareStatement("INSERT INTO Nauczyciele_Przedmioty (id_nauczyciela, id_przedmiotu) VALUES (?, ?)");
        pstmt->setInt(1, teacherId);
        pstmt->setInt(2, subjectId);
        pstmt->executeUpdate();

        std::cout << "Nauczyciel zosta³ przypisany do przedmiotu pomyœlnie." << std::endl;

        if (pstmt != nullptr) {
            delete pstmt;
        }
    }
    catch (sql::SQLException& e) {
        std::cout << "B³¹d SQL: " << e.what() << std::endl;
    }

    pressEnterToContinue();
    clearConsole();
}

void Admin::assignStudentToClass() {
    clearConsole();

    try {
        sql::PreparedStatement* pstmt = con->prepareStatement("SELECT id, username FROM Uczniowie");
        sql::ResultSet* res = pstmt->executeQuery();

        std::map<int, int> students;
        int studentCounter = 1;
        std::cout << "Dostêpni uczniowie:" << std::endl;
        while (res->next()) {
            int studentId = res->getInt("id");
            std::string studentUsername = res->getString("username");
            students[studentCounter] = studentId;
            std::cout << studentCounter << ". " << studentUsername << std::endl;
            studentCounter++;
        }
        delete res;

        if (students.empty()) {
            std::cout << "Brak dostêpnych uczniów." << std::endl;
            delete pstmt;
            return;
        }

        int studentChoice;
        std::cout << "Wybierz numer ucznia: ";
        std::cin >> studentChoice;

        if (students.find(studentChoice) == students.end()) {
            std::cout << "Nieprawid³owy wybór ucznia." << std::endl;
            delete pstmt;
            return;
        }

        int studentId = students[studentChoice];

        pstmt = con->prepareStatement("SELECT id, nazwa FROM Klasy");
        res = pstmt->executeQuery();

        std::map<int, int> classes;
        int classCounter = 1;
        std::cout << "Dostêpne klasy:" << std::endl;
        while (res->next()) {
            int classId = res->getInt("id");
            std::string className = res->getString("nazwa");
            classes[classCounter] = classId;
            std::cout << classCounter << ". " << className << std::endl;
            classCounter++;
        }
        delete res;

        if (classes.empty()) {
            std::cout << "Brak dostêpnych klas." << std::endl;
            delete pstmt;
            return;
        }

        int classChoice;
        std::cout << "Wybierz numer klasy: ";
        std::cin >> classChoice;

        if (classes.find(classChoice) == classes.end()) {
            std::cout << "Nieprawid³owy wybór klasy." << std::endl;
            delete pstmt;
            return;
        }

        int classId = classes[classChoice];

        pstmt = con->prepareStatement("INSERT INTO Klasy_Uczniowie (id_ucznia, id_klasy) VALUES (?, ?)");
        pstmt->setInt(1, studentId);
        pstmt->setInt(2, classId);
        int affectedRows = pstmt->executeUpdate();

        if (affectedRows > 0) {
            std::cout << "Uczeñ zosta³ przypisany do klasy pomyœlnie." << std::endl;
        }
        else {
            std::cout << "Nie uda³o siê przypisaæ ucznia do klasy." << std::endl;
        }

        if (pstmt != nullptr) {
            delete pstmt;
        }
    }
    catch (sql::SQLException& e) {
        std::cout << "B³¹d SQL: " << e.what() << std::endl;
    }

    pressEnterToContinue();
    clearConsole();
}

void Admin::adminMenu() {
    int choice;
    do {
        clearConsole();

        std::cout << "\n--- Menu Administratora ---\n";
        std::cout << "1. Dodaj nauczyciela\n";
        std::cout << "2. Edytuj nauczyciela\n";
        std::cout << "3. Usuñ nauczyciela\n";
        std::cout << "4. Dodaj ucznia\n";
        std::cout << "5. Edytuj ucznia\n";
        std::cout << "6. Usuñ ucznia\n";
        std::cout << "7. Utwórz klasê\n";
        std::cout << "8. Edytuj klasê\n";
        std::cout << "9. Usuñ klasê\n";
        std::cout << "10. Dodaj przedmiot\n";
        std::cout << "11. Edytuj przedmiot\n";
        std::cout << "12. Usuñ przedmiot\n";
        std::cout << "13. Przypisz nauczyciela do klasy\n";
        std::cout << "14. Przypisz nauczyciela do przedmiotu\n";
        std::cout << "15. Przypisz ucznia do klasy\n";
        std::cout << "16. Wyloguj\n";
        std::cout << "Twój wybór: ";
        std::cin >> choice;

        switch (choice) {
        case 1:
            addTeacher();
            break;
        case 2:
            editTeacher();
            break;
        case 3:
            removeTeacher();
            break;
        case 4:
            addStudent();
            break;
        case 5:
            editStudent();
            break;
        case 6:
            removeStudent();
            break;
        case 7:
            createClass();
            break;
        case 8:
            editClass();
            break;
        case 9:
            removeClass();
            break;
        case 10:
            addSubject();
            break;
        case 11:
            editSubject();
            break;
        case 12:
            removeSubject();
            break;
        case 13:
            assignTeacherToClass();
            break;
        case 14:
            assignTeacherToSubject();
            break;
        case 15:
            assignStudentToClass();
            break;
        case 16:
            return;
        default:
            std::cout << "Nieprawid³owy wybór. Spróbuj ponownie." << std::endl;
        }
    } while (true);
}
