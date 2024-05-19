#include <iostream>
#include <locale.h>
#include <mysql_connection.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>

using namespace std;

const string server = "tcp://127.0.0.1:3306";
const string username = "root";
const string password = "";
const string db = "schoolcrm";

void loginAsTeacher(sql::Connection* con) {
    string teacherUsername;
    string teacherPassword;
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
        }
        else {
            cout << "Logowanie nie powiodło się. Niepoprawna nazwa użytkownika lub hasło." << endl;
        }
        delete pstmt;
        delete res;
    }
    catch (sql::SQLException& e) {
        cout << "Błąd SQL: " << e.what() << endl;
    }
}

void addGrade(sql::Connection* con) {
    // Implementacja dodawania oceny
}

void addBehavioralNote(sql::Connection* con) {
    // Implementacja dodawania uwagi z zachowania
}

void loginAsAdmin(sql::Connection* con) {
    string adminUsername;
    string adminPassword;
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
        }
        else {
            cout << "Logowanie nie powiodło się. Niepoprawna nazwa użytkownika lub hasło." << endl;
        }
        delete pstmt;
        delete res;
    }
    catch (sql::SQLException& e) {
        cout << "Błąd SQL: " << e.what() << endl;
    }
}

void addStudent(sql::Connection* con) {
    string studentName;
    string studentSurname;
    string studentBirthdate;
    string studentPesel;
    string studentAddress;
    string studentPhone;
    string studentEmail;
    string studentUsername;
    string studentPassword;

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

    cout << "Podaj numer telefonu ucznia : ";
    cin >> studentPhone;
    cout << "Podaj adres e-mail ucznia: ";
    cin >> studentEmail;
    cout << "Wpisz nazwę użytkownika ucznia: ";
    cin >> studentUsername;
    cout << "Podaj hasło ucznia: ";
    cin >> studentPassword;

    try {
        // Dodawanie nowego ucznia do tabeli "Osoby"
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

        delete pstmt;

        // Pobranie identyfikatora ostatnio wstawionego ucznia
        pstmt = con->prepareStatement("SELECT LAST_INSERT_ID()");
        sql::ResultSet* generatedKeys = pstmt->executeQuery();
        int studentId;
        if (generatedKeys->next()) {
            studentId = generatedKeys->getInt(1);
        }
        else {
            throw sql::SQLException("Nie zwrócono wygenerowanych kluczy");
        }
        delete pstmt;
        delete generatedKeys;

        // Dodawanie nowego ucznia do tabeli "Uczniowie" z powiązaniem z odpowiednią osobą
        pstmt = con->prepareStatement("INSERT INTO Uczniowie (id, username, password) VALUES (?, ?, ?)");
        pstmt->setInt(1, studentId);
        pstmt->setString(2, studentUsername);
        pstmt->setString(3, studentPassword);
        pstmt->executeUpdate();

        cout << "Student dodał pomyślnie." << endl;

        delete pstmt;
    }
    catch (sql::SQLException& e) {
        cout << "Błąd SQL: " << e.what() << endl;
    }
}

void createClass(sql::Connection* con) {
    // Implementacja tworzenia klasy
    string className;

    cout << "Wpisz nazwę klasy: ";
    cin >> className;

    try {
        // Dodawanie nowej klasy do tabeli "klasy"
        sql::PreparedStatement* pstmt;
        pstmt = con->prepareStatement("INSERT INTO klasy (nazwa) VALUES (?);");
        pstmt->setString(1, className);
        pstmt->executeUpdate();

        delete pstmt;

        cout << "Klasa została dodana pomyślnie." << endl;
    }
    catch (sql::SQLException& e) {
        cout << "Błąd SQL: " << e.what() << endl;
    }
}

void assignStudentToClass(sql::Connection* con) {
    string studentImie;
    string studentNazwisko;
    string nazwaKlasy;

    // Poproś użytkownika o podanie imienia, nazwiska ucznia i nazwy klasy
    cout << "Podaj imię ucznia: ";
    cin >> studentImie;
    cout << "Podaj nazwisko ucznia: ";
    cin >> studentNazwisko;
    cout << "Podaj nazwę klasy: ";
    cin >> nazwaKlasy;

    try {
        // Znajdź identyfikator klasy na podstawie nazwy
        sql::PreparedStatement* pstmt;
        pstmt = con->prepareStatement("SELECT id FROM Klasy WHERE nazwa=?");
        pstmt->setString(1, nazwaKlasy);
        sql::ResultSet* res = pstmt->executeQuery();

        int idKlasy;
        if (res->next()) {
            idKlasy = res->getInt("id");

            // Znajdź identyfikator ucznia na podstawie imienia i nazwiska
            pstmt = con->prepareStatement("SELECT id FROM Osoby WHERE imie=? AND nazwisko=?");
            pstmt->setString(1, studentImie);
            pstmt->setString(2, studentNazwisko);
            res = pstmt->executeQuery();

            int idUcznia;
            if (res->next()) {
                idUcznia = res->getInt("id");

                // Wstawienie danych do tabeli Klasy_Uczniowie
                pstmt = con->prepareStatement("INSERT INTO Klasy_Uczniowie (id_klasy, id_ucznia) VALUES (?, ?)");
                pstmt->setInt(1, idKlasy);
                pstmt->setInt(2, idUcznia);
                pstmt->executeUpdate();

                cout << "Uczeń został przypisany do klasy pomyślnie." << endl;
            }
            else {
                cout << "Uczeń o imieniu " << studentImie << " i nazwisku " << studentNazwisko << " nie został znaleziony." << endl;
            }
        }
        else {
            cout << "Klasa o nazwie " << nazwaKlasy << " nie została znaleziona." << endl;
        }

        delete pstmt;
        delete res;
    }
    catch (sql::SQLException& e) {
        cout << "Błąd SQL: " << e.what() << endl;
    }
}


void removeStudentFromClass(sql::Connection* con) {
    // Implementacja usuwania ucznia z klasy
}

int main() {
    setlocale(LC_CTYPE, "polish");

    sql::Driver* driver;
    sql::Connection* con;

    try {
        driver = get_driver_instance();
        con = driver->connect(server, username, password);
        con->setSchema(db);

        int userType;
        do {
            cout << "Wybierz typ użytkownika:" << endl;
            cout << "1. Nauczyciel" << endl;
            cout << "2. Administrator" << endl;
            cout << "3. Wyjdź" << endl;
            cout << "Twój wybór: ";
            cin >> userType;

            switch (userType) {
            case 1:
                loginAsTeacher(con);
                addGrade(con);
                addBehavioralNote(con);
                break;
            case 2:
                loginAsAdmin(con);

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
                        addStudent(con);
                        break;
                    case 2:
                        createClass(con);
                        break;
                    case 3:
                        assignStudentToClass(con);
                        break;
                    case 4:
                        break; // Wylogowanie zagnieżdżone wewnętrznej pętli do...while
                    default:
                        cout << "Nieprawidłowy wybór." << endl;
                    }
                } while (adminOption != 4);
                break;
            case 3:
                // Zakończ program
                return 0;
            default:
                cout << "Nieprawidłowy wybór." << endl;
            }
        } while (true); // Pętla główna programu działa, dopóki użytkownik nie wybierze opcji "Wyloguj się"

        delete con;
    }
    catch (sql::SQLException& e) {
        cout << "Nie można połączyć się z serwerem. Komunikat błędu: " << e.what() << endl;
    }

    return 0;
}
