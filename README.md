# STACK
<ul>
  <li>xampp: https://www.apachefriends.org/pl/index.html</li>
  <li>MySQL Connector/C++: https://dev.mysql.com/downloads/connector/cpp/</li>
  <li>How to:</li>
  <ul>
    <li>MySQL Connector VisualStudio setup: https://www.youtube.com/watch?v=a_W4zt5sR1M</li>
    <li>Visual Studio project settings (when you download it, you must configure it!)</li>
    <ol>
        <li>Release! <img src="https://screen.proudmedia.eu/v4rand_12e1asd/devenv_2024-05-17_23-09-35.png"></li>
        <li>Include files:<img src="https://screen.proudmedia.eu/v4rand_12e1asd/setting1.png"></li>
        <li>Preprocesor:<img src="https://screen.proudmedia.eu/v4rand_12e1asd/setting2.png"></li>
        <li>lib, code generator:<img src="https://screen.proudmedia.eu/v4rand_12e1asd/setting3.png"></li>
        <li>Linker :<img src="https://screen.proudmedia.eu/v4rand_12e1asd/settings4.png"></li>
    </ol>
  </ul>
</ul>

# DATABASE - sql code
CREATE DATABASE IF NOT EXISTS schoolcrm;

USE schoolcrm;

CREATE TABLE IF NOT EXISTS Osoby (
    id INT AUTO_INCREMENT PRIMARY KEY,
    imie VARCHAR(50) NOT NULL,
    nazwisko VARCHAR(50) NOT NULL,
    data_urodzenia DATE NOT NULL,
    pesel VARCHAR(11) UNIQUE NOT NULL,
    adres VARCHAR(100) NOT NULL,
    telefon VARCHAR(15),
    email VARCHAR(50)
);

CREATE TABLE IF NOT EXISTS Nauczyciele (
    id INT PRIMARY KEY,
    FOREIGN KEY (id) REFERENCES Osoby(id) ON DELETE CASCADE,
    username VARCHAR(50) UNIQUE NOT NULL,
    password VARCHAR(50) NOT NULL
);

CREATE TABLE IF NOT EXISTS Uczniowie (
    id INT PRIMARY KEY,
    FOREIGN KEY (id) REFERENCES Osoby(id) ON DELETE CASCADE,
    username VARCHAR(50) UNIQUE NOT NULL,
    password VARCHAR(50) NOT NULL
);

CREATE TABLE IF NOT EXISTS Admin (
    id INT PRIMARY KEY,
    FOREIGN KEY (id) REFERENCES Osoby(id) ON DELETE CASCADE,
    username VARCHAR(50) UNIQUE NOT NULL,
    password VARCHAR(50) NOT NULL
);

CREATE TABLE IF NOT EXISTS Klasy (
    id INT AUTO_INCREMENT PRIMARY KEY,
    nazwa VARCHAR(50) UNIQUE NOT NULL
);

CREATE TABLE IF NOT EXISTS Przedmioty (
    id INT AUTO_INCREMENT PRIMARY KEY,
    nazwa VARCHAR(50) UNIQUE NOT NULL
);

CREATE TABLE IF NOT EXISTS Oceny (
    id INT AUTO_INCREMENT PRIMARY KEY,
    id_ucznia INT,
    id_przedmiotu INT,
    ocena DECIMAL(3, 2),
    data_oceny DATE NOT NULL,
    opis VARCHAR(80),
    FOREIGN KEY (id_ucznia) REFERENCES Uczniowie(id) ON DELETE CASCADE,
    FOREIGN KEY (id_przedmiotu) REFERENCES Przedmioty(id) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS Uwagi (
    id INT AUTO_INCREMENT PRIMARY KEY,
    id_ucznia INT,
    tresc VARCHAR(255) NOT NULL,
    FOREIGN KEY (id_ucznia) REFERENCES Uczniowie(id) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS Klasy_Uczniowie (
    id_klasy INT,
    id_ucznia INT,
    PRIMARY KEY (id_klasy, id_ucznia),
    FOREIGN KEY (id_klasy) REFERENCES Klasy(id) ON DELETE CASCADE,
    FOREIGN KEY (id_ucznia) REFERENCES Uczniowie(id) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS Klasy_Przedmioty (
    id_klasy INT,
    id_przedmiotu INT,
    PRIMARY KEY (id_klasy, id_przedmiotu),
    FOREIGN KEY (id_klasy) REFERENCES Klasy(id) ON DELETE CASCADE,
    FOREIGN KEY (id_przedmiotu) REFERENCES Przedmioty(id) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS Uprawnienia (
    id_nauczyciela INT,
    id_przedmiotu INT,
    PRIMARY KEY (id_nauczyciela, id_przedmiotu),
    FOREIGN KEY (id_nauczyciela) REFERENCES Nauczyciele(id) ON DELETE CASCADE,
    FOREIGN KEY (id_przedmiotu) REFERENCES Przedmioty(id) ON DELETE CASCADE
);
