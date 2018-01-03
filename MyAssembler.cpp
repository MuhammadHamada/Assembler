#include "MyAssembler.h"
#include "Utils.h"
#include <iostream>
using namespace std;


void MyAssembler::generateTables()
{
	/* Generate 2 operand instructions table */
	instrTable["ADD"] = "000000";
	instrTable["ADC"] = "000001";
	instrTable["SUB"] = "000010";
	instrTable["SBC"] = "000011";
	instrTable["AND"] = "000100";
	instrTable["OR"] = "000101";
	instrTable["XOR"] = "000110";
	instrTable["BIC"] = "000111";
	instrTable["MOV"] = "001000";
	instrTable["BIS"] = "000101";
	instrTable["CMP"] = "000010";

	/* Generate 1 operand instructions table */
	instrTable["INC"] = "010000";
	instrTable["DEC"] = "010001";
	instrTable["CLR"] = "010010";

	instrTable["LSR"] = "010100";
	instrTable["ROR"] = "010101";
	instrTable["RRC"] = "010110";
	instrTable["ASR"] = "010111";
	instrTable["LSL"] = "011000";
	instrTable["ROL"] = "011001";
	instrTable["RLC"] = "011010";
	instrTable["INV"] = "011011";

	/* Generate Branching instructions table */
	instrTable["BR"] = "100000";
	instrTable["BEQ"] = "100001";
	instrTable["BNE"] = "100010";
	instrTable["BLO"] = "100011";
	instrTable["BLS"] = "100100";
	instrTable["BHI"] = "100101";
	instrTable["BHS"] = "100110";

	/* Generate No operand instructions table */
	instrTable["HLT"] = "110000";
	instrTable["NOP"] = "110001";
	instrTable["RESET"] = "110010";
	instrTable["JSR"] = "110011";
	instrTable["RTS"] = "110100";

	/* Generate Addressing mode table */
	addrModeTable[Direct] = "00";
	addrModeTable[Auto_Increment] = "01";
	addrModeTable[Auto_Decrement] = "10";
	addrModeTable[indexed] = "11";

	/* Generate Register table */
	registerTable[R0] = "00";
	registerTable[R1] = "01";
	registerTable[R2] = "10";
	registerTable[R3] = "11";
}

MyAssembler::MyAssembler(string cfPath,string bcPath)
{
	DataRam.resize(2048);
	codePointer = labelPointer = 0; // starting address of code segment
	extraWords = 0;
	syntaxError = false;
	numLine = 1;
	codeFilePath = cfPath;
	this->codeFile.open(cfPath);
	this->binaryCodeFile.open(bcPath);
	this->generateTables();
}

void MyAssembler::lineParsing(string sLine)
{
	if (sLine == "")return;
	Utils::toUpperCase(sLine);


	int lstIndx = 0; // next index on the line that will be parsed

					 /* search for labels */
	string lbl = "";
	for (int i = 0; i < sLine.size(); i++) {

		if (sLine[i] == ':') {
			lstIndx = i;
			labelsTable[lbl] += extraWords;
			break;
		}
		else {
			if (sLine[i] != ' ')
				lbl += sLine[i];
		}
	}

	/* search for instruction */
	string instr = "";
	for (int i = lstIndx; i < sLine.size(); i++) {
		lstIndx++;
		if ((sLine[i] == ' ' && !instr.empty()) || sLine[i] == ',')break;
		if (sLine[i] < 'A' || sLine[i] > 'Z')continue;
		if (sLine[i] != ' ')
			instr += sLine[i];
	}
	if (instr == "")return;
	if (instrTable.count(instr) == 0) {
		this->errorMessege();
		cout << instr << " -- invalid instruction" << endl;
		return;
	}
	string opcode = instrTable[instr];
	this->DataRam[codePointer] += opcode;
	int x = (opcode[0] - '0') * 2 + (opcode[1] - '0');
	string src = "", dst = "";
	switch (x)
	{
	case TwoOperand:
	{
		bool coma = false;
		for (int i = lstIndx; i < sLine.size(); i++) {
			if (sLine[i] == ',') {
				coma = true;
				continue;
			}
			if (sLine[i] != ' ') {
				if (!coma)src += sLine[i];
				else dst += sLine[i];
			}
		}
		if (!coma)this->errorMessege();
		this->operandParsing(src);
		this->operandParsing(dst);
		if(instr == "MOV")this->DataRam[codePointer] += "01";
		else if (instr == "CMP")this->DataRam[codePointer] += "10";
		else this->DataRam[codePointer] += "00";
		codePointer++;
		printIndexedVec();
		break;
	}
	case OneOperand:
	{
		this->DataRam[codePointer] += "0000";
		for (int i = lstIndx; i < sLine.size(); i++) {
			if (sLine[i] != ' ')dst += sLine[i];
		}
		this->operandParsing(dst);
		this->DataRam[codePointer] += "00";
		codePointer++;
		printIndexedVec();
		break;
	}
	case Branch:
	{
		lbl = "";
		for (int i = lstIndx; i < sLine.size(); i++) {
			if (sLine[i] == ' ')continue;
			lbl += sLine[i];
		}
		if (this->labelsTable.count(lbl) == 0) {
			this->errorMessege();
			cout << "Label '" << lbl << "' is undefined" << endl;
		}
		else {
			branchesInstr.push_back({ lbl,codePointer + 1 });
			codePointer++;
		}
		break;
	}
	default:
	{
		// No operand instructions
		for (int i = 0; i < 10; i++) {
			this->DataRam[codePointer] += '0';
		}
		codePointer++;
		if (opcode == instrTable["JSR"]) {
			extraWords++;
			string subRt = "";
			for (int i = lstIndx; i < sLine.size(); i++) {
				if (sLine[i] == ' ')continue;
				subRt += sLine[i];
			}
			jsrInstr.push_back({ subRt ,codePointer });
			codePointer++;
		}
		break;
	}
	}
}

