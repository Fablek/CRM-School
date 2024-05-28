#include <iostream>
#include <locale.h>
#include <mysql_connection.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <string>

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
    DatabaseManager() {
        driver = get_driver_instance();
        con = driver->connect(server, username, password);
        con->setSchema(db);
    }

    ~DatabaseManager() {
        delete con;
    }

    sql::Connection* getConnection() {
        return con;
    }
};

class User {
protected:
    sql::Connection* con;

public:
    User(sql::Connection* connection) : con(connection) {}

    virtual bool login() = 0;
};

class Teacher : public User {
public:
    Teacher(sql::Connection* connection) : User(connection) {}

    bool login() override {
        string teacherUsername, teacherPassword;
        cout << "Wpisz swoją nazwę użytkownika: ";
        cin >> teacherUsername;
        cout << "Podaj hasło: ";
        cin >> teacherPassword;

        try {
            sql::PreparedStatement* pstmt;
            pstmt = con->prepareStatement("SELECT * FROM Nauczyciele WHERE username=? AND password=?");
            pstmt->setString(1, teacherUsername);
            pstmt->setString(2, teacherPassword);
            sql::ResultSet* res = pstmt->executeQuery();

            if (res->next()) {
                cout << "Pomyślnie zalogowano jako nauczyciel." << endl;
                delete pstmt;
                delete res;
                return true;
            }
            else {
                cout << "Logowanie nie powiodło się. Niepoprawna nazwa użytkownika lub hasło." << endl;
                delete pstmt;
                delete res;
                return false;
            }
        }
        catch (sql::SQLException& e) {
            cout << "Błąd SQL: " << e.what() << endl;
            return false;
        }
    }

    void addGrade() {
        string studentUsername, subjectName, gradeDate, gradeDescription;
        double grade;

        cout << "Wpisz nazwę użytkownika ucznia: ";
        cin >> studentUsername;
        cout << "Wpisz nazwę przedmiotu: ";
        cin >> subjectName;
        cout << "Wpisz ocenę: ";
        cin >> grade;
        cout << "Wpisz datę oceny (RRRR-MM-DD): ";
        cin >> gradeDate;
        cout << "Wpisz opis oceny: ";
        cin.ignore(); // Clear input buffer
        getline(cin, gradeDescription);

        try {
            sql::PreparedStatement* pstmt;
            sql::ResultSet* res;

            // Get student ID
            pstmt = con->prepareStatement("SELECT id FROM Uczniowie WHERE username=?");
            pstmt->setString(1, studentUsername);
            res = pstmt->executeQuery();

            int studentId;
            if (res->next()) {
                studentId = res->getInt("id");
            }
            else {
                cout << "Uczeń nie został znaleziony." << endl;
                delete pstmt;
                delete res;
                return;
            }

            delete res;

            // Get subject ID
            pstmt = con->prepareStatement("SELECT id FROM Przedmioty WHERE nazwa=?");
            pstmt->setString(1, subjectName);
            res = pstmt->executeQuery();

            int subjectId;
            if (res->next()) {
                subjectId = res->getInt("id");
            }
            else {
                cout << "Przedmiot nie został znaleziony." << endl;
                delete pstmt;
                delete res;
                return;
            }

            delete res;
            delete pstmt;

            // Insert grade
            pstmt = con->prepareStatement("INSERT INTO Oceny (id_ucznia, id_przedmiotu, ocena, data_oceny, opis) VALUES (?, ?, ?, ?, ?)");
            pstmt->setInt(1, studentId);
            pstmt->setInt(2, subjectId);
            pstmt->setDouble(3, grade);
            pstmt->setString(4, gradeDate);
            pstmt->setString(5, gradeDescription);
            pstmt->executeUpdate();

            cout << "Ocena została dodana pomyślnie." << endl;

            delete pstmt;
        }
        catch (sql::SQLException& e) {
            cout << "Błąd SQL: " << e.what() << endl;
        }
    }

    void addBehavioralNote() {
        // Implementacja dodawania uwagi z zachowania
        cout << "Dodawanie uwagi z zachowania (implementacja w toku)." << endl;
    }

    void teacherMenu() {
        int teacherOption;
        do {
            cout << "Wybierz jedną z dostępnych opcji:" << endl;
            cout << "1. Dodaj ocenę" << endl;
            cout << "2. Dodaj uwagę z zachowania" << endl;
            cout << "3. Wyloguj się" << endl;
            cout << "Twój wybór: ";
            cin >> teacherOption;

            switch (teacherOption) {
            case 1:
                addGrade();
                break;
            case 2:
                addBehavioralNote();
                break;
            case 3:
                return;
            default:
                cout << "Nieprawidłowy wybór." << endl;
            }
        } while (true);
    }
};

class Admin : public User {
public:
    Admin(sql::Connection* connection) : User(connection) {}

    bool login() override {
        string adminUsername, adminPassword;
        cout << "Wpisz swoją nazwę użytkownika: ";
        cin >> adminUsername;
        cout << "Podaj hasło: ";
        cin >> adminPassword;

        try {
            sql::PreparedStatement* pstmt;
            pstmt = con->prepareStatement("SELECT * FROM Admin WHERE username=? AND password=?");
            pstmt->setString(1, adminUsername);
            pstmt->setString(2, adminPassword);
            sql::ResultSet* res = pstmt->executeQuery();

            if (res->next()) {
                cout << "Pomyślnie zalogowano jako administrator." << endl;
                delete pstmt;
                delete res;
                return true;
            }
            else {
                cout << "Logowanie nie powiodło się. Niepoprawna nazwa użytkownika lub hasło." << endl;
                delete pstmt;
                delete res;
                return false;
            }
        }
        catch (sql::SQLException& e) {
            cout << "Błąd SQL: " << e.what() << endl;
            return false;
        }
    }

