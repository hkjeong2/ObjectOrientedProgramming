#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <list>
#include "SIMS.h"

using namespace std;

int main(int arg, char* argv[]) {


	StudentDB studentDB("file1.txt");
	studentDB.createObjectsFromFile();
	studentDB.sortByName();
	studentDB.printMenu();


}