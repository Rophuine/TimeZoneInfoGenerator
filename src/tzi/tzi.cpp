// tzi.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <windows.h>
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <stdio.h>
#include <conio.h>

using namespace std;

template <class T>
T ReadNumber() {
	T result = 0;
	while (true) {
		string input = "";
		getline(cin, input);
		stringstream inputStream(input);
		if (inputStream >> result)
			break;
		cout << "Invalid value, please try again." << endl;
	}
	return result;
}

typedef struct _REG_TZI_FORMAT
{
            long Bias;
            long StandardBias;
            long DaylightBias;
            SYSTEMTIME StandardDate;
            SYSTEMTIME DaylightDate;
} REG_TZI_FORMAT;

int _tmain(int argc, _TCHAR* argv[])
{
	REG_TZI_FORMAT TZI;	// This is the final structure we're going to write out
    
	// Create a pointer for byte-addressing the structure and initialize it all to 0 (unnecessary?)
	unsigned char *bytesTZI= (unsigned char *)&TZI;
    for (int i = 0; i < sizeof(TZI); i++) {
		bytesTZI[i] = 0;
    }

	cout << "Generating a custom timezone registry file." << endl;
	cout << "PLEASE be careful with this tool!" << endl << "It was knocked together during an emergency and is almost completely untested." << endl;
	cout << "You should review the generated file carefully before installing it." << endl << endl;

	cout << "Enter a display name for the timezone. Localization is not supported!" << endl;
	string display = "";
	getline(cin, display);

	cout << "Enter a description for the timezone during normal (non-DST) time." << endl;
	string standardDisplay = "";
	getline(cin, standardDisplay);

	cout << "Enter a description for the timezone during DST (empty to disable DST)." << endl;
	string daylightDisplay = "";
	getline(cin, daylightDisplay);

	bool enableDST = daylightDisplay != "";

	cout << "IMPORTANT: Enter timezone offsets using the standard value" << endl << "i.e. the amount to ADD to UTC to get local." << endl;
	cout << "Enter the timezone offset in MINUTES eg +10 would be 600, -5 would be -300." << endl;
	
	long baseOffset = ReadNumber<long>();
	TZI.Bias = -baseOffset;

	// Unless you're doing something weird, this should be set to 0.
	// Technically it's the offset to be added to the base offset during non-DST time.
	long standardOffset = 0;
	TZI.StandardBias = standardOffset;

	if (enableDST) {
		cout << "Enter the full DST offset in minutes (not the difference)." << endl;
		cout << "e.g. if your base timezone is +10, so you entered 600 above..." << endl << "you might enter 660 here (for +11)." << endl;
		long daylightFullZone = ReadNumber<long>();
		// Windows wants the DST value stored as the value to ADD to the base timezone offset from UTC to get the full DST offset
		TZI.DaylightBias = (-daylightFullZone) - TZI.Bias;

		cout << "Daylight normally starts and ends based on the nth (day of week) of a month." << endl << "e.g. the 1st Sunday in April." << endl;
		cout << "Which day of the week does DST START on? 0=Sunday, 1=Monday, .. 6=Saturday" << endl;
		short startDOW = ReadNumber<short>();
		TZI.DaylightDate.wDayOfWeek = startDOW;

		string days[] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
		cout << "Which " << days[startDOW] << " of the month does DST START on?" << endl;
		short startDOWOfMonth = ReadNumber<short>();
		TZI.DaylightDate.wDay = startDOWOfMonth;

		cout << "Which month does DST START in? 1=Jan, 2=Feb, ... 12=Dec" << endl;
		short startMonth = ReadNumber<short>();
		TZI.DaylightDate.wMonth = startMonth;

		cout << "What time does DST START at? 24-hour format hh:mm e.g. 02:00" << endl;
		string startTime = "";
		getline(cin, startTime);
		// Replace colon with space and read space-delimited tokens using stringstream
		for (int i = 0; i < startTime.length(); i++) if (startTime[i] == ':') startTime[i] = ' ';
		stringstream startTimeStream(startTime);
		int starthour, startminute;
		startTimeStream >> starthour >> startminute;
		TZI.DaylightDate.wHour = starthour;
		TZI.DaylightDate.wMinute = startminute;

		cout << "Which day of the week does DST END on? 0=Sunday, 1=Monday, .. 6=Saturday" << endl;
		short endDOW = ReadNumber<short>();
		TZI.StandardDate.wDayOfWeek = endDOW;

		cout << "Which " << days[endDOW] << " of the month does DST END on?" << endl;
		short endDOWOfMonth = ReadNumber<short>();
		TZI.StandardDate.wDay = endDOWOfMonth;

		cout << "Which month does DST END in? 1=Jan, 2=Feb, ... 12=Dec" << endl;
		short endMonth = ReadNumber<short>();
		TZI.StandardDate.wMonth = endMonth;

		cout << "What time does DST END at? 24-hour format hh:mm e.g. 02:00" << endl;
		string endTime = "";
		getline(cin, endTime);
		for (int i = 0; i < endTime.length(); i++) if (endTime[i] == ':') endTime[i] = ' ';
		stringstream endTimeStream(endTime);
		int endhour, endminute;
		endTimeStream >> endhour >> endminute;
		TZI.StandardDate.wHour = endhour;
		TZI.StandardDate.wMinute = endminute;
	}

	string fn = "";
	cout << "Enter filename to write to (blank for customTZ.reg):" << endl;
	getline(cin, fn);
	if (fn == "") fn = "customTZ.reg";

	ofstream outputFile;
	cout << "Writing to " << fn << endl;
	outputFile.open(fn);

	if (outputFile.is_open()) {
		outputFile << "Windows Registry Editor Version 5.00" << endl << endl;
		outputFile << "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Time Zones\\" << display << "]" << endl;
		outputFile << "\"MUI_Display\"=\"" << display << "\"" << endl;
		outputFile << "\"MUI_Dlt\"=\"" << daylightDisplay << "\"" << endl;
		outputFile << "\"MUI_Std\"=\"" << standardDisplay << "\"" << endl;
		outputFile << "\"Display\"=\"" << display << "\"" << endl;
		outputFile << "\"Dlt\"=\"" << daylightDisplay << "\"" << endl;
		outputFile << "\"Std\"=\"" << standardDisplay << "\"" << endl;
		outputFile << "\"TZI\"=hex:";

		for (int i = 0; i < sizeof(TZI); i++) {
			switch (i) {
			case 0: break;
			case 23: outputFile << ",\\" << endl << "  "; break;
			default: outputFile << ","; break;
			}
			int value = bytesTZI[i];
			outputFile << setfill('0') << setw(2) << hex << value;
		}
		outputFile << endl;
		outputFile.close();
	}
	else cout << "Unable to open file." << endl;

	return 0;
}

