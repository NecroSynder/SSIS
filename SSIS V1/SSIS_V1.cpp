#include <fstream>
#include <iostream>
#include <string.h>
#include <vector>

using namespace std;

string trimfnc(string str) {
  const char *typeOfWhitespaces = " \t\n\r\f\v";
  str.erase(str.find_last_not_of(typeOfWhitespaces) + 1);
  str.erase(0, str.find_first_not_of(typeOfWhitespaces));
  return str;
}

bool duplicate(string elementCheck,
               int fileCheck) // checks inputs for duplicates in files
{
  ifstream duplicateCheck;
  string line;
  if (fileCheck == 1) // sets duplicate checker to the student info file
  {
    duplicateCheck.open("students.txt", ios::in);
    elementCheck = " | " + elementCheck + " | ";
  } else if (fileCheck == 2) // sets duplicate checker to the course file
  {
    duplicateCheck.open("courses.txt", ios::in);
    elementCheck = elementCheck + " ";
  } else
    return false;

  while (getline(duplicateCheck,
                 line)) // checks for duplicates in student IDs and courses
  {
    if (line.find(elementCheck) != string::npos) {
      cout << "Already in the system, going back to menu...." << endl;
      duplicateCheck.close();
      return true;
    }
  }

  return false;
}

void AddCourse() {
  ofstream courseSet;
  string courseInput;
  string line;
  cout << "Add Course:";
  cin.clear();
  cin.ignore();
  getline(cin, courseInput);
  courseInput = trimfnc(courseInput);

  if (courseInput.find_first_not_of(' ') ==
      std::string::npos) // makes sure blank inputs does not get inputted into
                         // the file
  {
    cout << "You cannot enter blank characters, returning to menu..." << endl;
    return;
  }

  if (duplicate(courseInput, 2) == true)
    return;

  courseSet.open("courses.txt",
                 ios::app); // input written into course file
  if (courseSet.is_open()) {
    courseSet << courseInput << " "
              << endl; // space is added to serve as detector for find function
                       // to find exact word.
    courseSet.close();
  }
  cout << "Course Added!" << endl;
}

string courseSelect() {
  int count = 0;
  ifstream course;
  string courseSelected = "none";
  string line;
  course.open("courses.txt", ios::in);
  if (course.is_open() == true) {
    while (getline(course, line)) // makes list with corresponding number for
                                  // user to select course with
    {
      cout << count + 1 << " " << line << endl;
      count++;
    }

    vector<string> courseList;
    count = 0;

    course.clear();
    course.seekg(0);

    while (getline(
        course, line)) // put courses stored in course file to vector for easy
                       // access when writing available courses to student file
    {
      courseList.push_back(line);
    }

    if (courseList.empty() == true) {
      cout << "No courses added yet, please add a course." << endl;
      return courseSelected;
    }

    int length = courseList.size();

    do // allows user to select course to add to student file from a list of
       // courses in course file
    {
      cout << "Select Course You Would Like To Add (input number from the left "
              "of the chosen course):\n";
      cin >> count;

      if (count < 1 || count > length) {
        cout << "Number does not correspond to listed courses. Try again: ";
        cin.clear();
        cin.ignore(10000, '\n');
      }

    } while (count < 1 || count > length);

    courseSelected = courseList[count - 1];
    cout << "Course Selected: " << courseSelected;
    course.close();
  } else {
    cout << "No courses added yet, please add a course." << endl;
  }

  cout << endl;
  return courseSelected;
}

void AddStudent() {
  ofstream studentSet;
  string getName;
  string getId;
  string getCourse;
  string line;

  cout << "Enter Name: ";
  cin.clear();
  cin.ignore();
  getline(cin, getName);
  getName = trimfnc(getName);
  cout << "Enter your ID: ";
  getline(cin, getId);
  getId = trimfnc(getId);

  if (getName.find_first_not_of(' ') ==
      std::string::npos) // makes sure no blank characters are inputted
  {
    cout << "You cannot enter blank characters, returning to menu...";
    return;
  }

  if (getId.find_first_not_of(' ') == std::string::npos) {
    cout << "You cannot enter blank characters, returning to menu...";
    return;
  }

  getCourse = courseSelect();

  if (getCourse ==
      "none") // cannot add student if there are no courses available
    return;

  if (duplicate(getId, 1) == true) // checks for duplicate IDs
    return;

  studentSet.open("students.txt", ios::app);
  if (studentSet.is_open() == true) {
    studentSet << getName << " | ";
    studentSet << getId << " | ";
    studentSet << getCourse << " " << endl;
    studentSet.close();
  }

  cout << "Student Added!" << endl;
}

