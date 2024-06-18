#include <iostream>
#include <locale.h>
#include <mysql_connection.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <string>
#include <map>
#include <tuple>
#include <utility>

using namespace std;

const string server = "tcp://127.0.0.1:3306";
const string username = "root";
const string password = "";
const string db = "schoolcrm";

class DatabaseManager {
private:
    sql::Driver* driver;
    sql::Connection* con;

public:
    DatabaseManager() : driver(nullptr), con(nullptr) {}

    DatabaseManager(const std::string& server, const std::string& username, const std::string& password, const std::string& db) {
        try {
            driver = get_driver_instance();
            con = driver->connect(server, username, password);
            con->setSchema(db);
        }
        catch (sql::SQLException& e) {
            std::cerr << "Wyjątek SQL: " << e.what() << std::endl;
            std::cerr << "Kod błędu MySQL: " << e.getErrorCode() << std::endl;
            std::cerr << "SQLState: " << e.getSQLState() << std::endl;
        }
        catch (std::exception& e) {
            std::cerr << "Standardowy wyjątek: " << e.what() << std::endl;
        }
        catch (...) {
            std::cerr << "Złapano nieznany wyjątek" << std::endl;
        }
    }

    ~DatabaseManager() {
        delete con;
    }

    sql::Connection* getConnection() {
        return con;
    }
};

void pressEnterToContinue() {
    std::cout << "\n\nNaciśnij Enter, aby kontynuować...";
    std::cin.ignore();
    std::cin.get();
}

void clearConsole() {
    system("cls");
}

class User {
protected:
    sql::Connection* con;

public:
    User(sql::Connection* connection) : con(connection) {}

    virtual bool login() = 0;
};

class Teacher : public User {
private:
    int teacherId;

public:
    Teacher(sql::Connection* connection) : User(connection), teacherId(-1) {}

    bool login() override {
        clearConsole();

        string teacherUsername, teacherPassword;
        cout << "Wpisz swoją nazwę użytkownika: ";
        cin >> teacherUsername;
        cout << "Podaj hasło: ";
        cin >> teacherPassword;

        try {
            sql::PreparedStatement* pstmt;
            pstmt = con->prepareStatement("SELECT id FROM Nauczyciele WHERE username=? AND password=?");
            pstmt->setString(1, teacherUsername);
            pstmt->setString(2, teacherPassword);
            sql::ResultSet* res = pstmt->executeQuery();

            if (res->next()) {
                teacherId = res->getInt("id");
                cout << "Pomyślnie zalogowano jako nauczyciel." << endl;
                delete pstmt;
                delete res;
                pressEnterToContinue();
                clearConsole();
                return true;
            }
            else {
                cout << "Logowanie nie powiodło się. Niepoprawna nazwa użytkownika lub hasło." << endl;
                delete pstmt;
                delete res;
                pressEnterToContinue();
                clearConsole();
                return false;
            }
        }
        catch (sql::SQLException& e) {
            cout << "Błąd SQL: " << e.what() << endl;
            return false;
        }

        clearConsole();
    }

    void addGrade() {
        clearConsole();

        try {
            sql::PreparedStatement* pstmt = nullptr;
            pstmt = con->prepareStatement("SELECT p.id, p.nazwa FROM Przedmioty p JOIN Nauczyciele_Przedmioty np ON p.id = np.id_przedmiotu WHERE np.id_nauczyciela = ?");
            pstmt->setInt(1, teacherId);
            sql::ResultSet* res = pstmt->executeQuery();

            map<int, string> subjects;
            while (res->next()) {
                int subjectId = res->getInt("id");
                string subjectName = res->getString("nazwa");
                subjects[subjectId] = subjectName;
            }

            if (subjects.empty()) {
                cout << "Nie jesteś przypisany do żadnego przedmiotu." << endl;
                delete pstmt;
                delete res;
                return;
            }

            cout << "Wybierz przedmiot:" << endl;
            for (const auto& pair : subjects) {
                cout << pair.first << ". " << pair.second << endl;
            }

            int subjectChoice;
            cout << "Twój wybór: ";
            cin >> subjectChoice;

            if (subjects.find(subjectChoice) != subjects.end()) {
                int subjectId = subjectChoice;
                string subjectName = subjects[subjectChoice];

                string studentUsername, gradeDate, gradeDescription;
                double grade;

                pstmt = con->prepareStatement("SELECT id, username FROM Uczniowie");
                res = pstmt->executeQuery();

                map<int, string> students;
                int studentCounter = 1;
                cout << "Dostępni uczniowie:" << endl;
                while (res->next()) {
                    int studentId = res->getInt("id");
                    string studentUsername = res->getString("username");
                    students[studentCounter] = studentUsername;
                    cout << studentCounter << ". " << studentUsername << endl;
                    studentCounter++;
                }
                delete res;

                if (students.empty()) {
                    cout << "Brak dostępnych uczniów." << endl;
                    delete pstmt;
                    return;
                }

                int studentChoice;
                cout << "Wybierz numer ucznia: ";
                cin >> studentChoice;

                if (students.find(studentChoice) != students.end()) {
                    string studentUsername = students[studentChoice];

                    int studentId;
                    pstmt = con->prepareStatement("SELECT id FROM Uczniowie WHERE username = ?");
                    pstmt->setString(1, studentUsername);
                    res = pstmt->executeQuery();
                    if (res->next()) {
                        studentId = res->getInt("id");

                        cout << "Wpisz ocenę: ";
                        cin >> grade;
                        cout << "Wpisz datę oceny (RRRR-MM-DD): ";
                        cin >> gradeDate;
                        cout << "Wpisz opis oceny: ";
                        cin.ignore();
                        getline(cin, gradeDescription);

                        pstmt = con->prepareStatement("INSERT INTO Oceny (id_ucznia, id_przedmiotu, ocena, data_oceny, opis) VALUES (?, ?, ?, ?, ?)");
                        pstmt->setInt(1, studentId);
                        pstmt->setInt(2, subjectId);
                        pstmt->setDouble(3, grade);
                        pstmt->setString(4, gradeDate);
                        pstmt->setString(5, gradeDescription);
                        pstmt->executeUpdate();

                        cout << "Ocena została dodana pomyślnie." << endl;
                    }
                    else {
                        cout << "Uczeń o podanym użytkowniku nie istnieje." << endl;
                    }
                    delete res;
                }
                else {
                    cout << "Nieprawidłowy wybór ucznia." << endl;
                }
            }
            else {
                cout << "Nieprawidłowy wybór przedmiotu." << endl;
            }

            if (pstmt != nullptr) {
                delete pstmt;
            }
        }
        catch (sql::SQLException& e) {
            cout << "Błąd SQL: " << e.what() << endl;
        }

        pressEnterToContinue();
        clearConsole();
    }