void MyAssembler::operandParsing(string op)
{
	int siz = op.size();
	string x_indexed;
	if (siz > 4) {
		x_indexed = op.substr(0, siz - 4);
	}
	int regNum;
	if (siz == 2 && op[0] == 'R' && op[1] >= '0' && op[1] < '4') {
		//format --> Ri where i = 0 , 1 , 2 or 3
		this->DataRam[codePointer] += addrModeTable[Direct];
		regNum = (op[1] - '0');
	}
	else if (siz == 5 && op[siz - 1] == '+' && op[siz - 2] == ')'
		&& (op[siz - 3] >= '0' && op[siz - 3] < '4') && op[siz - 4] == 'R' && op[siz - 5] == '(') {
		//format --> (Ri)+ where i = 0 , 1 , 2 or 3
		this->DataRam[codePointer] += addrModeTable[Auto_Increment];
		regNum = (op[siz - 3] - '0');
	}
	else if (siz == 5 && op[0] == '-' && op[1] == '('
		&& (op[3] >= '0' && op[3] < '4') && op[2] == 'R' && op[4] == ')') {
		//format --> -(Ri) where i = 0 , 1 , 2 or 3
		this->DataRam[codePointer] += addrModeTable[Auto_Decrement];
		regNum = (op[3] - '0');
	}
	else if (siz >= 5 && op[siz - 1] == ')' && op[siz - 2] >= '0' && op[siz - 2] < '4'
		&& op[siz - 3] == 'R' && op[siz - 4] == '(') {
		//format --> X(Ri) where i = 0 , 1 , 2 or 3 and X is an offset
		this->DataRam[codePointer] += addrModeTable[indexed];
		regNum = (op[siz - 2] - '0');
		x_indexedVec.push_back(Utils::int2Binary(stoi(x_indexed), !Twos_Complement));
	}
	else {
		if (!get_syntaxError())
			this->errorMessege();
		return;
	}
	switch (regNum)
	{
	case R0:
		this->DataRam[codePointer] += registerTable[R0];
		break;
	case R1:
		this->DataRam[codePointer] += registerTable[R1];
		break;
	case R2:
		this->DataRam[codePointer] += registerTable[R2];
		break;
	default:
		this->DataRam[codePointer] += registerTable[R3];
		break;
	}
}


void MyAssembler::run()
{
	scanLables();
	this->codeFile.open(this->codeFilePath);
	string line;
	bool foundCode = false;
	while (!this->codeFile.eof() && !syntaxError) {
		getline(codeFile, line);
		if (line == "") {
			numLine++;
			scanDataSegment();
			break;
		}
		lineParsing(line);
		numLine++;
	}
	fillBranchJSRinstr();
	this->printDataRAM();
}

