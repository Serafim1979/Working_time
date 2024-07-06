/*********************************************************************
* Description: A program for logging working hours and overwork.
* The user enters the start and end time for each
* the day of the month, the program calculates the working time
* days and recycling.
* Author: Ivan Korolkov
* Date of creation: July 6, 2024
*
* License: MIT License
*
* Input data: Start and end time for each day of the month.
* Output data: Working hours and processing.
*
* Libraries used:
*  - windows.h: for working with WinAPI.
*  - string: for working with strings.
*  - ctime: for working with date and time.
*  - chrono: for working with date and time.
*  - stream: for working with string streams.
*  - iomanip: for I/O formatting.
*  - iostream: for standard I/O.
*  - fstream: for working with files.
*********************************************************************/

#include <windows.h>
#include <string>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <chrono>


// Constants and macros
#define MAX_DAYS_IN_MONTH 31

#define IDC_START_HOURS(day) (101 + (day) * 6)
#define IDC_START_MINUTES(day) (102 + (day) * 6)
#define IDC_END_HOURS(day) (103 + (day) * 6)
#define IDC_END_MINUTES(day) (104 + (day) * 6)
#define IDC_RESULT(day) (105 + (day) * 6)
#define IDC_OVERTIME(day) (106 + (day) * 6)
#define IDC_SAVE_BUTTON 1070
#define IDM_EXIT 9001


// Global variables for storing control descriptors
HWND g_hEditHHStart[MAX_DAYS_IN_MONTH] = {nullptr};
HWND g_hEditMMStart[MAX_DAYS_IN_MONTH] = {nullptr};
HWND g_hEditHHEnd[MAX_DAYS_IN_MONTH] = {nullptr};
HWND g_hEditMMEnd[MAX_DAYS_IN_MONTH] = {nullptr};
HWND g_hEditHoursWorked[MAX_DAYS_IN_MONTH] = {nullptr};
HWND g_hEditOvertime[MAX_DAYS_IN_MONTH] = {nullptr};

// Function prototypes
std::string GetDayOfWeekAbbreviation(int year, int month, int day);
std::string FormatDate(int year, int month, int day);
void CleanupEditControls();
int GetDaysInMonth(int year, int month);
void CalculateDurationForDay(HWND hwnd, int day);
void CreateDayControls(HWND hwnd, int day, int x, int y);
void SaveDataToFile(HWND hwnd);
void LoadDataFromFile(HWND hwnd);
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    return 0;
}

// Returns the shortened day of the week for the specified date
std::string GetDayOfWeekAbbreviation(int year, int month, int day)
{
    struct tm timeinfo = {};
    timeinfo.tm_year = year - 1900;
    timeinfo.tm_mon = month - 1;
    timeinfo.tm_mday = day;
    mktime(&timeinfo);

    const char* days[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    return days[timeinfo.tm_wday];
}

// Formats the date in a string like DD.MM.YYYY
std::string FormatDate(int year, int month, int day)
{
    std::stringstream ss;
    ss << std::setw(2) << std::setfill('0') << day << "." << std::setw(2) << std::setfill('0') << month << "." << year;
    return ss.str();
}

// Deletes all created controls