    void editGrade() {
        clearConsole();

        try {
            sql::PreparedStatement* pstmt = nullptr;
            pstmt = con->prepareStatement("SELECT p.id, p.nazwa FROM Przedmioty p JOIN Nauczyciele_Przedmioty np ON p.id = np.id_przedmiotu WHERE np.id_nauczyciela = ?");
            pstmt->setInt(1, teacherId);
            sql::ResultSet* res = pstmt->executeQuery();

            map<int, string> subjects;
            while (res->next()) {
                int subjectId = res->getInt("id");
                string subjectName = res->getString("nazwa");
                subjects[subjectId] = subjectName;
            }

            if (subjects.empty()) {
                cout << "Nie jesteś przypisany do żadnego przedmiotu." << endl;
                delete pstmt;
                delete res;
                return;
            }

            cout << "Wybierz przedmiot:" << endl;
            for (const auto& pair : subjects) {
                cout << pair.first << ". " << pair.second << endl;
            }

            int subjectChoice;
            cout << "Twój wybór: ";
            cin >> subjectChoice;

            if (subjects.find(subjectChoice) != subjects.end()) {
                int subjectId = subjectChoice;
                string subjectName = subjects[subjectChoice];

                string studentUsername;
                pstmt = con->prepareStatement("SELECT id, username FROM Uczniowie");
                res = pstmt->executeQuery();

                map<int, string> students;
                int studentCounter = 1;
                cout << "Dostępni uczniowie:" << endl;
                while (res->next()) {
                    int studentId = res->getInt("id");
                    string studentUsername = res->getString("username");
                    students[studentCounter] = studentUsername;
                    cout << studentCounter << ". " << studentUsername << endl;
                    studentCounter++;
                }
                delete res;

                if (students.empty()) {
                    cout << "Brak dostępnych uczniów." << endl;
                    delete pstmt;
                    return;
                }

                int studentChoice;
                cout << "Wybierz numer ucznia: ";
                cin >> studentChoice;

                if (students.find(studentChoice) != students.end()) {
                    string studentUsername = students[studentChoice];

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

                        map<int, tuple<int, double, string, string>> grades;
                        int gradeCounter = 1;
                        cout << "Oceny ucznia:" << endl;
                        while (res->next()) {
                            int gradeId = res->getInt("id");
                            double gradeValue = res->getDouble("ocena");
                            string gradeDate = res->getString("data_oceny");
                            string gradeDescription = res->getString("opis");
                            grades[gradeCounter] = make_tuple(gradeId, gradeValue, gradeDate, gradeDescription);
                            cout << gradeCounter << ". Ocena: " << gradeValue << ", Data: " << gradeDate << ", Opis: " << gradeDescription << endl;
                            gradeCounter++;
                        }
                        delete res;

                        if (grades.empty()) {
                            cout << "Ten uczeń nie ma jeszcze żadnych ocen z tego przedmiotu." << endl;
                            delete pstmt;
                            return;
                        }

                        int gradeChoice;
                        cout << "Wybierz numer oceny do edycji: ";
                        cin >> gradeChoice;

                        if (grades.find(gradeChoice) != grades.end()) {
                            int gradeIdToEdit = get<0>(grades[gradeChoice]);

                            double newGrade;
                            string newGradeDate, newGradeDescription;
                            cout << "Wpisz nową ocenę: ";
                            cin >> newGrade;
                            cout << "Wpisz nową datę oceny (RRRR-MM-DD): ";
                            cin >> newGradeDate;
                            cout << "Wpisz nowy opis oceny: ";
                            cin.ignore();
                            getline(cin, newGradeDescription);

                            pstmt = con->prepareStatement("UPDATE Oceny SET ocena = ?, data_oceny = ?, opis = ? WHERE id = ?");
                            pstmt->setDouble(1, newGrade);
                            pstmt->setString(2, newGradeDate);
                            pstmt->setString(3, newGradeDescription);
                            pstmt->setInt(4, gradeIdToEdit);
                            pstmt->executeUpdate();

                            cout << "Ocena została pomyślnie zaktualizowana." << endl;
                        }
                        else {
                            cout << "Nieprawidłowy wybór oceny." << endl;
                        }
                    }
                    else {
                        cout << "Uczeń o podanym użytkowniku nie istnieje." << endl;
                    }
                   // delete res;
                }
                else {
                    cout << "Nieprawidłowy wybór ucznia." << endl;
                }
            }
            else {
                cout << "Nieprawidłowy wybór przedmiotu." << endl;
            }

            if (pstmt != nullptr) {
                delete pstmt;
            }
        }
        catch (sql::SQLException& e) {
            cout << "Błąd SQL: " << e.what() << endl;
        }