void listFiles(int choice) // lists files
{
  ifstream listName;
  string line;

  if (choice == 1)
    listName.open("students.txt", ios::in);
  else if (choice == 2)
    listName.open("courses.txt", ios::in);
  else
    return;

  if (listName.is_open() == true) {
    while (getline(listName, line)) {
      cout << line << endl;
    }

    listName.close();
  }
}

void deleteStudent() {
  listFiles(1);
  ifstream studentOrig("students.txt");
  ofstream studentNew("temp.txt");
  string line;
  string idToDelete;
  bool found = false;
  cin.clear();
  cin.ignore();
  cout << "Enter ID of student to delete: ";
  getline(cin, idToDelete);
  idToDelete = trimfnc(idToDelete);

  while (getline(studentOrig, line)) {
    if (line.find(" | " + idToDelete + " | ") ==
        string::npos) // transfer line from orig file to new file if string is
                      // not found
    {
      studentNew << line << endl;
    } else {
      found = true;
    }
  }

  if (!found) {
    cout << "Student not found." << endl;
  } else {
    cout << "Student deleted." << endl;
  }

  studentOrig.close();
  studentNew.close();
  remove("students.txt");
  rename("temp.txt", "students.txt");
}

void deleteCourse() {
  listFiles(2);
  ifstream courseOrig("courses.txt");
  ifstream studentOrig("students.txt");
  ofstream courseNew("temp.txt");
  ofstream studentNew("temp2.txt");
  string line;
  string courseToDel;
  bool found = false;

  cout << "Enter course to delete: ";
  cin.clear();
  cin.ignore();
  getline(cin, courseToDel);
  courseToDel = trimfnc(courseToDel);
  if (courseToDel.empty()) {
    cout << "No course chosen" << endl;
    return;
  }

  while (getline(
      courseOrig,
      line)) // checks if course inputted by user is stored in course file
  {
    if (line.find(courseToDel + " ") == string::npos) {
      courseNew << line << endl;
    } else {
      found = true;
    }
  }

  if (!found) {
    cout << "Course not found." << endl;
  } else {

    while (getline(studentOrig, line)) // transfer line where metioned course is
                                       // not found to new student file
    {
      if (line.find(courseToDel + " ") == string::npos) {
        studentNew << line << endl;
      }
    }

    cout << "Course and students associated with the course deleted." << endl;
  }

  courseOrig.close();
  courseNew.close();
  studentNew.close();
  studentOrig.close();
  remove("courses.txt"); // sets new (temp) files to same name as
                         // previous files
  rename("temp.txt", "courses.txt");
  remove("students.txt");
  rename("temp2.txt", "students.txt");
}

void editCourse() {
  listFiles(2);
  ifstream courseOrig("courses.txt");
  ifstream studentOrig("students.txt");
  ofstream courseNew("temp.txt");
  ofstream studentNew("temp2.txt");
  string line;
  string courseToEdit;
  string newCourseName;
  bool found = false;

  cout << "Enter course to edit: ";
  cin.clear();
  cin.ignore();
  getline(cin, courseToEdit);
  courseToEdit = trimfnc(courseToEdit);

  while (getline(courseOrig,
                 line)) // checks orig course file for course to be edited
  {
    if (line == courseToEdit + " ") {
      found = true;
      cout << "Enter new course name: ";
      getline(cin, newCourseName);
      newCourseName = trimfnc(newCourseName);

      if (duplicate(newCourseName, 2) == true) {
        courseOrig.close();
        courseNew.close();
        studentNew.close();
        studentOrig.close();
        remove("temp.txt");
        remove("temp2.txt");
        return;
      }
      courseNew << newCourseName << " " << endl;
    } else {
      courseNew << line << endl;
    }
  }

  if (!found) {
    cout << "Course not found." << endl;
  } else {
    cout << "Course edited." << endl;
  }

  if (found ==
      true) // updates student file to show the edited course name properly
  {
    while (getline(studentOrig, line)) {
      if (line.find(" | " + courseToEdit + " ") == string::npos) {
        studentNew << line << endl;
      } else {
        line.replace(line.find(courseToEdit), line.length(),
                     newCourseName + " ");
        studentNew << line << endl;
      }
    }
  }

  courseOrig.close();
  courseNew.close();
  studentNew.close();
  studentOrig.close();
  remove("courses.txt");
  rename("temp.txt", "courses.txt");
  remove("students.txt");
  rename("temp2.txt", "students.txt");
}

