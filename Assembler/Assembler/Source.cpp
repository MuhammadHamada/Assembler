#include "MyAssembler.h"
#include<iostream>
#include<fstream>
using namespace std;
string codeFilePath = "code.txt";
int main()
{
	MyAssembler Assembler(codeFilePath);
	Assembler.run();
	if (!Assembler.get_syntaxError()) {
		cout << "Done" << endl;
	}
	return 0;
}