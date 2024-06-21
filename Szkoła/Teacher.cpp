#include "Teacher.h"

Teacher::Teacher(sql::Connection* connection) : User(connection), teacherId(-1) {}

bool Teacher::login() {
    clearConsole();

    std::string teacherUsername, teacherPassword;
    std::cout << "Wpisz swoj¹ nazwê u¿ytkownika: ";
    std::cin >> teacherUsername;
    std::cout << "Podaj has³o: ";
    std::cin >> teacherPassword;

    try {
        sql::PreparedStatement* pstmt;
        pstmt = con->prepareStatement("SELECT id FROM Nauczyciele WHERE username=? AND password=?");
        pstmt->setString(1, teacherUsername);
        pstmt->setString(2, teacherPassword);
        sql::ResultSet* res = pstmt->executeQuery();

        if (res->next()) {
            teacherId = res->getInt("id");  // Przypisanie id nauczyciela
            std::cout << "Pomyœlnie zalogowano jako nauczyciel." << std::endl;
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
void Teacher::addGrade() {
    clearConsole();

    try {
        sql::PreparedStatement* pstmt = nullptr;
        pstmt = con->prepareStatement(
            "SELECT p.id, p.nazwa "
            "FROM Przedmioty p "
            "JOIN Nauczyciele_Przedmioty np ON p.id = np.id_przedmiotu "
            "WHERE np.id_nauczyciela = ?");
        pstmt->setInt(1, teacherId);
        sql::ResultSet* res = pstmt->executeQuery();

        std::map<int, std::string> subjects;
        std::map<int, int> subjectChoices;  // Mapa do przechowywania sekwencyjnego numerowania

        int counter = 1;
        while (res->next()) {
            int subjectId = res->getInt("id");
            std::string subjectName = res->getString("nazwa");
            subjects[subjectId] = subjectName;
            subjectChoices[counter] = subjectId;  // Mapa wyboru
            counter++;
        }

        if (subjects.empty()) {
            std::cout << "Nie jesteœ przypisany do ¿adnego przedmiotu." << std::endl;
            delete pstmt;
            delete res;
            pressEnterToContinue();
            clearConsole();
            return;
        }

        std::cout << "Wybierz przedmiot:" << std::endl;
        for (const auto& pair : subjectChoices) {
            std::cout << pair.first << ". " << subjects[pair.second] << std::endl;
        }

        int subjectChoice;
        std::cout << "Twój wybór: ";
        std::cin >> subjectChoice;

        if (subjectChoices.find(subjectChoice) != subjectChoices.end()) {
            int subjectId = subjectChoices[subjectChoice];

            std::string studentUsername, gradeDate, gradeDescription;
            double grade;

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
                pressEnterToContinue();
                clearConsole();
                return;
            }

            int studentChoice;
            std::cout << "Wybierz numer ucznia: ";
            std::cin >> studentChoice;

            if (students.find(studentChoice) != students.end()) {
                std::string studentUsername = students[studentChoice];

                int studentId;
                pstmt = con->prepareStatement("SELECT id FROM Uczniowie WHERE username = ?");
                pstmt->setString(1, studentUsername);
                res = pstmt->executeQuery();
                if (res->next()) {
                    studentId = res->getInt("id");

                    std::cout << "Wpisz ocenê: ";
                    std::cin >> grade;
                    std::cout << "Wpisz datê oceny (RRRR-MM-DD): ";
                    std::cin >> gradeDate;
                    std::cout << "Wpisz opis oceny: ";
                    std::cin.ignore();
                    std::getline(std::cin, gradeDescription);

                    pstmt = con->prepareStatement("INSERT INTO Oceny (id_ucznia, id_przedmiotu, ocena, data_oceny, opis) VALUES (?, ?, ?, ?, ?)");
                    pstmt->setInt(1, studentId);
                    pstmt->setInt(2, subjectId);
                    pstmt->setDouble(3, grade);
                    pstmt->setString(4, gradeDate);
                    pstmt->setString(5, gradeDescription);
                    pstmt->executeUpdate();

                    std::cout << "Ocena zosta³a dodana pomyœlnie." << std::endl;
                }
                else {
                    std::cout << "Uczeñ o podanym u¿ytkowniku nie istnieje." << std::endl;
                }
                delete res;
            }
            else {
                std::cout << "Nieprawid³owy wybór ucznia." << std::endl;
            }
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

void Teacher::editGrade() {
    clearConsole();

    try {
        sql::PreparedStatement* pstmt = nullptr;
        pstmt = con->prepareStatement("SELECT p.id, p.nazwa FROM Przedmioty p JOIN Nauczyciele_Przedmioty np ON p.id = np.id_przedmiotu WHERE np.id_nauczyciela = ?");
        pstmt->setInt(1, teacherId);
        sql::ResultSet* res = pstmt->executeQuery();

        std::map<int, std::string> subjects;
        while (res->next()) {
            int subjectId = res->getInt("id");
            std::string subjectName = res->getString("nazwa");
            subjects[subjectId] = subjectName;
        }

        if (subjects.empty()) {
            std::cout << "Nie jesteœ przypisany do ¿adnego przedmiotu." << std::endl;
            delete pstmt;
            delete res;
            return;
        }

        std::cout << "Wybierz przedmiot:" << std::endl;
        for (const auto& pair : subjects) {
            std::cout << pair.first << ". " << pair.second << std::endl;
        }

        int subjectChoice;
        std::cout << "Twój wybór: ";
        std::cin >> subjectChoice;

        if (subjects.find(subjectChoice) != subjects.end()) {
            int subjectId = subjectChoice;
            std::string subjectName = subjects[subjectChoice];

            std::string studentUsername;
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
            std::cout << "Wybierz numer ucznia: ";
            std::cin >> studentChoice;

            if (students.find(studentChoice) != students.end()) {
                std::string studentUsername = students[studentChoice];

                int studentId;
                pstmt = con->prepareStatement("SELECT id FROM Uczniowie WHERE username = ?");
                pstmt->setString(1, studentUsername);
                res = pstmt->executeQuery();
                if (res->next()) {
                    studentId = res->getInt("id");

                    pstmt = con->prepareStatement("SELECT id, ocena, data_oceny, opis FROM Oceny WHERE id_ucznia = ? AND id_przedmiotu = ?");
                    pstmt->setInt(1, studentId);
                    pstmt->setInt(2, subjectId);
                    res = pstmt->executeQuery();

                    std::map<int, std::tuple<int, double, std::string, std::string>> grades;
                    int gradeCounter = 1;
                    std::cout << "Oceny ucznia:" << std::endl;
                    while (res->next()) {
                        int gradeId = res->getInt("id");
                        double gradeValue = res->getDouble("ocena");
                        std::string gradeDate = res->getString("data_oceny");
                        std::string gradeDescription = res->getString("opis");
                        grades[gradeCounter] = std::make_tuple(gradeId, gradeValue, gradeDate, gradeDescription);
                        std::cout << gradeCounter << ". Ocena: " << gradeValue << ", Data: " << gradeDate << ", Opis: " << gradeDescription << std::endl;
                        gradeCounter++;
                    }
                    delete res;

                    if (grades.empty()) {
                        std::cout << "Ten uczeñ nie ma jeszcze ¿adnych ocen z tego przedmiotu." << std::endl;
                        delete pstmt;
                        return;
                    }

                    int gradeChoice;
                    std::cout << "Wybierz numer oceny do edycji: ";
                    std::cin >> gradeChoice;

                    if (grades.find(gradeChoice) != grades.end()) {
                        int gradeIdToEdit = std::get<0>(grades[gradeChoice]);

                        double newGrade;
                        std::string newGradeDate, newGradeDescription;
                        std::cout << "Wpisz now¹ ocenê: ";
                        std::cin >> newGrade;
                        std::cout << "Wpisz now¹ datê oceny (RRRR-MM-DD): ";
                        std::cin >> newGradeDate;
                        std::cout << "Wpisz nowy opis oceny: ";
                        std::cin.ignore();
                        std::getline(std::cin, newGradeDescription);

                        pstmt = con->prepareStatement("UPDATE Oceny SET ocena = ?, data_oceny = ?, opis = ? WHERE id = ?");
                        pstmt->setDouble(1, newGrade);
                        pstmt->setString(2, newGradeDate);
                        pstmt->setString(3, newGradeDescription);
                        pstmt->setInt(4, gradeIdToEdit);
                        pstmt->executeUpdate();

                        std::cout << "Ocena zosta³a pomyœlnie zaktualizowana." << std::endl;
                    }
                    else {
                        std::cout << "Nieprawid³owy wybór oceny." << std::endl;
                    }
                }
                else {
                    std::cout << "Uczeñ o podanym u¿ytkowniku nie istnieje." << std::endl;
                }
            }
            else {
                std::cout << "Nieprawid³owy wybór ucznia." << std::endl;
            }
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

void Teacher::deleteGrade() {
    clearConsole();

    try {
        sql::PreparedStatement* pstmt = nullptr;
        pstmt = con->prepareStatement("SELECT p.id, p.nazwa FROM Przedmioty p JOIN Nauczyciele_Przedmioty np ON p.id = np.id_przedmiotu WHERE np.id_nauczyciela = ?");
        pstmt->setInt(1, teacherId);
        sql::ResultSet* res = pstmt->executeQuery();

        std::map<int, std::string> subjects;
        while (res->next()) {
            int subjectId = res->getInt("id");
            std::string subjectName = res->getString("nazwa");
            subjects[subjectId] = subjectName;
        }

        if (subjects.empty()) {
            std::cout << "Nie jesteœ przypisany do ¿adnego przedmiotu." << std::endl;
            delete pstmt;
            delete res;
            return;
        }

        std::cout << "Wybierz przedmiot:" << std::endl;
        for (const auto& pair : subjects) {
            std::cout << pair.first << ". " << pair.second << std::endl;
        }

        int subjectChoice;
        std::cout << "Twój wybór: ";
        std::cin >> subjectChoice;

        if (subjects.find(subjectChoice) != subjects.end()) {
            int subjectId = subjectChoice;
            std::string subjectName = subjects[subjectChoice];

            std::string studentUsername;
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
            std::cout << "Wybierz numer ucznia: ";
            std::cin >> studentChoice;

            if (students.find(studentChoice) != students.end()) {
                std::string studentUsername = students[studentChoice];

                int studentId;
                pstmt = con->prepareStatement("SELECT id FROM Uczniowie WHERE username = ?");
                pstmt->setString(1, studentUsername);
                res = pstmt->executeQuery();
                if (res->next()) {
                    studentId = res->getInt("id");

                    std::cout << "\nOceny ucznia:" << std::endl;
                    pstmt = con->prepareStatement(
                        "SELECT o.id, p.nazwa AS przedmiot, o.ocena, o.data_oceny, o.opis "
                        "FROM Oceny o "
                        "JOIN Przedmioty p ON o.id_przedmiotu = p.id "
                        "WHERE o.id_ucznia = ?");
                    pstmt->setInt(1, studentId);
                    res = pstmt->executeQuery();

                    std::map<int, int> grades;
                    int gradeCounter = 1;
                    while (res->next()) {
                        int gradeId = res->getInt("id");
                        std::string subjectName = res->getString("przedmiot");
                        double gradeValue = res->getDouble("ocena");
                        std::string gradeDate = res->getString("data_oceny");
                        std::string gradeDescription = res->getString("opis");
                        std::cout << gradeCounter << ". Przedmiot: " << subjectName << ", Ocena: " << gradeValue
                            << ", Data: " << gradeDate << ", Opis: " << gradeDescription << std::endl;
                        grades[gradeCounter] = gradeId;
                        gradeCounter++;
                    }

                    if (!grades.empty()) {
                        int gradeChoice;
                        std::cout << "Wybierz numer oceny do usuniêcia: ";
                        std::cin >> gradeChoice;

                        if (grades.find(gradeChoice) != grades.end()) {
                            int gradeIdToDelete = grades[gradeChoice];

                            pstmt = con->prepareStatement("DELETE FROM Oceny WHERE id = ?");
                            pstmt->setInt(1, gradeIdToDelete);
                            pstmt->executeUpdate();

                            std::cout << "Ocena zosta³a usuniêta pomyœlnie." << std::endl;
                        }
                        else {
                            std::cout << "Nieprawid³owy wybór oceny." << std::endl;
                        }
                    }
                    else {
                        std::cout << "Ten uczeñ nie ma jeszcze ¿adnych ocen." << std::endl;
                    }
                }
                else {
                    std::cout << "Uczeñ o podanym u¿ytkowniku nie istnieje." << std::endl;
                }
                delete res;
            }
            else {
                std::cout << "Nieprawid³owy wybór ucznia." << std::endl;
            }
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

void Teacher::addComment() {
    clearConsole();

    try {
        sql::PreparedStatement* pstmt = nullptr;
        pstmt = con->prepareStatement("SELECT id, username FROM Uczniowie");
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
        std::cout << "Wybierz numer ucznia: ";
        std::cin >> studentChoice;

        if (students.find(studentChoice) != students.end()) {
            std::string studentUsername = students[studentChoice];

            int studentId;
            pstmt = con->prepareStatement("SELECT id FROM Uczniowie WHERE username = ?");
            pstmt->setString(1, studentUsername);
            res = pstmt->executeQuery();
            if (res->next()) {
                studentId = res->getInt("id");

                std::cin.ignore();
                std::string commentContent;
                std::cout << "Wpisz treœæ uwagi: ";
                std::getline(std::cin, commentContent);

                pstmt = con->prepareStatement("INSERT INTO Uwagi (id_ucznia, tresc) VALUES (?, ?)");
                pstmt->setInt(1, studentId);
                pstmt->setString(2, commentContent);
                pstmt->executeUpdate();

                std::cout << "Uwaga zosta³a dodana pomyœlnie." << std::endl;
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

void Teacher::editComment() {
    clearConsole();

    try {
        sql::PreparedStatement* pstmt = nullptr;
        pstmt = con->prepareStatement("SELECT id, username FROM Uczniowie");
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
        std::cout << "Wybierz numer ucznia: ";
        std::cin >> studentChoice;

        if (students.find(studentChoice) != students.end()) {
            std::string studentUsername = students[studentChoice];

            int studentId;
            pstmt = con->prepareStatement("SELECT id FROM Uczniowie WHERE username = ?");
            pstmt->setString(1, studentUsername);
            res = pstmt->executeQuery();
            if (res->next()) {
                studentId = res->getInt("id");

                pstmt = con->prepareStatement("SELECT id, tresc FROM Uwagi WHERE id_ucznia = ?");
                pstmt->setInt(1, studentId);
                res = pstmt->executeQuery();

                std::map<int, std::pair<int, std::string>> comments;
                int commentCounter = 1;
                std::cout << "Uwagi ucznia:" << std::endl;
                while (res->next()) {
                    int commentId = res->getInt("id");
                    std::string commentContent = res->getString("tresc");
                    comments[commentCounter] = std::make_pair(commentId, commentContent);
                    std::cout << commentCounter << ". " << commentContent << std::endl;
                    commentCounter++;
                }
                delete res;

                if (comments.empty()) {
                    std::cout << "Ten uczeñ nie ma jeszcze ¿adnych uwag." << std::endl;
                    delete pstmt;
                    return;
                }

                int commentChoice;
                std::cout << "Wybierz numer uwagi do edycji: ";
                std::cin >> commentChoice;

                if (comments.find(commentChoice) != comments.end()) {
                    int commentIdToEdit = comments[commentChoice].first;

                    std::cin.ignore();
                    std::string newCommentContent;
                    std::cout << "Wpisz now¹ treœæ uwagi: ";
                    std::getline(std::cin, newCommentContent);

                    pstmt = con->prepareStatement("UPDATE Uwagi SET tresc = ? WHERE id = ?");
                    pstmt->setString(1, newCommentContent);
                    pstmt->setInt(2, commentIdToEdit);
                    pstmt->executeUpdate();

                    std::cout << "Uwaga zosta³a pomyœlnie zaktualizowana." << std::endl;
                }
                else {
                    std::cout << "Nieprawid³owy wybór uwagi." << std::endl;
                }
            }
            else {
                std::cout << "Uczeñ o podanym u¿ytkowniku nie istnieje." << std::endl;
            }
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

void Teacher::deleteComment() {
    clearConsole();

    try {
        sql::PreparedStatement* pstmt = nullptr;
        pstmt = con->prepareStatement("SELECT id, username FROM Uczniowie");
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
        std::cout << "Wybierz numer ucznia: ";
        std::cin >> studentChoice;

        if (students.find(studentChoice) != students.end()) {
            std::string studentUsername = students[studentChoice];

            int studentId;
            pstmt = con->prepareStatement("SELECT id FROM Uczniowie WHERE username = ?");
            pstmt->setString(1, studentUsername);
            res = pstmt->executeQuery();
            if (res->next()) {
                studentId = res->getInt("id");

                pstmt = con->prepareStatement("SELECT id, tresc FROM Uwagi WHERE id_ucznia = ?");
                pstmt->setInt(1, studentId);
                res = pstmt->executeQuery();

                std::map<int, int> comments;
                int commentCounter = 1;
                std::cout << "Uwagi ucznia:" << std::endl;
                while (res->next()) {
                    int commentId = res->getInt("id");
                    std::string commentContent = res->getString("tresc");
                    comments[commentCounter] = commentId;
                    std::cout << commentCounter << ". " << commentContent << std::endl;
                    commentCounter++;
                }

                if (!comments.empty()) {
                    int commentChoice;
                    std::cout << "Wybierz numer uwagi do usuniêcia: ";
                    std::cin >> commentChoice;

                    if (comments.find(commentChoice) != comments.end()) {
                        int commentIdToDelete = comments[commentChoice];

                        pstmt = con->prepareStatement("DELETE FROM Uwagi WHERE id = ?");
                        pstmt->setInt(1, commentIdToDelete);
                        pstmt->executeUpdate();

                        std::cout << "Uwaga zosta³a usuniêta pomyœlnie." << std::endl;
                    }
                    else {
                        std::cout << "Nieprawid³owy wybór uwagi." << std::endl;
                    }
                }
                else {
                    std::cout << "Ten uczeñ nie ma jeszcze ¿adnych uwag." << std::endl;
                }
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

void Teacher::viewClassStudents() {
    clearConsole();

    try {
        sql::PreparedStatement* pstmt = nullptr;
        pstmt = con->prepareStatement("SELECT id, nazwa FROM Klasy");
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
        std::cout << "Wybierz numer klasy: ";
        std::cin >> classChoice;

        if (classes.find(classChoice) != classes.end()) {
            clearConsole();

            std::string className = classes[classChoice];

            pstmt = con->prepareStatement("SELECT id FROM Klasy WHERE nazwa = ?");
            pstmt->setString(1, className);
            res = pstmt->executeQuery();

            if (res->next()) {
                int classId = res->getInt("id");

                delete pstmt;
                pstmt = con->prepareStatement("SELECT o.id, o.imie, o.nazwisko FROM Osoby o JOIN Klasy_Uczniowie ku ON o.id = ku.id_ucznia WHERE ku.id_klasy = ?");
                pstmt->setInt(1, classId);
                res = pstmt->executeQuery();

                std::map<int, std::pair<int, std::pair<std::string, std::string>>> students;
                int counter = 1;
                while (res->next()) {
                    int studentId = res->getInt("id");
                    std::string studentName = res->getString("imie");
                    std::string studentSurname = res->getString("nazwisko");
                    students[counter] = std::make_pair(studentId, std::make_pair(studentName, studentSurname));
                    std::cout << counter << ". " << studentName << " " << studentSurname << std::endl;
                    counter++;
                }

                if (students.empty()) {
                    std::cout << "W tej klasie nie ma ¿adnych uczniów." << std::endl;
                    delete pstmt;
                    delete res;
                    return;
                }

                int choice;
                std::cout << "Wybierz numer ucznia, aby zobaczyæ szczegó³y: ";
                std::cin >> choice;

                if (students.find(choice) != students.end()) {
                    clearConsole();

                    int studentId = students[choice].first;

                    pstmt = con->prepareStatement("SELECT * FROM Osoby WHERE id = ?");
                    pstmt->setInt(1, studentId);
                    res = pstmt->executeQuery();

                    if (res->next()) {
                        std::cout << "Informacje o uczniu:" << std::endl;
                        std::cout << "Imiê: " << res->getString("imie") << std::endl;
                        std::cout << "Nazwisko: " << res->getString("nazwisko") << std::endl;
                        std::cout << "Data urodzenia: " << res->getString("data_urodzenia") << std::endl;
                        std::cout << "PESEL: " << res->getString("pesel") << std::endl;
                        std::cout << "Adres: " << res->getString("adres") << std::endl;
                        std::cout << "Telefon: " << res->getString("telefon") << std::endl;
                        std::cout << "Email: " << res->getString("email") << std::endl;
                    }
                    delete res;

                    std::cout << "\nOceny ucznia:" << std::endl;
                    pstmt = con->prepareStatement(
                        "SELECT p.nazwa, o.ocena, o.data_oceny, o.opis FROM Oceny o "
                        "JOIN Przedmioty p ON o.id_przedmiotu = p.id WHERE o.id_ucznia = ?");
                    pstmt->setInt(1, studentId);
                    res = pstmt->executeQuery();

                    while (res->next()) {
                        std::cout << "Przedmiot: " << res->getString("nazwa") << ", Ocena: " << res->getDouble("ocena")
                            << ", Data: " << res->getString("data_oceny") << ", Opis: " << res->getString("opis") << std::endl;
                    }
                    delete res;

                    std::cout << "\nUwagi ucznia:" << std::endl;
                    pstmt = con->prepareStatement("SELECT * FROM Uwagi WHERE id_ucznia = ?");
                    pstmt->setInt(1, studentId);
                    res = pstmt->executeQuery();

                    while (res->next()) {
                        std::cout << "Uwaga: " << res->getString("tresc") << std::endl;
                    }
                    delete res;
                }
                else {
                    std::cout << "Nieprawid³owy wybór ucznia." << std::endl;
                }
            }
            else {
                std::cout << "Wyst¹pi³ b³¹d podczas pobierania danych o klasie." << std::endl;
            }
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

void Teacher::teacherMenu() {
    int choice;
    do {
        clearConsole();

        std::cout << "\n--- Menu Nauczyciela ---\n";
        std::cout << "1. Dodaj ocenê\n";
        std::cout << "2. Edytuj ocenê\n";
        std::cout << "3. Usuñ ocenê\n";
        std::cout << "4. Dodaj uwagê\n";
        std::cout << "5. Edytuj uwagê\n";
        std::cout << "6. Usuñ uwagê\n";
        std::cout << "7. Wyœwietl uczniów klasy\n";
        std::cout << "8. Wyloguj\n";
        std::cout << "Twój wybór: ";
        std::cin >> choice;

        switch (choice) {
        case 1:
            addGrade();
            break;
        case 2:
            editGrade();
            break;
        case 3:
            deleteGrade();
            break;
        case 4:
            addComment();
            break;
        case 5:
            editComment();
            break;
        case 6:
            deleteComment();
            break;
        case 7:
            viewClassStudents();
            break;
        case 8:
            return;
        default:
            std::cout << "Nieprawid³owy wybór. Spróbuj ponownie." << std::endl;
        }
    } while (true);
}
