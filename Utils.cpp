#include "Utils.h"



Utils::Utils()
{
}


void Utils::toUpperCase(string &str)
{
	for (int i = 0; i < str.size(); i++)
	{
		if (str[i] >= 'a'&&str[i] <= 'z') {
			str[i] = ('A' + (str[i] - 'a'));
		}
	}
}

string Utils::int2Binary(long long num, bool twosComp)
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

// function to convert decimal to hexadecimal
void Utils::decToHexa(int n,ofstream &file)
{
	// char array to store hexadecimal number
	char hexaDeciNum[100];

	// counter for hexadecimal number array
	int i = 0;
	while (n != 0)
	{
		// temporary variable to store remainder
		int temp = 0;

		// storing remainder in temp variable.
		temp = n % 16;

		// check if temp < 10
		if (temp < 10)
		{
			hexaDeciNum[i] = temp + 48;
			i++;
		}
		else
		{
			hexaDeciNum[i] = temp + 55;
			i++;
		}

		n = n / 16;
	}

	// printing hexadecimal number array in reverse order
	bool f = false;
	for (int j = i - 1; j >= 0; j--) {
		if (hexaDeciNum[j] >= 'A'&& hexaDeciNum[j] <= 'F') {
			char ch = tolower(hexaDeciNum[j]);
			file << ch;
		}
		else
			file << hexaDeciNum[j];
		f = true;
	}
	if (!f)file << '0';
}

Utils::~Utils()
{
}
