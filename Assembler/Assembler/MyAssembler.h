#include<fstream>
#include<map>
#include<string>
#include<vector>
using namespace std;

/* types of instructions */
#define TwoOperand 0
#define OneOperand 1
#define Branch 2
#define NoOperand 3
/* addressing modes */
#define indexed 0
#define Auto_Decrement 1
#define Auto_Increment 2
#define Direct 3
/* Registers */
#define R0 0
#define R1 1
#define R2 2
#define R3 3

/* two_complement */
#define Twos_Complement true

class MyAssembler
{
	ifstream codeFile; // the file which contains the source code
	string codeFilePath;
	ofstream binaryCodeFile; // A binary file which conains the source code 

	map<string, string>instrTable; // codes of instructions
	map<int, string>addrModeTable; // codes of addressing modes
	map<int, string>registerTable; // codes of registers
	map<string, int>labelsTable; // labels which are used by the programmer (Branch)
	vector<pair<string, int>>branchesInstr;
	vector<pair<string, int>>jsrInstr;
	vector<string>x_indexedVec; // array of X (indexed mode) - format X(Ri)
	vector<string>DataRam; // data that will be put in the RAM
	vector<string>dataSeg;
	bool syntaxError; // boolean to check if there is a syntax error or not
	int numLine;      // the current line which is been decoding
	int codePointer;  /* pointer to the address that will be filled by the next
					  instruction in the RAM
					  */
	int labelPointer; /* pointer to the address that will be filled by the next
					  label in the RAM
					  */
	int extraWords;
	void generateTables(); // Build all tables
	void lineParsing(string sLine); // sLine is the line that will be parsed next
	void operandParsing(string op); /* op is operand that will be parsed next
									to know the addressing mode and the number
									register
									*/
	void toUpperCase(string &str); /* convert any lower case letters to upper
								   so the assembler is non-casesenstive
								   */

	string int2Binary(int num, bool twosComp);
	void printIndexedVec();
	void fillBranchJSRinstr(); // complete DataRam at branchesInstr && jsrInstr
	void printDataRAM();
	void errorMessege(); // to print syntax error 
	void scanDataSegment();
	void scanLables(); // scan code.txt to find all lables
public:
	MyAssembler(string cfPath); // the path of the source code file
	void run(); // run the Assembler :D
	bool get_syntaxError(); // getter of syntaxError
	~MyAssembler();
};

