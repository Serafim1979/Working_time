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
void CleanupEditControls()
{
    for (int i = 0; i < MAX_DAYS_IN_MONTH; ++i)
    {
        if (g_hEditHHStart[i])
            DestroyWindow(g_hEditHHStart[i]);
        if (g_hEditMMStart[i])
            DestroyWindow(g_hEditMMStart[i]);
        if (g_hEditHHEnd[i])
            DestroyWindow(g_hEditHHEnd[i]);
        if (g_hEditMMEnd[i])
            DestroyWindow(g_hEditMMEnd[i]);
        if (g_hEditHoursWorked[i])
            DestroyWindow(g_hEditHoursWorked[i]);
        if (g_hEditOvertime[i])
            DestroyWindow(g_hEditOvertime[i]);
    }
}

// Function for determining the number of days in a month
int GetDaysInMonth(int year, int month)
{
    if (month == 2)
    {
        if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))
        {
            return 29;
        }
        else
        {
            return 28;
        }
    }
    else if (month == 4 || month == 6 || month == 9 || month == 11)
    {
        return 30;
    }
    else
    {
        return 31;
    }
}

// Function for calculating the working day and processing time
void CalculateDurationForDay(HWND hwnd, int day)
{
    char buffer[5];
    int startHours = 0, startMinutes = 0, endHours = 0, endMinutes = 0;

    GetWindowText(GetDlgItem(hwnd, IDC_START_HOURS(day)), buffer, 5);
    startHours = atoi(buffer);

    GetWindowText(GetDlgItem(hwnd, IDC_START_MINUTES(day)), buffer, 5);
    startMinutes = atoi(buffer);

    GetWindowText(GetDlgItem(hwnd, IDC_END_HOURS(day)), buffer, 5);
    endHours = atoi(buffer);

    GetWindowText(GetDlgItem(hwnd, IDC_END_MINUTES(day)), buffer, 5);
    endMinutes = atoi(buffer);

    if (startHours < 0 || startHours > 23 || startMinutes < 0 || startMinutes > 59 ||
        endHours < 0 || endHours > 23 || endMinutes < 0 || endMinutes > 59)
    {
        SetWindowText(GetDlgItem(hwnd, IDC_RESULT(day)), "Invalid Input");
        return;
    }

    using namespace std::chrono;

    // Convert start and end times to time_point objects
    auto startTime = hours(startHours) + minutes(startMinutes);
    auto endTime = hours(endHours) + minutes(endMinutes);

    // Calculate duration
    auto duration = endTime - startTime;

    // Handle crossing midnight
    if (duration < minutes(0)) {
        duration += hours(24);
    }

    // Get total hours and minutes from duration
    auto totalHours = duration_cast<hours>(duration).count();
    auto totalMinutes = duration_cast<minutes>(duration).count() % 60;

    char result[50];
    sprintf(result, "%lldh %lldm", totalHours, totalMinutes);
    SetWindowText(GetDlgItem(hwnd, IDC_RESULT(day)), result);

    // Calculate and display overtime
    int overtimeMinutes = 0;
    if (totalHours >= 8) {
        overtimeMinutes = (totalHours - 8) * 60 + totalMinutes;
    }
    int overtimeHours = overtimeMinutes / 60;
    int overtimeRemainingMinutes = overtimeMinutes % 60;

    char overtimeResult[50];
    sprintf(overtimeResult, "%dh %dm", overtimeHours, overtimeRemainingMinutes);
    SetWindowText(GetDlgItem(hwnd, IDC_OVERTIME(day)), overtimeResult);
}
