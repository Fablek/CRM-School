#ifndef ADMIN_H
#define ADMIN_H

#include "User.h"

class Admin : public User {
public:
    Admin(sql::Connection* connection);
    bool login() override;
    void adminMenu();
    void addTeacher();
    void editTeacher();
    void removeTeacher();
    void addStudent();
    void editStudent();
    void removeStudent();
    void createClass();
    void editClass();
    void removeClass();
    void addSubject();
    void editSubject();
    void removeSubject();
    void assignTeacherToClass();
    void assignTeacherToSubject();
    void assignStudentToClass();
};

#endif 