        pressEnterToContinue();
        clearConsole();
    }

    void deleteGrade() {
        clearConsole();

        try {
            sql::PreparedStatement* pstmt = nullptr;
            pstmt = con->prepareStatement("SELECT p.id, p.nazwa FROM Przedmioty p JOIN Nauczyciele_Przedmioty np ON p.id = np.id_przedmiotu WHERE np.id_nauczyciela = ?");
            pstmt->setInt(1, teacherId);
            sql::ResultSet* res = pstmt->executeQuery();

            map<int, string> subjects;
            while (res->next()) {
                int subjectId = res->getInt("id");
                string subjectName = res->getString("nazwa");
                subjects[subjectId] = subjectName;
            }

            if (subjects.empty()) {
                cout << "Nie jesteś przypisany do żadnego przedmiotu." << endl;
                delete pstmt;
                delete res;
                return;
            }

            cout << "Wybierz przedmiot:" << endl;
            for (const auto& pair : subjects) {
                cout << pair.first << ". " << pair.second << endl;
            }

            int subjectChoice;
            cout << "Twój wybór: ";
            cin >> subjectChoice;

            if (subjects.find(subjectChoice) != subjects.end()) {
                int subjectId = subjectChoice;
                string subjectName = subjects[subjectChoice];

                string studentUsername;
                pstmt = con->prepareStatement("SELECT id, username FROM Uczniowie");
                res = pstmt->executeQuery();

                map<int, string> students;
                int studentCounter = 1;
                cout << "Dostępni uczniowie:" << endl;
                while (res->next()) {
                    int studentId = res->getInt("id");
                    string studentUsername = res->getString("username");
                    students[studentCounter] = studentUsername;
                    cout << studentCounter << ". " << studentUsername << endl;
                    studentCounter++;
                }
                delete res;

                if (students.empty()) {
                    cout << "Brak dostępnych uczniów." << endl;
                    delete pstmt;
                    return;
                }

                int studentChoice;
                cout << "Wybierz numer ucznia: ";
                cin >> studentChoice;

                if (students.find(studentChoice) != students.end()) {
                    string studentUsername = students[studentChoice];

                    int studentId;
                    pstmt = con->prepareStatement("SELECT id FROM Uczniowie WHERE username = ?");
                    pstmt->setString(1, studentUsername);
                    res = pstmt->executeQuery();
                    if (res->next()) {
                        studentId = res->getInt("id");

                        cout << "\nOceny ucznia:" << endl;
                        pstmt = con->prepareStatement(
                            "SELECT o.id, p.nazwa AS przedmiot, o.ocena, o.data_oceny, o.opis "
                            "FROM Oceny o "
                            "JOIN Przedmioty p ON o.id_przedmiotu = p.id "
                            "WHERE o.id_ucznia = ?");
                        pstmt->setInt(1, studentId);
                        res = pstmt->executeQuery();

                        map<int, int> grades;
                        int gradeCounter = 1;
                        while (res->next()) {
                            int gradeId = res->getInt("id");
                            string subjectName = res->getString("przedmiot");
                            double gradeValue = res->getDouble("ocena");
                            string gradeDate = res->getString("data_oceny");
                            string gradeDescription = res->getString("opis");
                            cout << gradeCounter << ". Przedmiot: " << subjectName << ", Ocena: " << gradeValue
                                << ", Data: " << gradeDate << ", Opis: " << gradeDescription << endl;
                            grades[gradeCounter] = gradeId;
                            gradeCounter++;
                        }

                        if (!grades.empty()) {
                            int gradeChoice;
                            cout << "Wybierz numer oceny do usunięcia: ";
                            cin >> gradeChoice;

                            if (grades.find(gradeChoice) != grades.end()) {
                                int gradeIdToDelete = grades[gradeChoice];

                                pstmt = con->prepareStatement("DELETE FROM Oceny WHERE id = ?");
                                pstmt->setInt(1, gradeIdToDelete);
                                pstmt->executeUpdate();

                                cout << "Ocena została usunięta pomyślnie." << endl;
                            }
                            else {
                                cout << "Nieprawidłowy wybór oceny." << endl;
                            }
                        }
                        else {
                            cout << "Ten uczeń nie ma jeszcze żadnych ocen." << endl;
                        }
                    }
                    else {
                        cout << "Uczeń o podanym użytkowniku nie istnieje." << endl;
                    }
                    delete res;
                }
                else {
                    cout << "Nieprawidłowy wybór ucznia." << endl;
                }
            }
            else {
                cout << "Nieprawidłowy wybór przedmiotu." << endl;
            }

            if (pstmt != nullptr) {
                delete pstmt;
            }
        }
        catch (sql::SQLException& e) {
            cout << "Błąd SQL: " << e.what() << endl;
        }

        pressEnterToContinue();
        clearConsole();
    }

    void addComment() {
        clearConsole();

        try {
            sql::PreparedStatement* pstmt = nullptr;
            pstmt = con->prepareStatement("SELECT id, username FROM Uczniowie");
            sql::ResultSet* res = pstmt->executeQuery();

            map<int, string> students;
            int studentCounter = 1;
            cout << "Dostępni uczniowie:" << endl;
            while (res->next()) {
                int studentId = res->getInt("id");
                string studentUsername = res->getString("username");
                students[studentCounter] = studentUsername;
                cout << studentCounter << ". " << studentUsername << endl;
                studentCounter++;
            }
            delete res;

            if (students.empty()) {
                cout << "Brak dostępnych uczniów." << endl;
                delete pstmt;
                return;
            }

            int studentChoice;
            cout << "Wybierz numer ucznia: ";
            cin >> studentChoice;

            if (students.find(studentChoice) != students.end()) {
                string studentUsername = students[studentChoice];

                int studentId;
                pstmt = con->prepareStatement("SELECT id FROM Uczniowie WHERE username = ?");
                pstmt->setString(1, studentUsername);
                res = pstmt->executeQuery();
                if (res->next()) {
                    studentId = res->getInt("id");

                    cin.ignore();
                    string commentContent;
                    cout << "Wpisz treść uwagi: ";
                    getline(cin, commentContent);

                    pstmt = con->prepareStatement("INSERT INTO Uwagi (id_ucznia, tresc) VALUES (?, ?)");
                    pstmt->setInt(1, studentId);
                    pstmt->setString(2, commentContent);
                    pstmt->executeUpdate();

                    cout << "Uwaga została dodana pomyślnie." << endl;
                }
                else {
                    cout << "Uczeń o podanym użytkowniku nie istnieje." << endl;
                }
                delete res;
            }
            else {
                cout << "Nieprawidłowy wybór ucznia." << endl;
            }

            if (pstmt != nullptr) {
                delete pstmt;
            }
        }
        catch (sql::SQLException& e) {
            cout << "Błąd SQL: " << e.what() << endl;
        }

        pressEnterToContinue();
        clearConsole();
    }

    void editComment() {
        clearConsole();

        try {
            sql::PreparedStatement* pstmt = nullptr;
            pstmt = con->prepareStatement("SELECT id, username FROM Uczniowie");
            sql::ResultSet* res = pstmt->executeQuery();

            map<int, string> students;
            int studentCounter = 1;
            cout << "Dostępni uczniowie:" << endl;
            while (res->next()) {
                int studentId = res->getInt("id");
                string studentUsername = res->getString("username");
                students[studentCounter] = studentUsername;
                cout << studentCounter << ". " << studentUsername << endl;
                studentCounter++;
            }
            delete res;

            if (students.empty()) {
                cout << "Brak dostępnych uczniów." << endl;
                delete pstmt;
                return;
            }

            int studentChoice;
            cout << "Wybierz numer ucznia: ";
            cin >> studentChoice;

            if (students.find(studentChoice) != students.end()) {
                string studentUsername = students[studentChoice];

                int studentId;
                pstmt = con->prepareStatement("SELECT id FROM Uczniowie WHERE username = ?");
                pstmt->setString(1, studentUsername);
                res = pstmt->executeQuery();
                if (res->next()) {
                    studentId = res->getInt("id");

                    pstmt = con->prepareStatement("SELECT id, tresc FROM Uwagi WHERE id_ucznia = ?");
                    pstmt->setInt(1, studentId);
                    res = pstmt->executeQuery();

                    map<int, pair<int, string>> comments;
                    int commentCounter = 1;
                    cout << "Uwagi ucznia:" << endl;
                    while (res->next()) {
                        int commentId = res->getInt("id");
                        string commentContent = res->getString("tresc");
                        comments[commentCounter] = make_pair(commentId, commentContent);
                        cout << commentCounter << ". " << commentContent << endl;
                        commentCounter++;
                    }
                    delete res;

                    if (comments.empty()) {
                        cout << "Ten uczeń nie ma jeszcze żadnych uwag." << endl;
                        delete pstmt;
                        return;
                    }

                    int commentChoice;
                    cout << "Wybierz numer uwagi do edycji: ";
                    cin >> commentChoice;

                    if (comments.find(commentChoice) != comments.end()) {
                        int commentIdToEdit = comments[commentChoice].first;

                        cin.ignore();
                        string newCommentContent;
                        cout << "Wpisz nową treść uwagi: ";
                        getline(cin, newCommentContent);

                        pstmt = con->prepareStatement("UPDATE Uwagi SET tresc = ? WHERE id = ?");
                        pstmt->setString(1, newCommentContent);
                        pstmt->setInt(2, commentIdToEdit);
                        pstmt->executeUpdate();

                        cout << "Uwaga została pomyślnie zaktualizowana." << endl;
                    }
                    else {
                        cout << "Nieprawidłowy wybór uwagi." << endl;
                    }
                }
                else {
                    cout << "Uczeń o podanym użytkowniku nie istnieje." << endl;
                }
                //delete res;
            }
            else {
                cout << "Nieprawidłowy wybór ucznia." << endl;
            }

            //if (pstmt != nullptr) {
            //    delete pstmt;
            //}
        }
        catch (sql::SQLException& e) {
            cout << "Błąd SQL: " << e.what() << endl;
        }

        pressEnterToContinue();
        clearConsole();
    }

    void deleteComment() {
        clearConsole();

        try {
            sql::PreparedStatement* pstmt = nullptr;
            pstmt = con->prepareStatement("SELECT id, username FROM Uczniowie");
            sql::ResultSet* res = pstmt->executeQuery();

            map<int, string> students;
            int studentCounter = 1;
            cout << "Dostępni uczniowie:" << endl;
            while (res->next()) {
                int studentId = res->getInt("id");
                string studentUsername = res->getString("username");
                students[studentCounter] = studentUsername;
                cout << studentCounter << ". " << studentUsername << endl;
                studentCounter++;
            }
            delete res;

            if (students.empty()) {
                cout << "Brak dostępnych uczniów." << endl;
                delete pstmt;
                return;
            }

            int studentChoice;
            cout << "Wybierz numer ucznia: ";
            cin >> studentChoice;

            if (students.find(studentChoice) != students.end()) {
                string studentUsername = students[studentChoice];

                int studentId;
                pstmt = con->prepareStatement("SELECT id FROM Uczniowie WHERE username = ?");
                pstmt->setString(1, studentUsername);
                res = pstmt->executeQuery();
                if (res->next()) {
                    studentId = res->getInt("id");

                    pstmt = con->prepareStatement("SELECT id, tresc FROM Uwagi WHERE id_ucznia = ?");
                    pstmt->setInt(1, studentId);
                    res = pstmt->executeQuery();

                    map<int, int> comments;
                    int commentCounter = 1;
                    cout << "Uwagi ucznia:" << endl;
                    while (res->next()) {
                        int commentId = res->getInt("id");
                        string commentContent = res->getString("tresc");
                        comments[commentCounter] = commentId;
                        cout << commentCounter << ". " << commentContent << endl;
                        commentCounter++;
                    }

                    if (!comments.empty()) {
                        int commentChoice;
                        cout << "Wybierz numer uwagi do usunięcia: ";
                        cin >> commentChoice;

                        if (comments.find(commentChoice) != comments.end()) {
                            int commentIdToDelete = comments[commentChoice];

                            pstmt = con->prepareStatement("DELETE FROM Uwagi WHERE id = ?");
                            pstmt->setInt(1, commentIdToDelete);
                            pstmt->executeUpdate();

                            cout << "Uwaga została usunięta pomyślnie." << endl;
                        }
                        else {
                            cout << "Nieprawidłowy wybór uwagi." << endl;
                        }
                    }
                    else {
                        cout << "Ten uczeń nie ma jeszcze żadnych uwag." << endl;
                    }
                }
                else {
                    cout << "Uczeń o podanym użytkowniku nie istnieje." << endl;
                }
                delete res;
            }
            else {
                cout << "Nieprawidłowy wybór ucznia." << endl;
            }

            if (pstmt != nullptr) {
                delete pstmt;
            }
        }
        catch (sql::SQLException& e) {
            cout << "Błąd SQL: " << e.what() << endl;
        }

        pressEnterToContinue();
        clearConsole();
    }

    void viewClassStudents() {
        clearConsole();

        try {
            sql::PreparedStatement* pstmt = nullptr;
            pstmt = con->prepareStatement("SELECT id, nazwa FROM Klasy");
            sql::ResultSet* res = pstmt->executeQuery();

            map<int, string> classes;
            int classCounter = 1;
            cout << "Dostępne klasy:" << endl;
            while (res->next()) {
                int classId = res->getInt("id");
                string className = res->getString("nazwa");
                classes[classCounter] = className;
                cout << classCounter << ". " << className << endl;
                classCounter++;
            }
            delete res;

            if (classes.empty()) {
                cout << "Brak dostępnych klas." << endl;
                delete pstmt;
                return;
            }

            int classChoice;
            cout << "Wybierz numer klasy: ";
            cin >> classChoice;

            if (classes.find(classChoice) != classes.end()) {
                clearConsole();

                string className = classes[classChoice];

                pstmt = con->prepareStatement("SELECT id FROM Klasy WHERE nazwa = ?");
                pstmt->setString(1, className);
                res = pstmt->executeQuery();

                if (res->next()) {
                    int classId = res->getInt("id");

                    delete pstmt;
                    pstmt = con->prepareStatement("SELECT o.id, o.imie, o.nazwisko FROM Osoby o JOIN Klasy_Uczniowie ku ON o.id = ku.id_ucznia WHERE ku.id_klasy = ?");
                    pstmt->setInt(1, classId);
                    res = pstmt->executeQuery();

                    map<int, pair<int, pair<string, string>>> students;
                    int counter = 1;
                    while (res->next()) {
                        int studentId = res->getInt("id");
                        string studentName = res->getString("imie");
                        string studentSurname = res->getString("nazwisko");
                        students[counter] = make_pair(studentId, make_pair(studentName, studentSurname));
                        cout << counter << ". " << studentName << " " << studentSurname << endl;
                        counter++;
                    }

                    if (students.empty()) {
                        cout << "W tej klasie nie ma żadnych uczniów." << endl;
                        delete pstmt;
                        delete res;
                        return;
                    }

                    int choice;
                    cout << "Wybierz numer ucznia, aby zobaczyć szczegóły: ";
                    cin >> choice;

                    if (students.find(choice) != students.end()) {
                        clearConsole();

                        int studentId = students[choice].first;

                        pstmt = con->prepareStatement("SELECT * FROM Osoby WHERE id = ?");
                        pstmt->setInt(1, studentId);
                        res = pstmt->executeQuery();

                        if (res->next()) {
                            cout << "Informacje o uczniu:" << endl;
                            cout << "Imię: " << res->getString("imie") << endl;
                            cout << "Nazwisko: " << res->getString("nazwisko") << endl;
                            cout << "Data urodzenia: " << res->getString("data_urodzenia") << endl;
                            cout << "PESEL: " << res->getString("pesel") << endl;
                            cout << "Adres: " << res->getString("adres") << endl;
                            cout << "Telefon: " << res->getString("telefon") << endl;
                            cout << "Email: " << res->getString("email") << endl;
                        }
                        delete res;

                        cout << "\nOceny ucznia:" << endl;
                        pstmt = con->prepareStatement(
                            "SELECT p.nazwa, o.ocena, o.data_oceny, o.opis FROM Oceny o "
                            "JOIN Przedmioty p ON o.id_przedmiotu = p.id WHERE o.id_ucznia = ?");
                        pstmt->setInt(1, studentId);
                        res = pstmt->executeQuery();

                        while (res->next()) {
                            cout << "Przedmiot: " << res->getString("nazwa") << ", Ocena: " << res->getDouble("ocena")
                                << ", Data: " << res->getString("data_oceny") << ", Opis: " << res->getString("opis") << endl;
                        }
                        delete res;

                        cout << "\nUwagi ucznia:" << endl;
                        pstmt = con->prepareStatement("SELECT * FROM Uwagi WHERE id_ucznia = ?");
                        pstmt->setInt(1, studentId);
                        res = pstmt->executeQuery();

                        while (res->next()) {
                            cout << "Uwaga: " << res->getString("tresc") << endl;
                        }
                        delete res;
                    }
                    else {
                        cout << "Nieprawidłowy wybór ucznia." << endl;
                    }
                }
                else {
                    cout << "Wystąpił błąd podczas pobierania danych o klasie." << endl;
                }
            }
            else {
                cout << "Nieprawidłowy wybór klasy." << endl;
            }

            if (pstmt != nullptr) {
                delete pstmt;
            }
        }
        catch (sql::SQLException& e) {
            cout << "Błąd SQL: " << e.what() << endl;
        }

        pressEnterToContinue();
        clearConsole();
    }

    void teacherMenu() {
        int choice;
        do {
            clearConsole();

            cout << "\n--- Menu Nauczyciela ---\n";
            cout << "1. Dodaj ocenę\n";
            cout << "2. Edytuj ocenę\n";
            cout << "3. Usuń ocenę\n";
            cout << "4. Dodaj uwagę\n";
            cout << "5. Edytuj uwagę\n";
            cout << "6. Usuń uwagę\n";
            cout << "7. Wyświetl uczniów klasy\n";
            cout << "8. Wyloguj\n";
            cout << "Twój wybór: ";
            cin >> choice;

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
                cout << "Nieprawidłowy wybór. Spróbuj ponownie." << endl;
            }
        } while (true);
    }
};

