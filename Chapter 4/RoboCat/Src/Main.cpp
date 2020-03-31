#include "RoboCatPCH.h"
#include <iostream>

const int MTU = 1300;

using namespace std;

int main()
{
	OutputMemoryStream outStream;
	vector<int> v({ 2, 4, 41, 4 });
	unordered_map<int, int> map({ {1, 5}, {3, 5}, {4, 6} });
	unordered_map<float, int> mapF({ {1.5f, -33}, {-2.1f, -25}, {3.14f, 543} });
	outStream.Write(v);
	outStream.Write(map);
	outStream.Write(mapF);
	int test = 228;
	outStream.Serialize(test);

	char* temporaryBuffer = static_cast<char*>(std::malloc(outStream.GetLength()));
	std::memcpy(temporaryBuffer, outStream.GetBufferPtr(), outStream.GetLength());
	InputMemoryStream inStream(temporaryBuffer, outStream.GetLength());
	vector<int> newV;
	unordered_map<int, int> newMap;
	unordered_map<float, int> newMapF;
	inStream.Read(newV);
	inStream.Read(newMap);
	inStream.Read(newMapF);

	for (int e : newV)
	{
		cout << e << " ";
	}

	cout << endl;

	for (pair<int, int> e : newMap)
	{
		cout << e.first << ", " << e.second << " | ";
	}

	cout << endl;

	for (pair<float, int> e : newMapF)
	{
		cout << e.first << ", " << e.second << " | ";
	}

	cout << endl;

	int newTest;
	inStream.Serialize(newTest);
	cout << newTest << endl;

	return 0;
}