void editStudent() {
  listFiles(1);
  fstream student;
  ifstream studentOrig("students.txt");
  ofstream studentNew("temp.txt");
  string line;
  string idToEdit;
  string newName;
  string newId;
  bool found = false;

  cin.clear();
  cin.ignore();
  cout << "Enter ID of student to edit: ";
  getline(cin, idToEdit);
  idToEdit = trimfnc(idToEdit);

  while (getline(
      studentOrig,
      line)) // checks student ID in file to edit student information if present
  {
    if (line.find(" | " + idToEdit + " | ") == string::npos) {
      studentNew << line << endl;
    } else // edit student information when found
    {
      found = true;
      cout << "Enter new name: ";
      getline(cin, newName);
      newName = trimfnc(newName);
      cout << "Enter new ID: ";
      getline(cin, newId);
      newId = trimfnc(newId);

      if (newName.find_first_not_of(' ') ==
          std::string::npos) // does not allow user to input only blank
                             // characters
      {
        cout << "You cannot enter blank characters, returning to menu...";
        studentOrig.close();
        studentNew.close();
        remove("temp.txt");
        return;
      }

      if (newId.find_first_not_of(' ') == std::string::npos) {
        cout << "You cannot enter blank characters, returning to menu...";
        studentOrig.close();
        studentNew.close();
        remove("temp.txt");
        return;
      }

      if (duplicate(newId, 1) == true) {
        studentOrig.close();
        studentNew.close();
        remove("temp.txt");
        return;
      }

      studentNew << newName << " | " << newId << " | " << courseSelect() << " "
                 << endl;
    }
  }

  if (!found) {
    cout << "Student not found." << endl;
  } else {
    cout << "Student edited." << endl;
  }

  studentOrig.close();
  studentNew.close();
  remove("students.txt");
  rename("temp.txt", "students.txt");
}

void searchStudent() {
  ifstream studentList;
  string line;
  string searchName, searchId;
  bool found = false;

  cout << "Enter student name to search: ";
  cin.clear();
  cin.ignore();
  getline(cin, searchName);
  searchName = trimfnc(searchName);

  cout << "Enter student ID to search: ";
  getline(cin, searchId);
  searchId = trimfnc(searchId);

  studentList.open("students.txt", ios::in);

  if (studentList.is_open() == true) {
    cout << "NAME | ID NUMBER | COURSE" << endl;
    while (getline(studentList, line)) // find student with the associated name
                                       // and ID and show all information
    {
      if (line.find(searchName + " | " + searchId) != string::npos) {
        cout << line << endl;
        found = true;
      }
    }
    if (!found) {
      cout << "Student not found." << endl;
    }
  } else {
    cout << "Unable to open students.txt file." << endl;
  }
}

int main() {
  int i;
  bool exit = false;
  cout << "\nSimple Student Information System!" << endl;
  while (exit == false) {
    cout << endl;
    int y;
    cout << " 1) Add student/Course\n 2) Delete Student/Course\n 3) Edit "
            "Student/Course\n 4) List of Students/Course\n 5) Search student "
            "by name,id\n 10) Exit"
         << endl;
    cin >> i;

    switch (i) // options for user to select from
    {

    case 1:
      do {
        cin.clear();
        cin.ignore();
        cout << "input 1 to add a student or 2 to add a course:";
        cin >> y;
        cout << endl;
        if (y == 2)
          AddCourse();

        if (y == 1)
          AddStudent();

      } while (y != 1 && y != 2);

      break;

    case 2:
      do {
        cin.clear();
        cin.ignore();
        cout << "input 1 to delete a student or 2 to delete a course:";
        cin >> y;
        cout << endl;

        if (y == 1)
          deleteStudent();

        if (y == 2)
          deleteCourse();
      } while (y != 1 && y != 2);
      break;

    case 3:
      do {
        cin.clear();
        cin.ignore();
        cout << "input 1 to edit students or 2 to edit course:";
        cin >> y;
        cout << endl;

        if (y == 1)
          editStudent();

        if (y == 2)
          editCourse();

      } while (y != 1 && y != 2);
      break;

    case 4:
      do {

        cin.clear();
        cin.ignore();
        cout << "input 1 to list students or 2 to list course:";
        cin >> y;

        if (y == 1) {
          cout << endl;
          cout << "Listing Students..." << endl;
          cout << endl;
          cout << "NAME | ID NUMBER | COURSE" << endl;
          cout << endl;
          listFiles(1);
        }

        if (y == 2) {
          cout << "Listing Courses..." << endl;
          listFiles(2);
        }

      } while (y != 2 && y != 1);
      break;

    case 5:
      searchStudent();
      break;

    case 10:
      exit = true;
      break;

    default:
      cout << "Option not Found!" << endl;
    }
  }

  cout << "Thank you!" << endl;
  return 0;
}