class Admin : public User {
public:
    Admin(sql::Connection* connection) : User(connection) {}

    bool login() override {
        clearConsole();

        string adminUsername, adminPassword;
        cout << "Wpisz swoją nazwę użytkownika: ";
        cin >> adminUsername;
        cout << "Podaj hasło: ";
        cin >> adminPassword;

        try {
            sql::PreparedStatement* pstmt;
            pstmt = con->prepareStatement("SELECT id FROM Admin WHERE username=? AND password=?");
            pstmt->setString(1, adminUsername);
            pstmt->setString(2, adminPassword);
            sql::ResultSet* res = pstmt->executeQuery();

            if (res->next()) {
                cout << "Pomyślnie zalogowano jako administrator." << endl;
                delete pstmt;
                delete res;
                pressEnterToContinue();
                clearConsole();
                return true;
            }
            else {
                cout << "Logowanie nie powiodło się. Niepoprawna nazwa użytkownika lub hasło." << endl;
                delete pstmt;
                delete res;
                pressEnterToContinue();
                clearConsole();
                return false;
            }
        }
        catch (sql::SQLException& e) {
            cout << "Błąd SQL: " << e.what() << endl;
            return false;
        }
    }

    void addTeacher() {
        clearConsole();

        string teacherName, teacherSurname, teacherDob, teacherPesel, teacherAddress, teacherPhone, teacherEmail, teacherUsername, teacherPassword;
        cout << "Wpisz imię nauczyciela: ";
        cin >> teacherName;
        cout << "Wpisz nazwisko nauczyciela: ";
        cin >> teacherSurname;
        cout << "Wpisz datę urodzenia nauczyciela (RRRR-MM-DD): ";
        cin >> teacherDob;
        cout << "Wpisz PESEL nauczyciela: ";
        cin >> teacherPesel;
        cout << "Wpisz adres nauczyciela: ";
        cin.ignore();
        getline(cin, teacherAddress);
        cout << "Wpisz telefon nauczyciela: ";
        cin >> teacherPhone;
        cout << "Wpisz email nauczyciela: ";
        cin >> teacherEmail;
        cout << "Wpisz nazwę użytkownika nauczyciela: ";
        cin >> teacherUsername;
        cout << "Wpisz hasło nauczyciela: ";
        cin >> teacherPassword;

        try {
            sql::PreparedStatement* pstmt;
            pstmt = con->prepareStatement("INSERT INTO Osoby (imie, nazwisko, data_urodzenia, pesel, adres, telefon, email) VALUES (?, ?, ?, ?, ?, ?, ?)");
            pstmt->setString(1, teacherName);
            pstmt->setString(2, teacherSurname);
            pstmt->setString(3, teacherDob);
            pstmt->setString(4, teacherPesel);
            pstmt->setString(5, teacherAddress);
            pstmt->setString(6, teacherPhone);
            pstmt->setString(7, teacherEmail);
            pstmt->executeUpdate();

            pstmt = con->prepareStatement("SELECT id FROM Osoby WHERE pesel=?");
            pstmt->setString(1, teacherPesel);
            sql::ResultSet* res = pstmt->executeQuery();

            int teacherId;
            if (res->next()) {
                teacherId = res->getInt("id");
                pstmt = con->prepareStatement("INSERT INTO Nauczyciele (id, username, password) VALUES (?, ?, ?)");
                pstmt->setInt(1, teacherId);
                pstmt->setString(2, teacherUsername);
                pstmt->setString(3, teacherPassword);
                pstmt->executeUpdate();

                cout << "Nauczyciel został dodany pomyślnie." << endl;
            }

            delete pstmt;
            delete res;
        }
        catch (sql::SQLException& e) {
            cout << "Błąd SQL: " << e.what() << endl;
        }
    }

