#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <list>
#include "SIMS.h"

using namespace std;

void StudentDB::createObjectsFromFile() {
	string line;
	ifstream fin(filename);
	int index = 0;
	string token;
	string aToken[100];

	if (fin.fail()) {
		ofstream fout(filename);
		ifstream fin(filename);
	}
	if (fin.is_open()) {
		while (getline(fin, line)) {
			if (line.length() == 0)
				break;
			istringstream iss(line);
			while (getline(iss, token, '\n')) {
				aToken[index] = token;
			}
			index += 1;
		}
	}
	if (aToken[0].compare("") != 0) {
		for (int i = 0; i < index; i++) {
			Student st(aToken[i].substr(aToken[i].find_first_not_of((" "), 0), 20 - aToken[i].find_first_not_of((" "), 0)),
				aToken[i].substr(aToken[i].find_first_not_of((" "), 20), 40 - aToken[i].find_first_not_of((" "), 20)),
				aToken[i].substr(aToken[i].find_first_not_of((" "), 40), 60 - aToken[i].find_first_not_of((" "), 40)),
				aToken[i].substr(aToken[i].find_first_not_of((" "), 60), 80 - aToken[i].find_first_not_of((" "), 60)),
				aToken[i].substr(aToken[i].find_first_not_of((" "), 80), 100 - aToken[i].find_first_not_of((" "), 80)));
			studentList.push_back(st);
		}
	}
}
void StudentDB::printMenu() {
	int option;
	cout << "1.Insertion\n" << "2.Search\n" << "3.Sorting Option\n" << "4.Exit\n\n" << "> ";
	cin >> option;
	cin.clear();
	cin.ignore(999, '\n');
	if (option >= 1 && option <= 4) {
		switch (option) {
		case 1:
			insert_checkAndPassInput();
			break;
		case 2:
			if (studentList.size() == 0) {
				cout << "\nInsert data first\n" << endl;
				printMenu();
			}
			else {
				search_menu();
			}
			break;
		case 3:
			if (studentList.size() == 0) {
				cout << "\nInsert data first\n" << endl;
				printMenu();
			}
			else {
				sorting_menu();
			}
			break;
		case 4:
			cout << "\nterminating ... " << endl;;
			exit(1);
		}
	}
	else {
		cout << "\ninput a number between 1~4\n";
		printMenu();
	}
}

void StudentDB::insert(string name, string studentID, string birthYear, string department, string tel) {
	Student student(name, studentID, birthYear, department, tel);

	studentList.push_back(student);

	ofstream fout(filename, ios::app);
	cout.setf(ios::left);
	fout << setw(20) << name << setw(20) << studentID << setw(20) << birthYear << setw(20) << department << setw(20) << tel << endl;

	cout << "\nThe student information has been inserted\n\n";
	sortByName();
	printMenu();
}

void StudentDB::insert_checkAndPassInput() {
	string name;
	string studentID;
	string birthYear;
	string department;
	string tel;

	cout << "\nName: ";
	getline(cin, name);
	while (name.length()>=16) {
		cout << "Error : Name is up to 15 words\n";
		cout << "Name: ";
		getline(cin, name);
	}
	cout << "Student ID (10 digits): ";
	getline(cin, studentID);
	while (studentID.length() != 10 || (isAlreadyExist(studentID)) == 1) {
		if (studentID.length() != 10) {
			cout << "Error: ID must be 10 digits\n";
		}
		else if ((isAlreadyExist(studentID)) == 1) {
			cout << "Error : Already inserted\n";
		}
		cout << "Student ID (10 digits): ";
		getline(cin, studentID);
	}
	cout << "Birth Year (4 digits): ";
	getline(cin, birthYear);
	while (birthYear.length() != 4) {
		cout << "Error : Birth Year must be 4 digits\n";
		cout << "Birth Year (4 digits): ";
		getline(cin, birthYear);
	}
	cout << "Department: ";
	getline(cin, department);
	while (department.length() >= 19) {
		cout << "Error : Department is up to 18 digits\n";
		cout << "Department: ";
		getline(cin, department);
	}
	cout << "Tel: ";
	getline(cin, tel);
	while (tel.length() >= 13) {
		cout << "Error : Telephone number is up to 12 digits\n";
		cout << "Tel: ";
		getline(cin, tel);
	}

	insert(name,studentID, birthYear, department, tel);
}

