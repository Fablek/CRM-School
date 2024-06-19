#ifndef TEACHER_H
#define TEACHER_H

#include "User.h"
#include <map>
#include <tuple>

class Teacher : public User {
private:
    int teacherId;

public:
    Teacher(sql::Connection* connection);
    bool login() override;
    void teacherMenu();
    void addGrade();
    void editGrade();
    void deleteGrade();
    void addComment();
    void editComment();
    void deleteComment();
    void viewClassStudents();
};

#endif
