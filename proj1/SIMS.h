#pragma once

class Student {
public:
	std::string name;
	std::string studentID;
	std::string birthYear;
	std::string department;
	std::string tel;
	Student() {
	}
	Student(std::string name, std::string studentID, std::string birthYear, std::string department, std::string tel) :
		name(name), studentID(studentID), birthYear(birthYear), department(department), tel(tel) {
	}
	/*Student(std::string name, std::string studentID, std::string birthYear, std::string department, std::string tel) {
		this->name = name;
		this->studentID = studentID;
		this->birthYear = birthYear;
		this->department = department;
		this->tel = tel;
	}*/
};

class StudentDB {
public:
	std::string filename;
	std::list<Student> studentList;
	std::list<Student>::iterator iter = studentList.begin();

	StudentDB(std::string filename) {
		this->filename = filename;
	}

	void createObjectsFromFile();
	void printMenu();
	void insert(std::string, std::string, std::string, std::string, std::string);
	void search_menu();
	void sorting_menu();

	void insert_checkAndPassInput();
	int isAlreadyExist(std::string);

	void searchByName();
	void searchByID();
	void searchByADYear();
	void searchByDepart();
	void listAll();

	void sortByName();
	void sortByID();
	void sortByADYear();
	void sortByDepart();
};