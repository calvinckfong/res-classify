/*
 * main.cpp
 *
 *  Created on: 2021年2月20日
 *      Author: calvin.fong
 */

#include "res-classifier.h"
#include "iostream"

using namespace std;

void ShowHelp(const char *progName)
{
	cout << endl;
	cout << "Usage:" << endl;
	cout << "  " << progName << " [filename]" << endl;
	cout << endl;
}


int main(int argc, char* argv[])
{
	if (argc<2)
	{
		cout << "Insufficient arguments." << endl;
		ShowHelp(argv[0]);
		return -1;
	}

	ResClassifier* classifier = new ResClassifier;
	classifier->Classify(argv[1]);

	return 0;
}
