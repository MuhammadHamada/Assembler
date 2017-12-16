#include "MyAssembler.h"
#include<iostream>
#include<fstream>
using namespace std;
string codeFilePath = "code.txt";
string ramFilePath = "binaryCode.txt";
int main()
{
	MyAssembler Assembler(codeFilePath, ramFilePath);
	Assembler.run();
	if (!Assembler.get_syntaxError()) {
		cout << "Done -- Have Fun" << endl;
	}
	return 0;
}