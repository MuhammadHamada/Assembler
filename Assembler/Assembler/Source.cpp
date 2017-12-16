#include "MyAssembler.h"
#include<iostream>
#include<fstream>
using namespace std;
string codeFilePath = "code.txt";
string ramFilePath = "binaryCode.txt";
string memRamFilePath = "RAM.mem";

int main()
{
	MyAssembler Assembler(codeFilePath, ramFilePath);
	Assembler.run();
	if (!Assembler.get_syntaxError()) {
		cout << "Converting to mem file , please wait ..." << endl;
		Assembler.convertToMemFile(memRamFilePath);
		cout << "Done -- Have Fun" << endl;
	}
	return 0;
}