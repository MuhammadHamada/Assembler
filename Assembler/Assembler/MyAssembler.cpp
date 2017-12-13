#include "MyAssembler.h"
#include <iostream>
using namespace std;


void MyAssembler::generateTables()
{
	/* Generate 2 operand instructions table */
	instrTable["MOV"] = "000000";
	instrTable["ADD"] = "000001";
	instrTable["ADC"] = "000010";
	instrTable["SUB"] = "000011";
	instrTable["SBC"] = "000100";
	instrTable["AND"] = "000101";
	instrTable["OR"] = "000110";
	instrTable["XOR"] = "000111";
	instrTable["BIS"] = "001000";
	instrTable["BIC"] = "001001";
	instrTable["CMP"] = "001010";

	/* Generate 1 operand instructions table */
	instrTable["INC"] = "010000";
	instrTable["DEC"] = "010001";
	instrTable["CLR"] = "010010";
	instrTable["INV"] = "010011";
	instrTable["LSR"] = "010100";
	instrTable["ROR"] = "010101";
	instrTable["RRC"] = "010110";
	instrTable["ASR"] = "010111";
	instrTable["LSL"] = "011000";
	instrTable["ROL"] = "011001";
	instrTable["RLC"] = "011010";

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
	addrModeTable[indexed] = "00";
	addrModeTable[Auto_Decrement] = "01";
	addrModeTable[Auto_Increment] = "10";
	addrModeTable[Direct] = "11";

	/* Generate Register table */
	registerTable[indexed] = "00";
	registerTable[Auto_Decrement] = "01";
	registerTable[Auto_Increment] = "10";
	registerTable[Direct] = "11";
}

MyAssembler::MyAssembler(string cfPath)
{
	DataRam.resize(2048);
	codePointer = labelPointer = 10; // starting address of code segment
	extraWords = 0;
	syntaxError = false;
	numLine = 1;
	codeFilePath = cfPath;
	this->codeFile.open(cfPath);
	this->binaryCodeFile.open("binaryCode.txt");
	this->generateTables();
}

void MyAssembler::lineParsing(string sLine)
{
	if (sLine == "")return;
	toUpperCase(sLine);


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
		this->DataRam[codePointer] += "00";
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
			branchesInstr.push_back({ lbl,codePointer });
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
		x_indexedVec.push_back(int2Binary(stoi(x_indexed), !Twos_Complement));
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

void MyAssembler::toUpperCase(string &str)
{
	for (int i = 0; i < str.size(); i++)
	{
		if (str[i] >= 'a'&&str[i] <= 'z') {
			str[i] = ('A' + (str[i] - 'a'));
		}
	}
}

void MyAssembler::run()
{
	scanDataSegment();
	scanLables();
	this->codeFile.open(this->codeFilePath);
	string line;
	bool foundCode = false;
	while (!this->codeFile.eof() && !syntaxError) {
		getline(codeFile, line);
		toUpperCase(line);
		if (line == ".CODE")foundCode = true;
		if (!foundCode || line == ".CODE")continue;
		lineParsing(line);
		numLine++;
	}
	fillBranchJSRinstr();
	this->printDataRAM();
}

bool MyAssembler::get_syntaxError()
{
	return syntaxError;
}

string MyAssembler::int2Binary(int num, bool twosComp)
{
	string ret = "0000000000000000";
	if (num == 0)return ret;
	string ans;
	int x = 0;
	while (num > 0)
	{
		x = num % 2;
		num /= 2;
		ans += ('0' + x);
	}
	if (twosComp) {
		bool one = 0;
		for (int i = 0; i < 10; i++) {
			if (i >= ans.size())ans += '0';
		}
		for (int i = 0; i < ans.size(); i++)
		{
			if (ans[i] == '1' && !one) {
				one = true;
				continue;
			}
			if (!one)continue;
			ans[i] = '0' + (ans[i] == '0');
		}
		reverse(ans.begin(), ans.end());
		return ans;
	}
	reverse(ans.begin(), ans.end());
	if (ans.size() > 16)return ans;
	for (int i = (int)ans.size() - 1, j = 15; i >= 0; i--) {
		ret[j] = ans[i];
		j--;
	}
	return ret;
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
			this->DataRam[cp] += int2Binary(cp - nxtInstr, Twos_Complement);
		}
		else {
			string str = int2Binary(nxtInstr - cp, !Twos_Complement);
			this->DataRam[cp] += str.substr(6, str.size() - 6);
		}
	}
	for (int i = 0; i < jsrInstr.size(); i++) {
		string lbl = jsrInstr[i].first;
		int cp = jsrInstr[i].second;
		int nxtInstr = labelsTable[lbl];
		this->DataRam[cp] += int2Binary(nxtInstr, !Twos_Complement);
	}
}

void MyAssembler::printDataRAM()
{
	for (int i = 0; i < dataSeg.size(); i++) {
		DataRam[i] = dataSeg[i];
	}
	for (int i = 0; i < DataRam.size(); i++) {
		/*if (DataRam[i] == "") {
		for (int j = 0; j < 16; j++) {
		binaryCodeFile << 0;
		}
		binaryCodeFile << endl;
		continue;
		}
		*/
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
	string line;
	bool foundData = false;
	while (!this->codeFile.eof() && !syntaxError) {
		getline(codeFile, line);
		if (line == "")continue;
		toUpperCase(line);
		if (line == ".DATA") {
			foundData = true;
			continue;
		}
		else if (line == ".CODE")break;

		if (foundData) {
			for (int i = 0; i < line.size(); i++) {
				if (line[i]<'0' || line[i] > '9') {
					this->errorMessege();
					cout << "please insert a decimal number" << endl;
					return;
				}
			}
			string num = int2Binary(stoi(line), !Twos_Complement);
			while (num.size() < 32) {
				num = "0" + num;
			}
			dataSeg.push_back(num.substr(16, 16));
			dataSeg.push_back(num.substr(0, 16));
		}
	}
}

void MyAssembler::scanLables()
{
	string line;
	while (!this->codeFile.eof() && !syntaxError) {
		getline(codeFile, line);
		if (line == "")continue;
		toUpperCase(line);
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
		this->binaryCodeFile.open("binaryCode.txt");
		this->binaryCodeFile.close();
	}
	this->binaryCodeFile.close();
}