    void editTeacher() {
        clearConsole();

        try {
            sql::PreparedStatement* pstmt = con->prepareStatement("SELECT id, username FROM Nauczyciele");
            sql::ResultSet* res = pstmt->executeQuery();

            map<int, string> teachers;
            int teacherCounter = 1;
            cout << "Dostępni nauczyciele:" << endl;
            while (res->next()) {
                int teacherId = res->getInt("id");
                string teacherUsername = res->getString("username");
                teachers[teacherCounter] = teacherUsername;
                cout << teacherCounter << ". " << teacherUsername << endl;
                teacherCounter++;
            }
            delete res;

            if (teachers.empty()) {
                cout << "Brak dostępnych nauczycieli." << endl;
                delete pstmt;
                return;
            }

            int teacherChoice;
            cout << "Wybierz numer nauczyciela do edycji: ";
            cin >> teacherChoice;

            if (teachers.find(teacherChoice) != teachers.end()) {
                string teacherUsername = teachers[teacherChoice];

                int teacherId;
                pstmt = con->prepareStatement("SELECT id FROM Nauczyciele WHERE username = ?");
                pstmt->setString(1, teacherUsername);
                res = pstmt->executeQuery();
                if (res->next()) {
                    teacherId = res->getInt("id");

                    string teacherName, teacherSurname, teacherDob, teacherPesel, teacherAddress, teacherPhone, teacherEmail, newTeacherUsername, newTeacherPassword;
                    cout << "Wpisz nowe imię nauczyciela (pozostaw puste, aby nie zmieniać): ";
                    cin.ignore();
                    getline(cin, teacherName);
                    cout << "Wpisz nowe nazwisko nauczyciela (pozostaw puste, aby nie zmieniać): ";
                    getline(cin, teacherSurname);
                    cout << "Wpisz nową datę urodzenia nauczyciela (pozostaw puste, aby nie zmieniać): ";
                    getline(cin, teacherDob);
                    cout << "Wpisz nowy PESEL nauczyciela (pozostaw puste, aby nie zmieniać): ";
                    getline(cin, teacherPesel);
                    cout << "Wpisz nowy adres nauczyciela (pozostaw puste, aby nie zmieniać): ";
                    getline(cin, teacherAddress);
                    cout << "Wpisz nowy telefon nauczyciela (pozostaw puste, aby nie zmieniać): ";
                    getline(cin, teacherPhone);
                    cout << "Wpisz nowy email nauczyciela (pozostaw puste, aby nie zmieniać): ";
                    getline(cin, teacherEmail);
                    cout << "Wpisz nową nazwę użytkownika nauczyciela (pozostaw puste, aby nie zmieniać): ";
                    getline(cin, newTeacherUsername);
                    cout << "Wpisz nowe hasło nauczyciela (pozostaw puste, aby nie zmieniać): ";
                    getline(cin, newTeacherPassword);

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

                    cout << "Dane nauczyciela zostały pomyślnie zaktualizowane." << endl;
                }
                else {
                    cout << "Nauczyciel o podanym użytkowniku nie istnieje." << endl;
                }
                delete res;
            }
            else {
                cout << "Nieprawidłowy wybór nauczyciela." << endl;
            }

            if (pstmt != nullptr) {
                delete pstmt;
            }
        }
        catch (sql::SQLException& e) {
            cout << "Błąd SQL: " << e.what() << endl;
        }

        pressEnterToContinue();
        clearConsole();
    }

    void removeTeacher() {
        clearConsole();

        try {
            sql::PreparedStatement* pstmt = nullptr;
            sql::ResultSet* res = nullptr;

            pstmt = con->prepareStatement("SELECT id, username FROM Nauczyciele");
            res = pstmt->executeQuery();

            map<int, string> teachers;
            int teacherCounter = 1;
            cout << "Dostępni nauczyciele:" << endl;
            while (res->next()) {
                int teacherId = res->getInt("id");
                string teacherUsername = res->getString("username");
                teachers[teacherCounter] = teacherUsername;
                cout << teacherCounter << ". " << teacherUsername << endl;
                teacherCounter++;
            }
            delete res;

            if (teachers.empty()) {
                cout << "Brak dostępnych nauczycieli." << endl;
                delete pstmt;
                return;
            }

            int teacherChoice;
            cout << "Wybierz numer nauczyciela do usunięcia: ";
            cin >> teacherChoice;

            if (teachers.find(teacherChoice) == teachers.end()) {
                cout << "Nieprawidłowy wybór nauczyciela." << endl;
                delete pstmt;
                return;
            }

            string teacherUsername = teachers[teacherChoice];

            pstmt = con->prepareStatement("DELETE FROM Nauczyciele WHERE username = ?");
            pstmt->setString(1, teacherUsername);

            int updateCount = pstmt->executeUpdate();

            if (updateCount > 0) {
                cout << "Nauczyciel " << teacherUsername << " został usunięty." << endl;
            }
            else {
                cout << "Nie udało się usunąć nauczyciela." << endl;
            }

            if (pstmt != nullptr) {
                delete pstmt;
            }
        }
        catch (sql::SQLException& e) {
            cout << "Błąd SQL: " << e.what() << endl;
        }

        pressEnterToContinue();
        clearConsole();
    }

    void addStudent() {
        clearConsole();

        string studentName, studentSurname, studentDob, studentPesel, studentAddress, studentPhone, studentEmail, studentUsername, studentPassword;
        cout << "Wpisz imię ucznia: ";
        cin >> studentName;
        cout << "Wpisz nazwisko ucznia: ";
        cin >> studentSurname;
        cout << "Wpisz datę urodzenia ucznia (RRRR-MM-DD): ";
        cin >> studentDob;
        cout << "Wpisz PESEL ucznia: ";
        cin >> studentPesel;
        cout << "Wpisz adres ucznia: ";
        cin.ignore();
        getline(cin, studentAddress);
        cout << "Wpisz telefon ucznia: ";
        cin >> studentPhone;
        cout << "Wpisz email ucznia: ";
        cin >> studentEmail;
        cout << "Wpisz nazwę użytkownika ucznia: ";
        cin >> studentUsername;
        cout << "Wpisz hasło ucznia: ";
        cin >> studentPassword;

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

                cout << "Uczeń został dodany pomyślnie." << endl;
            }