int StudentDB::isAlreadyExist(string str) {
	for (iter = studentList.begin(); iter != studentList.end(); iter++) {
		if (str.compare((*iter).studentID) == 0)
			return 1;
	}
	return 0;
}
void StudentDB::search_menu() {
	cout << "\n- Search -\n" << "1. Search by name\n" << "2. Search by student ID (10 numbers)\n" << "3. Search by admission year (4 numbers)\n" << "4. Search by department name\n" << "5. List All\n\n" << "> ";
	int option;
	cin >> option;
	cin.clear();
	cin.ignore(999, '\n');
	if (option >= 1 && option <= 5) {
		switch (option) {
		case 1:
			searchByName();
			break;
		case 2:
			searchByID();
			break;
		case 3:
			searchByADYear();
			break;
		case 4:
			searchByDepart();
			break;
		case 5:
			listAll();
			break;
		}
	}
	else {
		cout << "\ninput a number between 1~4\n";
		search_menu();
	}
}
void StudentDB::searchByName() {
	string name;
	bool is_exist = false;
	cout << "\nName: ";
	getline(cin, name);
	cout << "\n";
	cout.setf(ios::left);
	for (iter = studentList.begin(); iter != studentList.end(); iter++) {
		if (name.compare((*iter).name) == 0) {
			is_exist = true;
			break;
		}
	}
	if (is_exist) {
		cout << setw(20) << "Name" << setw(20) << "StudentID" << setw(20) << "Dept" << setw(20) << "Birth Year" << setw(20) << "Tel" << endl;
		for (iter = studentList.begin(); iter != studentList.end(); iter++) {
			if (name.compare((*iter).name) == 0) {
				cout << setw(20) << (*iter).name << setw(20) << (*iter).studentID << setw(20) << (*iter).department << setw(20) << (*iter).birthYear << setw(20) << (*iter).tel << endl;
			}
		}
	}
	else {
		cout << "No such result found\n";
	}
	cout << "\n";
	printMenu();
}
void StudentDB::searchByID() {
	string studentID;
	bool is_exist = false;
	cout << "\nStudentID: ";
	getline(cin, studentID);
	cout << "\n";
	cout.setf(ios::left);
	for (iter = studentList.begin(); iter != studentList.end(); iter++) {
		if (studentID.compare((*iter).studentID) == 0) {
			is_exist = true;
			break;
		}
	}
	if (is_exist) {
		cout << setw(20) << "Name" << setw(20) << "StudentID" << setw(20) << "Dept" << setw(20) << "Birth Year" << setw(20) << "Tel" << endl;
		for (iter = studentList.begin(); iter != studentList.end(); iter++) {
			if (studentID.compare((*iter).studentID) == 0) {
				cout << setw(20) << (*iter).name << setw(20) << (*iter).studentID << setw(20) << (*iter).department << setw(20) << (*iter).birthYear << setw(20) << (*iter).tel << endl;
			}
		}
	}
	else {
		cout << "No such result found\n";
	}
	cout << "\n";
	printMenu();
}
void StudentDB::searchByADYear() {
	string adYear;
	bool is_exist = false;
	cout << "\nAdmission year: ";
	getline(cin, adYear);
	cout << "\n";
	cout.setf(ios::left);
	for (iter = studentList.begin(); iter != studentList.end(); iter++) {
		if (adYear.compare((*iter).studentID.substr(0, 4)) == 0) {
			is_exist = true;
			break;
		}
	}
	if (is_exist) {
		cout << setw(20) << "Name" << setw(20) << "StudentID" << setw(20) << "Dept" << setw(20) << "Birth Year" << setw(20) << "Tel" << endl;
		for (iter = studentList.begin(); iter != studentList.end(); iter++) {
			if (adYear.compare((*iter).studentID.substr(0, 4)) == 0) {
				cout << setw(20) << (*iter).name << setw(20) << (*iter).studentID << setw(20) << (*iter).department << setw(20) << (*iter).birthYear << setw(20) << (*iter).tel << endl;
			}
		}
	}
	else {
		cout << "No such result found\n";
	}
	cout << "\n";
	printMenu();
}
void StudentDB::searchByDepart() {
	string dept;
	bool is_exist = false;
	cout << "\nDepartment name: ";
	getline(cin, dept);
	cout << "\n";
	cout.setf(ios::left);
	for (iter = studentList.begin(); iter != studentList.end(); iter++) {
		if (dept.compare((*iter).department) == 0) {
			is_exist = true;
			break;
		}
	}
	if (is_exist) {
		cout << setw(20) << "Name" << setw(20) << "StudentID" << setw(20) << "Dept" << setw(20) << "Birth Year" << setw(20) << "Tel" << endl;
		for (iter = studentList.begin(); iter != studentList.end(); iter++) {
			if (dept.compare((*iter).department) == 0) {
				cout << setw(20) << (*iter).name << setw(20) << (*iter).studentID << setw(20) << (*iter).department << setw(20) << (*iter).birthYear << setw(20) << (*iter).tel << endl;
			}
		}
	}
	else {
		cout << "No such result found\n";
	}
	cout << "\n";
	printMenu();
}
void StudentDB::listAll() {
	cout << "\n";
	cout.setf(ios::left);
	cout << setw(20) << "Name" << setw(20) << "StudentID" << setw(20) << "Dept" << setw(20) << "Birth Year" << setw(20) << "Tel" << endl;
	for (iter = studentList.begin(); iter != studentList.end(); iter++) {
		cout << setw(20) << (*iter).name << setw(20) << (*iter).studentID << setw(20) << (*iter).department << setw(20) << (*iter).birthYear << setw(20) << (*iter).tel << endl;
	}
	cout << "\n";
	printMenu();
}
void StudentDB::sorting_menu() {
	cout << "\n- Sorting Option -\n" << "1. Sort by Name\n" << "2. Sort by Student ID\n" << "3. Sort by Admission Year\n" << "4. Sort by Department Name\n\n" << "> ";
	int option;
	cin >> option;
	cin.clear();
	cin.ignore(999, '\n');
	if (option >= 1 && option <= 4) {
		switch (option) {
		case 1:
			sortByName();
			cout << "\nThe list has been sorted by name\n" << endl;
			printMenu();
			break;
		case 2:
			sortByID();
			cout << "\nThe list has been sorted by student ID\n" << endl;
			printMenu();
			break;
		case 3:
			sortByADYear();
			cout << "\nThe list has been sorted by admission year\n" << endl;
			printMenu();
			break;
		case 4:
			sortByDepart();
			cout << "\nThe list has been sorted by department name\n" << endl;
			printMenu();
			break;
		}
	}
	else {
		cout << "\ninput a number between 1~4\n";
		sorting_menu();
	}
}
void StudentDB::sortByName() {
	studentList.sort([](const Student& student1, const Student& student2) {
		if (student1.name == student2.name)
			return student1.studentID < student2.studentID;
		return student1.name < student2.name;
		});
}
void StudentDB::sortByID() {
	studentList.sort([](const Student& student1, const Student& student2) {
		return student1.studentID < student2.studentID;
		});
}
void StudentDB::sortByADYear() {
	sortByName();
	studentList.sort([](const Student& student1, const Student& student2) {
		return student1.studentID.substr(0, 4) < student2.studentID.substr(0, 4);
		});
}
void StudentDB::sortByDepart() {
	sortByName();
	studentList.sort([](const Student& student1, const Student& student2) {
		return student1.department < student2.department;
		});
}