void MyAssembler::convertToMemFile(string fpath)
{
	ofstream file;
	file.open(fpath);
	file << "// memory data file (do not edit the following line - required for mem load use)\n";
	file << "// instance=/pu/RAM_LAB/MEMORY\n";
	file << "// format=bin addressradix=h dataradix=b version=1.0 wordsperline=1";
	file << "wordsperline=1" << endl;
	for (int i = 0; i < this->DataRam.size(); i++)
	{
		if (i <= 0xf) {
			file << "  @";
		}
		else if (i <= 0xff) {
			file << " @";
		}
		else {
			file << "@";
		}
		Utils::decToHexa(i, file);
		file << " ";
		if (this->DataRam[i] == "") {
			int x = 16;
			while (x--) {
				file << 'X';
			}
		}
		else {
			file << this->DataRam[i];
		}
		file << endl;
	}

}

bool MyAssembler::get_syntaxError()
{
	return syntaxError;
}


void MyAssembler::printIndexedVec()
{
	for (int i = 0; i < x_indexedVec.size(); i++) {
		//binaryCodeFile << x_indexedVec[i] << endl;
		this->DataRam[codePointer] = x_indexedVec[i];
		codePointer++;
		extraWords++;
	}
	x_indexedVec.clear();
}

void MyAssembler::fillBranchJSRinstr()
{
	for (int i = 0; i < branchesInstr.size(); i++) {
		string lbl = branchesInstr[i].first;
		int cp = branchesInstr[i].second;
		int nxtInstr = labelsTable[lbl];
		if (nxtInstr < cp) {
			this->DataRam[cp-1] += Utils::int2Binary(cp - nxtInstr, Twos_Complement);
		}
		else {
			string str = Utils::int2Binary(nxtInstr - cp, !Twos_Complement);
			this->DataRam[cp-1] += str.substr(6, str.size() - 6);
		}
	}
	for (int i = 0; i < jsrInstr.size(); i++) {
		string lbl = jsrInstr[i].first;
		int cp = jsrInstr[i].second;
		int nxtInstr = labelsTable[lbl];
		this->DataRam[cp] += Utils::int2Binary(nxtInstr, !Twos_Complement);
	}
}

void MyAssembler::printDataRAM()
{
	for (int i = 0; i < DataRam.size(); i++) {
		if (DataRam[i] == "") {
		binaryCodeFile << endl;
		continue;
		}
		binaryCodeFile << DataRam[i] << endl;
	}
}

void MyAssembler::errorMessege()
{
	syntaxError = true;
	cout << "Syntax Error --- " << "Line number " << numLine << endl;
}

void MyAssembler::scanDataSegment()
{

	long long address, data;
	while (!this->codeFile.eof() && !syntaxError) {
		codeFile >> address >> data;
		if (address < codePointer) {
			cout << "Data Segmnet error -- " <<  address  
				<< " is invalid address" << endl;
			cout << "Line (" << numLine << ")" << endl;
			syntaxError = true;
			break;
		}
		string num = Utils::int2Binary(data, !Twos_Complement);
		while (num.size() < 32) {
			num = "0" + num;
		}
		DataRam[address] = num.substr(16, 16);
		DataRam[address+1] = num.substr(0, 16);
		numLine++;
	}
}

void MyAssembler::scanLables()
{
	string line;
	while (!this->codeFile.eof() && !syntaxError) {
		getline(codeFile, line);
		if (line == "")continue;
		Utils::toUpperCase(line);
		bool labelFound = false;
		string lbl = "", instr = "";
		int lstIndx = 0;
		for (int i = lstIndx; i < line.size(); i++) {
			lstIndx++;
			if (line[i] == ':') {
				labelFound = true;
				break;
			}
			else {
				lbl += line[i];
			}
		}
		for (int i = lstIndx; i < line.size(); i++) {
			if (line[i] != ' ') {
				instr += line[i];
			}
		}
		if (labelFound) {
			if (labelsTable.count(lbl) != 0) {
				this->errorMessege();
				cout << "Label '" << lbl << "' comes more than once" << endl;
			}
			this->labelsTable[lbl] = labelPointer;
			if (instr != "")labelPointer++;
		}
		else {
			if (line != "")
				labelPointer++;
		}
		numLine++;
	}
	numLine = 1;
	this->codeFile.close();
}


MyAssembler::~MyAssembler()
{
	this->codeFile.close();
	if (this->get_syntaxError()) {
		this->binaryCodeFile.clear();
		this->binaryCodeFile.open(this->binaryCodeFilePath);
		this->binaryCodeFile.close();
	}
	this->binaryCodeFile.close();
}