            delete pstmt;
            delete res;
        }
        catch (sql::SQLException& e) {
            cout << "Błąd SQL: " << e.what() << endl;
        }
    }

    void editStudent() {
        clearConsole();

        try {
            sql::PreparedStatement* pstmt = con->prepareStatement("SELECT id, username FROM Uczniowie");
            sql::ResultSet* res = pstmt->executeQuery();

            map<int, string> students;
            int studentCounter = 1;
            cout << "Dostępni uczniowie:" << endl;
            while (res->next()) {
                int studentId = res->getInt("id");
                string studentUsername = res->getString("username");
                students[studentCounter] = studentUsername;
                cout << studentCounter << ". " << studentUsername << endl;
                studentCounter++;
            }
            delete res;

            if (students.empty()) {
                cout << "Brak dostępnych uczniów." << endl;
                delete pstmt;
                return;
            }

            int studentChoice;
            cout << "Wybierz numer ucznia do edycji: ";
            cin >> studentChoice;

            if (students.find(studentChoice) != students.end()) {
                string studentUsername = students[studentChoice];

                int studentId;
                pstmt = con->prepareStatement("SELECT id FROM Uczniowie WHERE username = ?");
                pstmt->setString(1, studentUsername);
                res = pstmt->executeQuery();
                if (res->next()) {
                    studentId = res->getInt("id");

                    string studentName, studentSurname, studentDob, studentPesel, studentAddress, studentPhone, studentEmail, newStudentUsername, newStudentPassword;
                    cout << "Wpisz nowe imię ucznia (pozostaw puste, aby nie zmieniać): ";
                    cin.ignore();
                    getline(cin, studentName);
                    cout << "Wpisz nowe nazwisko ucznia (pozostaw puste, aby nie zmieniać): ";
                    getline(cin, studentSurname);
                    cout << "Wpisz nową datę urodzenia ucznia (pozostaw puste, aby nie zmieniać): ";
                    getline(cin, studentDob);
                    cout << "Wpisz nowy PESEL ucznia (pozostaw puste, aby nie zmieniać): ";
                    getline(cin, studentPesel);
                    cout << "Wpisz nowy adres ucznia (pozostaw puste, aby nie zmieniać): ";
                    getline(cin, studentAddress);
                    cout << "Wpisz nowy telefon ucznia (pozostaw puste, aby nie zmieniać): ";
                    getline(cin, studentPhone);
                    cout << "Wpisz nowy email ucznia (pozostaw puste, aby nie zmieniać): ";
                    getline(cin, studentEmail);
                    cout << "Wpisz nową nazwę użytkownika ucznia (pozostaw puste, aby nie zmieniać): ";
                    getline(cin, newStudentUsername);
                    cout << "Wpisz nowe hasło ucznia (pozostaw puste, aby nie zmieniać): ";
                    getline(cin, newStudentPassword);

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

                    cout << "Dane ucznia zostały pomyślnie zaktualizowane." << endl;
                }
                else {
                    cout << "Uczeń o podanym użytkowniku nie istnieje." << endl;
                }
                delete res;
            }
            else {
                cout << "Nieprawidłowy wybór ucznia." << endl;
            }

            if (pstmt != nullptr) {
                delete pstmt;
            }
        }
        catch (sql::SQLException& e) {
            cout << "Błąd SQL: " << e.what() << endl;
        }

        pressEnterToContinue();
        clearConsole();
    }

    void removeStudent() {
        clearConsole();

        try {
            sql::PreparedStatement* pstmt = nullptr;
            sql::ResultSet* res = nullptr;

            pstmt = con->prepareStatement("SELECT id, username FROM Uczniowie");
            res = pstmt->executeQuery();

            map<int, string> students;
            int studentCounter = 1;
            cout << "Dostępni uczniowie:" << endl;
            while (res->next()) {
                int studentId = res->getInt("id");
                string studentUsername = res->getString("username");
                students[studentCounter] = studentUsername;
                cout << studentCounter << ". " << studentUsername << endl;
                studentCounter++;
            }
            delete res;

            if (students.empty()) {
                cout << "Brak dostępnych uczniów." << endl;
                delete pstmt;
                return;
            }

            int studentChoice;
            cout << "Wybierz numer ucznia do usunięcia: ";
            cin >> studentChoice;

            if (students.find(studentChoice) == students.end()) {
                cout << "Nieprawidłowy wybór ucznia." << endl;
                delete pstmt;
                return;
            }

            string studentUsername = students[studentChoice];

            pstmt = con->prepareStatement("DELETE FROM Uczniowie WHERE username = ?");
            pstmt->setString(1, studentUsername);

            int updateCount = pstmt->executeUpdate();

            if (updateCount > 0) {
                cout << "Uczeń " << studentUsername << " został usunięty." << endl;
            }
            else {
                cout << "Nie udało się usunąć ucznia." << endl;
            }

            if (pstmt != nullptr) {
                delete pstmt;
            }
        }
        catch (sql::SQLException& e) {
            cout << "Błąd SQL: " << e.what() << endl;
        }

        pressEnterToContinue();
        clearConsole();
    }

    void createClass() {
        clearConsole();

        string className;
        cout << "Wpisz nazwę klasy: ";
        cin >> className;

        try {
            sql::PreparedStatement* pstmt;
            pstmt = con->prepareStatement("INSERT INTO Klasy (nazwa) VALUES (?)");
            pstmt->setString(1, className);
            pstmt->executeUpdate();

            cout << "Klasa została dodana pomyślnie." << endl;

            delete pstmt;
        }
        catch (sql::SQLException& e) {
            cout << "Błąd SQL: " << e.what() << endl;
        }
    }

    void editClass() {
        clearConsole();

        try {
            sql::PreparedStatement* pstmt = con->prepareStatement("SELECT id, nazwa FROM Klasy");
            sql::ResultSet* res = pstmt->executeQuery();

            map<int, string> classes;
            int classCounter = 1;
            cout << "Dostępne klasy:" << endl;
            while (res->next()) {
                int classId = res->getInt("id");
                string className = res->getString("nazwa");
                classes[classCounter] = className;
                cout << classCounter << ". " << className << endl;
                classCounter++;
            }
            delete res;

            if (classes.empty()) {
                cout << "Brak dostępnych klas." << endl;
                delete pstmt;
                return;
            }

            int classChoice;
            cout << "Wybierz numer klasy do edycji: ";
            cin >> classChoice;

            if (classes.find(classChoice) != classes.end()) {
                string className = classes[classChoice];

                int classId;
                pstmt = con->prepareStatement("SELECT id FROM Klasy WHERE nazwa = ?");
                pstmt->setString(1, className);
                res = pstmt->executeQuery();
                if (res->next()) {
                    classId = res->getInt("id");

                    string newClassName;
                    cout << "Wpisz nową nazwę klasy (pozostaw puste, aby nie zmieniać): ";
                    cin.ignore();
                    getline(cin, newClassName);

                    if (!newClassName.empty()) {
                        pstmt = con->prepareStatement("UPDATE Klasy SET nazwa = ? WHERE id = ?");
                        pstmt->setString(1, newClassName);
                        pstmt->setInt(2, classId);
                        pstmt->executeUpdate();

                        cout << "Nazwa klasy została pomyślnie zaktualizowana." << endl;
                    }
                    else {
                        cout << "Nie wprowadzono nowej nazwy klasy." << endl;
                    }
                }
                else {
                    cout << "Klasa o podanej nazwie nie istnieje." << endl;
                }
                delete res;
            }
            else {
                cout << "Nieprawidłowy wybór klasy." << endl;
            }

            if (pstmt != nullptr) {
                delete pstmt;
            }
        }
        catch (sql::SQLException& e) {
            cout << "Błąd SQL: " << e.what() << endl;
        }

        pressEnterToContinue();
        clearConsole();
    }

    void removeClass() {
        clearConsole();

        try {
            sql::PreparedStatement* pstmt = nullptr;
            sql::ResultSet* res = nullptr;

            pstmt = con->prepareStatement("SELECT id, nazwa FROM Klasy");
            res = pstmt->executeQuery();

            map<int, string> classes;
            int classCounter = 1;
            cout << "Dostępne klasy:" << endl;
            while (res->next()) {
                int classId = res->getInt("id");
                string className = res->getString("nazwa");
                classes[classCounter] = className;
                cout << classCounter << ". " << className << endl;
                classCounter++;
            }
            delete res;

            if (classes.empty()) {
                cout << "Brak dostępnych klas." << endl;
                delete pstmt;
                return;
            }

            int classChoice;
            cout << "Wybierz numer klasy do usunięcia: ";
            cin >> classChoice;

            if (classes.find(classChoice) == classes.end()) {
                cout << "Nieprawidłowy wybór klasy." << endl;
                delete pstmt;
                return;
            }

            string className = classes[classChoice];

            pstmt = con->prepareStatement("DELETE FROM Klasy WHERE nazwa = ?");
            pstmt->setString(1, className);

            int updateCount = pstmt->executeUpdate();

            if (updateCount > 0) {
                cout << "Klasa " << className << " została usunięta." << endl;
            }
            else {
                cout << "Nie udało się usunąć klasy." << endl;
            }

            if (pstmt != nullptr) {
                delete pstmt;
            }
        }
        catch (sql::SQLException& e) {
            cout << "Błąd SQL: " << e.what() << endl;
        }

        pressEnterToContinue();
        clearConsole();
    }

    void addSubject() {
        clearConsole();

        try {
            string subjectName;
            cout << "Podaj nazwę nowego przedmiotu: ";
            cin.ignore();
            getline(cin, subjectName);

            sql::PreparedStatement* pstmt = con->prepareStatement("INSERT INTO Przedmioty (nazwa) VALUES (?)");
            pstmt->setString(1, subjectName);

            int updateCount = pstmt->executeUpdate();

            if (updateCount > 0) {
                cout << "Przedmiot " << subjectName << " został dodany." << endl;
            }
            else {
                cout << "Nie udało się dodać przedmiotu." << endl;
            }

            delete pstmt;
        }
        catch (sql::SQLException& e) {
            cout << "Błąd SQL: " << e.what() << endl;
        }

        pressEnterToContinue();
        clearConsole();
    }

    void editSubject() {
        clearConsole();

        try {
            sql::PreparedStatement* pstmt = con->prepareStatement("SELECT id, nazwa FROM Przedmioty");
            sql::ResultSet* res = pstmt->executeQuery();

            map<int, string> subjects;
            int subjectCounter = 1;
            cout << "Dostępne przedmioty:" << endl;
            while (res->next()) {
                int subjectId = res->getInt("id");
                string subjectName = res->getString("nazwa");
                subjects[subjectCounter] = subjectName;
                cout << subjectCounter << ". " << subjectName << endl;
                subjectCounter++;
            }
            delete res;

            if (subjects.empty()) {
                cout << "Brak dostępnych przedmiotów." << endl;
                delete pstmt;
                return;
            }

            int subjectChoice;
            cout << "Wybierz numer przedmiotu do edycji: ";
            cin >> subjectChoice;

            if (subjects.find(subjectChoice) != subjects.end()) {
                string subjectName = subjects[subjectChoice];

                int subjectId;
                pstmt = con->prepareStatement("SELECT id FROM Przedmioty WHERE nazwa = ?");
                pstmt->setString(1, subjectName);
                res = pstmt->executeQuery();
                if (res->next()) {
                    subjectId = res->getInt("id");

                    string newSubjectName;
                    cout << "Wpisz nową nazwę przedmiotu (pozostaw puste, aby nie zmieniać): ";
                    cin.ignore();
                    getline(cin, newSubjectName);

                    if (!newSubjectName.empty()) {
                        pstmt = con->prepareStatement("UPDATE Przedmioty SET nazwa = ? WHERE id = ?");
                        pstmt->setString(1, newSubjectName);
                        pstmt->setInt(2, subjectId);
                        pstmt->executeUpdate();

                        cout << "Nazwa przedmiotu została pomyślnie zaktualizowana." << endl;
                    }
                    else {
                        cout << "Nie wprowadzono nowej nazwy przedmiotu." << endl;
                    }
                }
                else {
                    cout << "Przedmiot o podanej nazwie nie istnieje." << endl;
                }
                delete res;
            }
            else {
                cout << "Nieprawidłowy wybór przedmiotu." << endl;
            }

            if (pstmt != nullptr) {
                delete pstmt;
            }
        }
        catch (sql::SQLException& e) {
            cout << "Błąd SQL: " << e.what() << endl;
        }

        pressEnterToContinue();
        clearConsole();
    }

    void removeSubject() {
        clearConsole();

        try {
            sql::PreparedStatement* pstmt = nullptr;
            sql::ResultSet* res = nullptr;

            pstmt = con->prepareStatement("SELECT id, nazwa FROM Przedmioty");
            res = pstmt->executeQuery();

            map<int, string> subjects;
            int subjectCounter = 1;
            cout << "Dostępne przedmioty:" << endl;
            while (res->next()) {
                int subjectId = res->getInt("id");
                string subjectName = res->getString("nazwa");
                subjects[subjectCounter] = subjectName;
                cout << subjectCounter << ". " << subjectName << endl;
                subjectCounter++;
            }
            delete res;

            if (subjects.empty()) {
                cout << "Brak dostępnych przedmiotów." << endl;
                delete pstmt;
                return;
            }

            int subjectChoice;
            cout << "Wybierz numer przedmiotu do usunięcia: ";
            cin >> subjectChoice;

            if (subjects.find(subjectChoice) == subjects.end()) {
                cout << "Nieprawidłowy wybór przedmiotu." << endl;
                delete pstmt;
                return;
            }

            string subjectName = subjects[subjectChoice];

            pstmt = con->prepareStatement("DELETE FROM Przedmioty WHERE nazwa = ?");
            pstmt->setString(1, subjectName);

            int updateCount = pstmt->executeUpdate();

            if (updateCount > 0) {
                cout << "Przedmiot " << subjectName << " został usunięty." << endl;
            }
            else {
                cout << "Nie udało się usunąć przedmiotu." << endl;
            }

            if (pstmt != nullptr) {
                delete pstmt;
            }
        }
        catch (sql::SQLException& e) {
            cout << "Błąd SQL: " << e.what() << endl;
        }

        pressEnterToContinue();
        clearConsole();
    }

    void assignTeacherToClass() {
        clearConsole();

        try {
            sql::PreparedStatement* pstmt = nullptr;
            sql::ResultSet* res = nullptr;

            pstmt = con->prepareStatement("SELECT id, username FROM Nauczyciele");
            res = pstmt->executeQuery();

            map<int, string> teachers;
            int teacherCounter = 1;
            cout << "Dostępni nauczyciele:" << endl;
            while (res->next()) {
                int teacherId = res->getInt("id");
                string teacherUsername = res->getString("username");
                teachers[teacherCounter] = teacherUsername;
                cout << teacherCounter << ". " << teacherUsername << endl;
                teacherCounter++;
            }
            delete res;

            if (teachers.empty()) {
                cout << "Brak dostępnych nauczycieli." << endl;
                delete pstmt;
                return;
            }

            int teacherChoice;
            cout << "Wybierz numer nauczyciela: ";
            cin >> teacherChoice;

            if (teachers.find(teacherChoice) == teachers.end()) {
                cout << "Nieprawidłowy wybór nauczyciela." << endl;
                delete pstmt;
                return;
            }

            string teacherUsername = teachers[teacherChoice];

            pstmt = con->prepareStatement("SELECT id, nazwa FROM Klasy");
            res = pstmt->executeQuery();

            map<int, string> classes;
            int classCounter = 1;
            cout << "Dostępne klasy:" << endl;
            while (res->next()) {
                int classId = res->getInt("id");
                string className = res->getString("nazwa");
                classes[classCounter] = className;
                cout << classCounter << ". " << className << endl;
                classCounter++;
            }
            delete res;

            if (classes.empty()) {
                cout << "Brak dostępnych klas." << endl;
                delete pstmt;
                return;
            }

            int classChoice;
            cout << "Wybierz numer klasy: ";
            cin >> classChoice;

            if (classes.find(classChoice) == classes.end()) {
                cout << "Nieprawidłowy wybór klasy." << endl;
                delete pstmt;
                return;
            }

            string className = classes[classChoice];

            pstmt = con->prepareStatement("INSERT INTO Nauczyciele_Klasy (id_nauczyciela, id_klasy) SELECT Nauczyciele.id, Klasy.id FROM Nauczyciele, Klasy WHERE Nauczyciele.username = ? AND Klasy.nazwa = ?");
            pstmt->setString(1, teacherUsername);
            pstmt->setString(2, className);

            int updateCount = pstmt->executeUpdate();

            if (updateCount > 0) {
                cout << "Nauczyciel " << teacherUsername << " został przypisany do klasy " << className << "." << endl;
            }
            else {
                cout << "Nie udało się przypisać nauczyciela do klasy." << endl;
            }

            if (pstmt != nullptr) {
                delete pstmt;
            }
        }
        catch (sql::SQLException& e) {
            cout << "Błąd SQL: " << e.what() << endl;
        }

        pressEnterToContinue();
        clearConsole();
    }

    void assignTeacherToSubject() {
        clearConsole();

        try {
            sql::PreparedStatement* pstmt = nullptr;
            sql::ResultSet* res = nullptr;

            pstmt = con->prepareStatement("SELECT id, username FROM Nauczyciele");
            res = pstmt->executeQuery();

            map<int, string> teachers;
            int teacherCounter = 1;
            cout << "Dostępni nauczyciele:" << endl;
            while (res->next()) {
                int teacherId = res->getInt("id");
                string teacherUsername = res->getString("username");
                teachers[teacherCounter] = teacherUsername;
                cout << teacherCounter << ". " << teacherUsername << endl;
                teacherCounter++;
            }
            delete res;

            if (teachers.empty()) {
                cout << "Brak dostępnych nauczycieli." << endl;
                delete pstmt;
                return;
            }

            int teacherChoice;
            cout << "Wybierz numer nauczyciela: ";
            cin >> teacherChoice;

            if (teachers.find(teacherChoice) == teachers.end()) {
                cout << "Nieprawidłowy wybór nauczyciela." << endl;
                delete pstmt;
                return;
            }

            string teacherUsername = teachers[teacherChoice];

            pstmt = con->prepareStatement("SELECT id, nazwa FROM Przedmioty");
            res = pstmt->executeQuery();

            map<int, string> subjects;
            int subjectCounter = 1;
            cout << "Dostępne przedmioty:" << endl;
            while (res->next()) {
                int subjectId = res->getInt("id");
                string subjectName = res->getString("nazwa");
                subjects[subjectCounter] = subjectName;
                cout << subjectCounter << ". " << subjectName << endl;
                subjectCounter++;
            }
            delete res;

            if (subjects.empty()) {
                cout << "Brak dostępnych przedmiotów." << endl;
                delete pstmt;
                return;
            }

            int subjectChoice;
            cout << "Wybierz numer przedmiotu: ";
            cin >> subjectChoice;

            if (subjects.find(subjectChoice) == subjects.end()) {
                cout << "Nieprawidłowy wybór przedmiotu." << endl;
                delete pstmt;
                return;
            }

            string subjectName = subjects[subjectChoice];

            pstmt = con->prepareStatement("INSERT INTO Nauczyciele_Przedmioty (id_nauczyciela, id_przedmiotu) SELECT Nauczyciele.id, Przedmioty.id FROM Nauczyciele, Przedmioty WHERE Nauczyciele.username = ? AND Przedmioty.nazwa = ?");
            pstmt->setString(1, teacherUsername);
            pstmt->setString(2, subjectName);

            int updateCount = pstmt->executeUpdate();

            if (updateCount > 0) {
                cout << "Nauczyciel " << teacherUsername << " został przypisany do przedmiotu " << subjectName << "." << endl;
            }
            else {
                cout << "Nie udało się przypisać nauczyciela do przedmiotu." << endl;
            }

            if (pstmt != nullptr) {
                delete pstmt;
            }
        }
        catch (sql::SQLException& e) {
            cout << "Błąd SQL: " << e.what() << endl;
        }

        pressEnterToContinue();
        clearConsole();
    }

    void assignStudentToClass() {
        clearConsole();

        try {
            // Pobierz listę wszystkich uczniów
            sql::PreparedStatement* pstmt;
            pstmt = con->prepareStatement("SELECT id, imie, nazwisko FROM Osoby WHERE id IN (SELECT id FROM Uczniowie)");
            sql::ResultSet* res = pstmt->executeQuery();

            map<int, pair<int, string>> students;
            int counter = 1;
            cout << "Wybierz ucznia:" << endl;
            while (res->next()) {
                int studentId = res->getInt("id");
                string studentName = res->getString("imie");
                string studentSurname = res->getString("nazwisko");
                students[counter] = make_pair(studentId, studentName + " " + studentSurname);
                cout << counter << ". " << studentName << " " << studentSurname << endl;
                counter++;
            }
            delete pstmt;
            delete res;

            // Wybierz ucznia
            int studentChoice;
            cout << "Podaj numer ucznia: ";
            cin >> studentChoice;

            if (students.find(studentChoice) == students.end()) {
                cout << "Nieprawidłowy wybór ucznia." << endl;
                return;
            }

            int studentId = students[studentChoice].first;
            string studentFullName = students[studentChoice].second;

            // Pobierz listę klas, do których ucznik jeszcze nie jest przypisany
            pstmt = con->prepareStatement("SELECT id, nazwa FROM Klasy WHERE id NOT IN (SELECT id_klasy FROM Klasy_Uczniowie WHERE id_ucznia=?)");
            pstmt->setInt(1, studentId);
            res = pstmt->executeQuery();

            map<int, pair<int, string>> classes;
            counter = 1;
            cout << "Wybierz klasę do przypisania ucznia " << studentFullName << ":" << endl;
            while (res->next()) {
                int classId = res->getInt("id");
                string className = res->getString("nazwa");
                classes[counter] = make_pair(classId, className);
                cout << counter << ". " << className << endl;
                counter++;
            }
            delete pstmt;
            delete res;

            // Wybierz klasę do przypisania ucznia
            int classChoice;
            cout << "Podaj numer klasy: ";
            cin >> classChoice;

            if (classes.find(classChoice) == classes.end()) {
                cout << "Nieprawidłowy wybór klasy." << endl;
                return;
            }

            int classId = classes[classChoice].first;

            // Wstaw ucznia do wybranej klasy
            pstmt = con->prepareStatement("INSERT INTO Klasy_Uczniowie (id_klasy, id_ucznia) VALUES (?, ?)");
            pstmt->setInt(1, classId);
            pstmt->setInt(2, studentId);
            pstmt->executeUpdate();

            cout << "Uczeń " << studentFullName << " został przypisany do klasy." << endl;

            delete pstmt;
        }
        catch (sql::SQLException& e) {
            cout << "Błąd SQL: " << e.what() << endl;
        }
    }

    void adminMenu() {
        int choice;
        do {
            clearConsole();

            cout << "\n--- Menu Administratora ---\n";
            cout << "1. Dodaj nauczyciela\n";
            cout << "2. Edytuj nauczyciela\n";
            cout << "3. Usuń nauczyciela\n";
            cout << "4. Dodaj ucznia\n";
            cout << "5. Edytuj ucznia\n";
            cout << "6. Usuń ucznia\n";
            cout << "7. Utwórz klasę\n";
            cout << "8. Edytuj klasę\n";
            cout << "9. Usuń klasę\n";
            cout << "10. Utwórz przedmiot\n";
            cout << "11. Edytuj przedmiot\n";
            cout << "12. Usuń przedmiot\n";
            cout << "13. Przypisz ucznia do klasy\n";
            cout << "14. Przypisz nauczyciela do klasy\n";
            cout << "15. Przypisz nauczyciela do przedmiotu\n";
            cout << "16. Wyloguj\n";
            cout << "Twój wybór: ";
            cin >> choice;

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
                assignStudentToClass();
                break;
            case 14:
                assignTeacherToClass();
                break;
            case 15:
                assignTeacherToSubject();
                break;
            case 16:
                return;
            default:
                cout << "Nieprawidłowy wybór. Spróbuj ponownie." << endl;
            }
        } while (true);
    }
};

int main() {
    setlocale(LC_CTYPE, "polish");

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
