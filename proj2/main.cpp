#include "inf_int.h"
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <stdlib.h>
#include <algorithm>
#include <string>
#include <vector>
#include <sstream>

using namespace std;

int main()
{
	while (1) {

		inf_int a;
		inf_int b;
		inf_int c;

		vector<string> str;
		string token;
		string input;
		cout << "Input: ";
		getline(cin, input);
		if (input == "0")
			break;

		stringstream ss(input);

		while (getline(ss, token, ' ')) {
			str.push_back(token);
		}

		char ch[100];
		char temp;
		int i = 0;

		for (auto& elem : str) {
			if (i == 0) {
				strcpy(ch, elem.c_str());
				a = inf_int(ch);
			}
			if (i == 1) {
				strcpy(ch, elem.c_str());
				temp = ch[0];
			}
			if (i == 2) {
				strcpy(ch, elem.c_str());
				b = inf_int(ch);
			}
			i++;
		}
		if (temp == '+')
			c = a + b;
		else if (temp == '-')
			c = a - b;
		else if (temp == '*')
			c = a * b;
		cout << "Output: " << c << endl;
	}

	return 0;
}
