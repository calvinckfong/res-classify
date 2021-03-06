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
	cout << "  " << progName << " [method] [filename]" << endl;
	cout << "    method:  0: MSE" << endl;
	cout << "             1: HighPass" << endl;
	cout << endl;
}


int main(int argc, char* argv[])
{
	if (argc<3)
	{
		cout << "Insufficient arguments." << endl;
		ShowHelp(argv[0]);
		return -1;
	}

	int method = atoi(argv[1]);

	ResClassifier* classifier = new ResClassifier(method);
	classifier->Classify(argv[2]);

	return 0;
}
