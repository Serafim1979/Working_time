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
HWND g_hTotalOvertime = nullptr;

// Function prototypes
std::string GetDayOfWeekAbbreviation(int year, int month, int day);
void UpdateTotalOverTime(HWND hwnd);
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
    const char CLASS_NAME[] = "WindowClass";
    WNDCLASS wc = {};

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        "Time Logger",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT, 980, 730,
        NULL, NULL, hInstance, NULL
    );

    if(hwnd == NULL)
    {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};
    while(GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
    default:
        {
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
    }
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

void UpdateTotalOverTime(HWND hwnd)
{
    int totalOvertimeMinutes = 0;
    for(int day = 1; day <= MAX_DAYS_IN_MONTH; ++day)
    {
        char buffer[10];
        GetWindowText(GetDlgItem(hwnd, IDC_OVERTIME(day)), buffer, 10);

        int overtimeHours = 0, overtimeMinutes = 0;
        sscanf(buffer, "%dh %dm", &overtimeHours, &overtimeMinutes);
        totalOvertimeMinutes += overtimeHours * 60 + overtimeMinutes;
    }

    int totalOvertimeHours = totalOvertimeMinutes / 60;
    int totalOvertimeRemainingMinutes = totalOvertimeMinutes % 60;

    char totalOvertimeResult[50];
    sprintf(totalOvertimeResult, "Total Overtime: %dh %dm", totalOvertimeHours, totalOvertimeRemainingMinutes);
    SetWindowText(g_hTotalOvertime, totalOvertimeResult);
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

    int launchBreakMinutes = 45;

    GetWindowText(GetDlgItem(hwnd, IDC_START_HOURS(day)), buffer, 5);
    startHours = atoi(buffer);

    GetWindowText(GetDlgItem(hwnd, IDC_START_MINUTES(day)), buffer, 5);
    startMinutes = atoi(buffer);

    GetWindowText(GetDlgItem(hwnd, IDC_END_HOURS(day)), buffer, 5);
    endHours = atoi(buffer);

    GetWindowText(GetDlgItem(hwnd, IDC_END_MINUTES(day)), buffer, 5);
    endMinutes = atoi(buffer);

    // Checking the correctness of the input
    if (startHours < 0 || startHours > 23 || startMinutes < 0 || startMinutes > 59 ||
        endHours < 0 || endHours > 23 || endMinutes < 0 || endMinutes > 59)
    {
        SetWindowText(GetDlgItem(hwnd, IDC_RESULT(day)), "Invalid Input");
        return;
    }



    // calculate the duration for the current day
    int totalStartMinutes = startHours * 60 + startMinutes;
    int totalEndMinutes = endHours * 60 + endMinutes;
    totalEndMinutes -= launchBreakMinutes;

    // Calculate duration
    int durationMinutes = totalEndMinutes - totalStartMinutes;

    // Handle crossing midnight
    if (durationMinutes < 0) {
        durationMinutes += 24 * 60;
    }

    // Get total hours and minutes from duration
    int totalHours = durationMinutes / 60;
    int totalMinutes = durationMinutes % 60;

    char result[50];
    sprintf(result, "%dh %dm", totalHours, totalMinutes);
    SetWindowText(GetDlgItem(hwnd, IDC_RESULT(day)), result);

    // Calculate and display overtime
    SYSTEMTIME systemTime;
    GetLocalTime(&systemTime);

    int current_year = systemTime.wYear;
    int current_month = systemTime.wMonth;
    int current_day = systemTime.wDay;

    std::string dayOfWeek = GetDayOfWeekAbbreviation(current_year, current_month, day);
    int normalWorkMinutes = 0;

    if(dayOfWeek == "Mon" || dayOfWeek == "Tue" || dayOfWeek == "Wed" || dayOfWeek == "Thu")
    {
        normalWorkMinutes = 8 * 60 + 15;
    }
    else if(dayOfWeek == "Fri")
    {
        normalWorkMinutes = 7 * 60;
    }
    else
    {
        normalWorkMinutes = 8 * 60;
    }

    int overtimeMinutes = durationMinutes > normalWorkMinutes ? durationMinutes - normalWorkMinutes : 0;
    int overtimeHours = overtimeMinutes / 60;
    int overtimeRemainingMinutes = overtimeMinutes % 60;

    char overtimeResult[50];
    sprintf(overtimeResult, "%dh %dm", overtimeHours, overtimeRemainingMinutes);
    SetWindowText(GetDlgItem(hwnd, IDC_OVERTIME(day)), overtimeResult);

    UpdateTotalOverTime(hwnd);
}

void CreateDayControls(HWND hwnd, int day, int x, int y)
{
    CreateWindow("STATIC", "Start", WS_VISIBLE | WS_CHILD | SS_LEFT, x, y, 40, 20, hwnd, NULL, NULL, NULL);
    CreateWindow("STATIC", "hh:", WS_VISIBLE | WS_CHILD | SS_LEFT, x + 50, y, 20, 20, hwnd, NULL, NULL, NULL);
    CreateWindow("STATIC", "mm:", WS_VISIBLE | WS_CHILD | SS_LEFT, x + 120, y, 30, 20, hwnd, NULL, NULL, NULL);

    g_hEditHHStart[day - 1] = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER, x + 75, y, 30, 20, hwnd, (HMENU)IDC_START_HOURS(day), NULL, NULL);
    g_hEditMMStart[day - 1] = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER, x + 155, y, 30, 20, hwnd, (HMENU)IDC_START_MINUTES(day), NULL, NULL);

    CreateWindow("STATIC", "End", WS_VISIBLE | WS_CHILD | SS_LEFT, x + 220, y, 40, 20, hwnd, NULL, NULL, NULL);
    CreateWindow("STATIC", "hh:", WS_VISIBLE | WS_CHILD | SS_LEFT, x + 265, y, 20, 20, hwnd, NULL, NULL, NULL);
    CreateWindow("STATIC", "mm:", WS_VISIBLE | WS_CHILD | SS_LEFT, x + 340, y, 30, 20, hwnd, NULL, NULL, NULL);

    g_hEditHHEnd[day - 1] = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER, x + 290, y, 30, 20, hwnd, (HMENU)IDC_END_HOURS(day), NULL, NULL);
    g_hEditMMEnd[day - 1] = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER, x + 375, y, 30, 20, hwnd, (HMENU)IDC_END_MINUTES(day), NULL, NULL);

    CreateWindow("STATIC", "Hours Worked:", WS_VISIBLE | WS_CHILD | SS_LEFT, x + 430, y, 100, 20, hwnd, NULL, NULL, NULL);
    g_hEditHoursWorked[day - 1] = CreateWindow("STATIC", "", WS_VISIBLE | WS_CHILD | SS_LEFT, x + 535, y, 60, 20, hwnd, (HMENU)IDC_RESULT(day), NULL, NULL);

    CreateWindow("STATIC", "Overtime", WS_VISIBLE | WS_CHILD | SS_LEFT, x + 630, y, 110, 20, hwnd, NULL, NULL, NULL);
    g_hEditHoursWorked[day - 1] = CreateWindow("STATIC", "", WS_VISIBLE | WS_CHILD | SS_LEFT, x + 750, y, 60, 20, hwnd, (HMENU)IDC_RESULT(day), NULL, NULL);
}
