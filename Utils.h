#include<fstream>
#include<string>
using namespace std;

class Utils
{
public:
	Utils();
	static void toUpperCase(string &str); /* convert any lower case letters to upper
								   so the assembler is non-casesenstive
								   */
	static string int2Binary(long long num, bool twosComp);
	static void decToHexa(int n,ofstream &file);
	~Utils();
};