    void adminMenu() {
        int adminOption;
        do {
            cout << "Wybierz jedną z dostępnych opcji:" << endl;
            cout << "1. Dodaj ucznia" << endl;
            cout << "2. Stwórz klasę" << endl;
            cout << "3. Dodaj ucznia do klasy" << endl;
            cout << "4. Wyloguj się" << endl;
            cout << "Twój wybór: ";
            cin >> adminOption;

            switch (adminOption) {
            case 1:
                addStudent();
                break;
            case 2:
                createClass();
                break;
            case 3:
                assignStudentToClass();
                break;
            case 4:
                return;
            default:
                cout << "Nieprawidłowy wybór." << endl;
            }
        } while (true);
    }

    void addStudent() {
        string studentName, studentSurname, studentBirthdate, studentPesel, studentAddress, studentPhone, studentEmail, studentUsername, studentPassword;

        cout << "Wpisz imię ucznia: ";
        cin >> studentName;
        cout << "Wpisz nazwisko ucznia: ";
        cin >> studentSurname;
        cout << "Wpisz datę urodzenia ucznia (RRRR-MM-DD): ";
        cin >> studentBirthdate;
        cout << "Wpisz PESEL studenta: ";
        cin >> studentPesel;
        cout << "Wpisz adres studenta: ";
        cin.ignore(); // Clearing the input buffer
        getline(cin, studentAddress);
        cout << "Podaj numer telefonu ucznia: ";
        cin >> studentPhone;
        cout << "Podaj adres e-mail ucznia: ";
        cin >> studentEmail;
        cout << "Wpisz nazwę użytkownika ucznia: ";
        cin >> studentUsername;
        cout << "Podaj hasło ucznia: ";
        cin >> studentPassword;

        try {
            sql::PreparedStatement* pstmt;
            pstmt = con->prepareStatement("INSERT INTO Osoby (imie, nazwisko, data_urodzenia, pesel, adres, telefon, email) VALUES (?, ?, ?, ?, ?, ?, ?)");
            pstmt->setString(1, studentName);
            pstmt->setString(2, studentSurname);
            pstmt->setString(3, studentBirthdate);
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

    void createClass() {
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

    void assignStudentToClass() {
        string className, studentName, studentSurname;
        cout << "Wpisz nazwę klasy: ";
        cin >> className;
        cout << "Wpisz imię ucznia: ";
        cin >> studentName;
        cout << "Wpisz nazwisko ucznia: ";
        cin >> studentSurname;

        try {
            sql::PreparedStatement* pstmt;
            pstmt = con->prepareStatement("SELECT id FROM Klasy WHERE nazwa=?");
            pstmt->setString(1, className);
            sql::ResultSet* res = pstmt->executeQuery();

            int classId;
            if (res->next()) {
                classId = res->getInt("id");

                pstmt = con->prepareStatement("SELECT id FROM Osoby WHERE imie=? AND nazwisko=?");
                pstmt->setString(1, studentName);
                pstmt->setString(2, studentSurname);
                res = pstmt->executeQuery();

                int studentId;
                if (res->next()) {
                    studentId = res->getInt("id");

                    pstmt = con->prepareStatement("SELECT * FROM Klasy_Uczniowie WHERE id_klasy=? AND id_ucznia=?");
                    pstmt->setInt(1, classId);
                    pstmt->setInt(2, studentId);
                    res = pstmt->executeQuery();

                    if (res->next()) {
                        cout << "Uczeń już należy do tej klasy." << endl;
                    }
                    else {
                        pstmt = con->prepareStatement("INSERT INTO Klasy_Uczniowie (id_klasy, id_ucznia) VALUES (?, ?)");
                        pstmt->setInt(1, classId);
                        pstmt->setInt(2, studentId);
                        pstmt->executeUpdate();

                        cout << "Uczeń został przypisany do klasy pomyślnie." << endl;
                    }
                }
                else {
                    cout << "Uczeń o imieniu " << studentName << " i nazwisku " << studentSurname << " nie został znaleziony." << endl;
                }
            }
            else {
                cout << "Klasa o nazwie " << className << " nie została znaleziona." << endl;
            }

            delete pstmt;
            delete res;
        }
        catch (sql::SQLException& e) {
            cout << "Błąd SQL: " << e.what() << endl;
        }
    }
};

int main() {
    setlocale(LC_CTYPE, "polish");

    try {
        DatabaseManager dbManager;
        sql::Connection* con = dbManager.getConnection();

        int userType;
        do {
            cout << "Wybierz typ użytkownika:" << endl;
            cout << "1. Nauczyciel" << endl;
            cout << "2. Administrator" << endl;
            cout << "3. Wyjdź" << endl;
            cout << "Twój wybór: ";
            cin >> userType;

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
                return 0;
            default:
                cout << "Nieprawidłowy wybór." << endl;
            }
        } while (true);

    }
    catch (sql::SQLException& e) {
        cout << "Nie można połączyć się z serwerem. Komunikat błędu: " << e.what() << endl;
    }

    return 0;
}
