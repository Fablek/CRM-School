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
private:
    int teacherId;  // Przechowujemy id nauczyciela po zalogowaniu

public:
    Teacher(sql::Connection* connection) : User(connection), teacherId(-1) {}

    bool login() override {
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
        try {
            // Pobierz wszystkie dostępne przedmioty z bazy danych, do których jest przypisany nauczyciel
            sql::PreparedStatement* pstmt;
            pstmt = con->prepareStatement("SELECT p.id, p.nazwa FROM Przedmioty p JOIN Nauczyciele_Przedmioty np ON p.id = np.id_przedmiotu WHERE np.id_nauczyciela = ?");
            pstmt->setInt(1, teacherId);
            sql::ResultSet* res = pstmt->executeQuery();

            // Tworzymy mapę przechowującą identyfikatory przedmiotów i ich nazwy
            map<int, string> subjects;
            while (res->next()) {
                int subjectId = res->getInt("id");
                string subjectName = res->getString("nazwa");
                subjects[subjectId] = subjectName;
            }

            // Jeśli nauczyciel nie jest przypisany do żadnego przedmiotu, wyświetlamy komunikat i przerywamy działanie funkcji
            if (subjects.empty()) {
                cout << "Nie jesteś przypisany do żadnego przedmiotu." << endl;
                delete pstmt;
                delete res;
                return;
            }

            // Wyświetlamy menu wyboru przedmiotu dla użytkownika
            cout << "Wybierz przedmiot:" << endl;
            for (const auto& pair : subjects) {
                cout << pair.first << ". " << pair.second << endl;
            }

            // Pobieramy wybór użytkownika
            int choice;
            cout << "Twój wybór: ";
            cin >> choice;

            // Sprawdzamy, czy wybór jest prawidłowy
            if (subjects.find(choice) != subjects.end()) {
                string subjectName = subjects[choice];
                // Pozostała część funkcji addGrade()

                string studentUsername, gradeDate, gradeDescription;
                double grade;

                cout << "Wpisz nazwę użytkownika ucznia: ";
                cin >> studentUsername;
                cout << "Wpisz ocenę: ";
                cin >> grade;
                cout << "Wpisz datę oceny (RRRR-MM-DD): ";
                cin >> gradeDate;
                cout << "Wpisz opis oceny: ";
                cin.ignore(); // Ignorujemy znak nowej linii z poprzedniego wczytania
                getline(cin, gradeDescription);

                // Pobierz identyfikator ucznia z bazy danych na podstawie jego nazwy użytkownika
                int studentId;
                pstmt = con->prepareStatement("SELECT id FROM Uczniowie WHERE username = ?");
                pstmt->setString(1, studentUsername);
                res = pstmt->executeQuery();
                if (res->next()) {
                    studentId = res->getInt("id");
                }
                else {
                    cout << "Uczeń o podanym użytkowniku nie istnieje." << endl;
                    delete pstmt;
                    delete res;
                    return;
                }

                // Dodaj ocenę do bazy danych
                pstmt = con->prepareStatement("INSERT INTO Oceny (id_ucznia, id_przedmiotu, ocena, data_oceny, opis) VALUES (?, ?, ?, ?, ?)");
                pstmt->setInt(1, studentId);
                pstmt->setInt(2, choice);
                pstmt->setDouble(3, grade);
                pstmt->setString(4, gradeDate);
                pstmt->setString(5, gradeDescription);
                pstmt->executeUpdate();

                cout << "Ocena została dodana pomyślnie." << endl;
            }
            else {
                cout << "Nieprawidłowy wybór przedmiotu." << endl;
            }

            delete pstmt;
            delete res;
        }
        catch (sql::SQLException& e) {
            cout << "Błąd SQL: " << e.what() << endl;
        }
    }

    void addComment() {
        try {
            string studentUsername, commentContent;

            cout << "Wpisz nazwę użytkownika ucznia: ";
            cin >> studentUsername;
            cout << "Wpisz treść uwagi: ";
            cin.ignore();
            getline(cin, commentContent);

            // Pobierz identyfikator ucznia z bazy danych na podstawie jego nazwy użytkownika
            int studentId;
            sql::PreparedStatement* pstmt = con->prepareStatement("SELECT id FROM Uczniowie WHERE username = ?");
            pstmt->setString(1, studentUsername);
            sql::ResultSet* res = pstmt->executeQuery();
            if (res->next()) {
                studentId = res->getInt("id");
            }
            else {
                cout << "Uczeń o podanym użytkowniku nie istnieje." << endl;
                delete pstmt;
                delete res;
                return;
            }

            // Dodaj uwagę do bazy danych
            pstmt = con->prepareStatement("INSERT INTO Uwagi (id_ucznia, tresc) VALUES (?, ?)");
            pstmt->setInt(1, studentId);
            pstmt->setString(2, commentContent);
            pstmt->executeUpdate();

            cout << "Uwaga została dodana pomyślnie." << endl;

            delete pstmt;
            delete res;
        }
        catch (sql::SQLException& e) {
            cout << "Błąd SQL: " << e.what() << endl;
        }
    }

    void viewClassStudents() {
        try {
            string className;
            cout << "Wpisz nazwę klasy: ";
            cin >> className;

            // Pobierz identyfikator klasy na podstawie nazwy klasy
            sql::PreparedStatement* pstmt;
            pstmt = con->prepareStatement("SELECT id FROM Klasy WHERE nazwa = ?");
            pstmt->setString(1, className);
            sql::ResultSet* res = pstmt->executeQuery();

            int classId;
            if (res->next()) {
                classId = res->getInt("id");
            }
            else {
                cout << "Klasa o podanej nazwie nie istnieje." << endl;
                delete pstmt;
                delete res;
                return;
            }
            delete res;

            // Pobierz listę uczniów przypisanych do tej klasy
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
                int studentId = students[choice].first;

                // Wyświetl szczegóły ucznia
                cout << "Informacje o uczniu:" << endl;
                pstmt = con->prepareStatement("SELECT * FROM Osoby WHERE id = ?");
                pstmt->setInt(1, studentId);
                res = pstmt->executeQuery();

                if (res->next()) {
                    cout << "Imię: " << res->getString("imie") << endl;
                    cout << "Nazwisko: " << res->getString("nazwisko") << endl;
                    cout << "Data urodzenia: " << res->getString("data_urodzenia") << endl;
                    cout << "PESEL: " << res->getString("pesel") << endl;
                    cout << "Adres: " << res->getString("adres") << endl;
                    cout << "Telefon: " << res->getString("telefon") << endl;
                    cout << "Email: " << res->getString("email") << endl;
                }
                delete res;

                // Wyświetl oceny ucznia
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

                // Wyświetl uwagi ucznia
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

            delete pstmt;
        }
        catch (sql::SQLException& e) {
            cout << "Błąd SQL: " << e.what() << endl;
        }
    }

    void teacherMenu() {
        int choice;
        do {
            cout << "\n--- Menu Nauczyciela ---\n";
            cout << "1. Dodaj ocenę\n";
            cout << "2. Dodaj uwagę\n";
            cout << "3. Wyświetl uczniów klasy\n";
            cout << "4. Wyloguj\n";
            cout << "Twój wybór: ";
            cin >> choice;

            switch (choice) {
            case 1:
                addGrade();
                break;
            case 2:
                addComment();
                break;
            case 3:
                viewClassStudents();
                break;
            case 4:
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

    void addTeacher() {
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

    void addStudent() {
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

            int studentId = students[studentChoice].first;
            string studentFullName = students[studentChoice].second;

            // Pobierz listę klas, do których ucznik jeszcze nie jest przypisany
            pstmt = con->prepareStatement("SELECT id, nazwa FROM Klasy WHERE id NOT IN (SELECT id FROM Klasy_Uczniowie WHERE id_ucznia=?)");
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
            cout << "\n--- Menu Administratora ---\n";
            cout << "1. Dodaj nauczyciela\n";
            cout << "2. Dodaj ucznia\n";
            cout << "3. Utwórz klasę\n";
            cout << "4. Przypisz ucznia do klasy\n";
            cout << "5. Wyloguj\n";
            cout << "Twój wybór: ";
            cin >> choice;

            switch (choice) {
            case 1:
                addTeacher();
                break;
            case 2:
                addStudent();
                break;
            case 3:
                createClass();
                break;
            case 4:
                assignStudentToClass();
                break;
            case 5:
                return;
            default:
                cout << "Nieprawidłowy wybór. Spróbuj ponownie." << endl;
            }
        } while (true);
    }
};

int main() {
    setlocale(LC_CTYPE, "polish");

    DatabaseManager dbManager;
    sql::Connection* con = dbManager.getConnection();

    if (con) {
        int userType;
        do {
            cout << "\n--- System Zarządzania Szkołą ---\n";
            cout << "1. Zaloguj jako nauczyciel\n";
            cout << "2. Zaloguj jako administrator\n";
            cout << "3. Wyjdź\n";
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
                cout << "Zamykanie programu..." << endl;
                break;
            default:
                cout << "Nieprawidłowy wybór. Spróbuj ponownie." << endl;
            }
        } while (userType != 3);
    }
    else {
        cout << "Nie udało się połączyć z bazą danych." << endl;
    }

    return 0;
}
