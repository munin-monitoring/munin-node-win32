// PdhTests.cpp : définit le point d'entrée pour l'application console.
//

#include "stdafx.h"
#include <iostream>

using namespace std;

const size_t BUFF_SIZE = 4 * 1024 * 1024; // 4MiB

vector<string> convertToVectOfString(const char* str, DWORD len) {
		vector<string> strings;
		const CHAR* s = str;
		for (DWORD i = 0; i < len; i ++) {
			if (str[i] != 0) {
				continue;
			}
			string currentString = string(s);
			if (currentString.size() == 0) break;
			strings.push_back(currentString);
			s = str + i + 1;
		}
		return strings;
}

int _tmain(int argc, _TCHAR* argv[])
{
	PDH_STATUS status;

	vector<string> objects;
	{
		DWORD len = BUFF_SIZE;
		PZZSTR objectList = (PZZSTR) malloc(len);
		status = PdhEnumObjectsA(0, 0, objectList, &len, PERF_DETAIL_WIZARD, false);  
		objects = convertToVectOfString(objectList, len);
		free(objectList);
	}

	for (auto i = objects.begin(); i != objects.end(); i ++) {
		string object = *i;
		DWORD lenCounterList = BUFF_SIZE;
		PZZSTR counterList = (PZZSTR) malloc(lenCounterList);
		DWORD lenInstanceList = BUFF_SIZE;
		PZZSTR instanceList = (PZZSTR) malloc(lenInstanceList);
		status = PdhEnumObjectItemsA(0, 0, object.c_str(), counterList, &lenCounterList, instanceList, &lenInstanceList, PERF_DETAIL_WIZARD, true);  
		
		vector<string> counters = convertToVectOfString(counterList, lenCounterList);
		vector<string> instances = convertToVectOfString(instanceList, lenInstanceList);

		free(counterList);
		free(instanceList);

		// Iterate on both, to have everything
		for (auto counterIterator = counters.begin(); counterIterator != counters.end(); counterIterator++) {
			if (instances.size() == 0) {
				string counterAsStr;
				counterAsStr +=  string("\\") + object;
				counterAsStr +=  string("\\") + (*counterIterator);
				cout << counterAsStr << endl;
			} else for (auto instanceIterator = instances.begin(); instanceIterator != instances.end(); instanceIterator++) {
				string counterAsStr;
				counterAsStr +=  string("\\") + object;
				counterAsStr +=  string("(") + (*instanceIterator) + string(")");
				counterAsStr +=  string("\\") + (*counterIterator);
				cout << counterAsStr << endl;
			}
		}
	}

	return 0;
}

