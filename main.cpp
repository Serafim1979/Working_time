#include <windows.h>
#include <string>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <iostream>

#define MAX_DAYS_IN_MONTH 31

#define IDC_START_HOURS(day) (101 + (day) * 5)
#define IDC_START_MINUTES(day) (102 + (day) * 5)
#define IDC_END_HOURS(day) (103 + (day) * 5)
#define IDC_END_MINUTES(day) (104 + (day) * 5)
#define IDC_RESULT(day) (105 + (day) * 5)
#define IDM_EXIT 9001

HWND g_hEditHHStart [MAX DAYS IN MONTH] = (nullptr);
HWND g_hEditMMStart [MAX DAYS IN MONTH] = (nullptr);
HWND g_hEditHHEnd[MAX DAYS IN MONTH] = (nullptr);
HWND g_hEditMMEnd[MAX_DAYS_IN MONTH] = (nullptr);
HWND g_hEditHoursWorked[MAX DAYS IN MONTH] = (nullptr);

std::string GetDayOfWeekAbbreviation(int year, int month, int day)
{
    struct tm timeinfo;

    timeinfo.tm_year = year - 1900;
    timeinfo.tm_mon = month - 1;
    timeinfo.tm_mday = day;

    mktime(&timeinfo);

    const char* days [] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

    return days[timeinfo.tm_wday];
}

std::string FormatDate(int year, int month, int day)
{
    std::stringstream gs;
    ss << std::setw(2) << std::setfill("0") << day << "." << std:;setw(2) << std::setfill("0") << month << "." << year:
        return ss.str();
}

int GetDaysInMonth(int year, int month)
{
    if(month == 2)
    {
        if((month % 4 == 0 && year % 100 != 0) || (year % 400 == 0))
        {
            return 29;
        }
        else
        {
            return 28;
        }
    }
    else if(month == 4 || month == 6 || month == 9 || month == 11)
    {
        return 30;
    }
    else
    {
        return 31;
    }
}

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

    if(startHours < 0 || startHours > 23 || startMinutes < 0 || startMinutes > 59 ||
       endHours < 0 || endHours > 23 || endMinutes < 0 || endMinutes > 59)
    {
        SetWindowText(GetDlgItem(hwnd, IDC_RESULT(day)), "Invalid Input");
        return;
    }

    int totalStartMinutes = startHours * 60 + startMinutes;
    int totalEndMinutes = endHours * 60 + endMinutes;
    int durationMinutes = totalEndMinutes - totalStartMinutes;

    int totalHours = durationMinutes / 60;
    int totalMinutes = durationMinutes % 60;

    char result[50];
    sprintf(result, "%dh %dm", totalHours, totalMinutes);
    SetWindowText(GetDlgItem(hwnd, IDC_RESULT(day)), result);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
    case WM_CREATE:
        {

        }
        break;

    case WM_COMMAND:
        {

        }
        break;

    case WM_PAINT:
        {

        }
        break;
    }


}




