#include <windows.h>
#include <string>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <fstream>

#define MAX_DAYS_IN_MONTH 31

#define IDC_START_HOURS(day)(101 + (day) * 6)
#define IDC_START_MINUTES(day)(102 + (day) * 6)
#define IDC_END_HOURS(day)(103 + (day) * 6)
#define IDC_END_MINUTES(day)(104 + (day) * 6)
#define IDC_RESULT(day)(105 + (day) * 6)
#define IDC_OVERTIME(day)(106 + (day) * 6)

#define IDC_SAVE_BUTTON 1070

#define IDC_TOTAL_OVERTIME 1080

#define IDM_EXIT 9001

HWND g_hEditHHStart[MAX_DAYS_IN_MONTH] = {nullptr};
HWND g_hEditMMStart[MAX_DAYS_IN_MONTH] = {nullptr};
HWND g_hEditHHEnd[MAX_DAYS_IN_MONTH] = {nullptr};
HWND g_hEditMMEnd[MAX_DAYS_IN_MONTH] = {nullptr};
HWND g_hEditHoursWorked[MAX_DAYS_IN_MONTH] = {nullptr};
HWND g_hEditOvertime[MAX_DAYS_IN_MONTH] = {nullptr};
HWND g_hTotalOvertime = nullptr;

std::string GetDayOfWeekAbbreviation(int year, int month, int day)
{
    struct tm timeinfo = {0};
    timeinfo.tm_year = year - 1900;
    timeinfo.tm_mon = month - 1;
    timeinfo.tm_mday = day;
    mktime(&timeinfo);

    const char* days[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    return days[timeinfo.tm_wday];
}

void UpdateTotalOvertime(HWND hwnd)
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

std::string FormatDate(int year, int month, int day)
{
    std::stringstream ss;
    ss << std::setw(2) << std::setfill('0') << day << "." << std::setw(2) << std::setfill('0') << month << "." << year;
    return ss.str();
}

void CleanupEditControls()
{
    for(int i = 0; i < MAX_DAYS_IN_MONTH; ++i)
    {
        if(g_hEditHHStart[i])
            DestroyWindow(g_hEditHHStart[i]);
        if(g_hEditMMStart[i])
            DestroyWindow(g_hEditMMStart[i]);
        if(g_hEditHHEnd[i])
            DestroyWindow(g_hEditHHEnd[i]);
        if(g_hEditMMEnd[i])
            DestroyWindow(g_hEditMMEnd[i]);
        if(g_hEditHoursWorked[i])
            DestroyWindow(g_hEditHoursWorked[i]);
        if(g_hEditOvertime[i])
            DestroyWindow(g_hEditOvertime[i]);
    }
}

int GetDaysInMonth(int year, int month)
{
    if(month == 2)
    {
        if((month % 4 == 0 && year % 100 != 0) || (year % 400 == 0))
            return 29;
        else
            return 28;
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
    int lauchBreakMinutes = 45;

    GetWindowText(GetDlgItem(hwnd, IDC_START_HOURS(day)), buffer, 5);
    startHours = atoi(buffer);

    GetWindowText(GetDlgItem(hwnd, IDC_START_MINUTES(day)), buffer, 5);
    startMinutes = atoi(buffer);

    GetWindowText(GetDlgItem(hwnd, IDC_END_HOURS(day)), buffer, 5);
    endHours = atoi(buffer);

    GetWindowText(GetDlgItem(hwnd, IDC_END_MINUTES(day)), buffer, 5);
    endMinutes = atoi(buffer);


    // Проверка корректности ввода
    if(startHours < 0 || startHours > 23 || startMinutes < 0 || startMinutes > 59 ||
       endHours < 0 || endHours > 23 || endMinutes < 0 || endMinutes > 59)
    {
        SetWindowText(GetDlgItem(hwnd, IDC_RESULT(day)), "Invalid Input");
        return;
    }

    // Вычисляем продолжительность для текущего дня
    int totalStartMinutes = startHours * 60 + startMinutes;
    int totalEndMinutes = endHours * 60 + endMinutes;

    totalEndMinutes -= lauchBreakMinutes;

    int durationMinutes = totalEndMinutes - totalStartMinutes;

    if(durationMinutes < 0)
    {
        durationMinutes += 24 * 60;
    }

    int totalHours = durationMinutes / 60;
    int totalMinutes = durationMinutes % 60;

    char result[50];
    sprintf(result, "%dh %dm", totalHours, totalMinutes);
    SetWindowText(GetDlgItem(hwnd, IDC_RESULT(day)), result);


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

    UpdateTotalOvertime(hwnd);
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
    g_hEditOvertime[day - 1] = CreateWindow("STATIC", "", WS_VISIBLE | WS_CHILD | SS_LEFT, x + 750, y, 60, 20, hwnd, (HMENU)IDC_OVERTIME(day), NULL, NULL);
}

void SaveDataToFile(HWND hwnd)
{
    SYSTEMTIME systemTime;
    GetLocalTime(&systemTime);
    int current_year = systemTime.wYear;
    int current_month = systemTime.wMonth;

    std::ofstream file("timelog.txt");
    if(file.is_open())
    {
        for(int day = 1; day <= MAX_DAYS_IN_MONTH; ++day)
        {
            char buffer[10];

            GetWindowText(GetDlgItem(hwnd, IDC_START_HOURS(day)), buffer, 10);
            int startHours = atoi(buffer);

            GetWindowText(GetDlgItem(hwnd, IDC_START_MINUTES(day)), buffer, 10);
            int startMinutes = atoi(buffer);

            GetWindowText(GetDlgItem(hwnd, IDC_END_HOURS(day)), buffer, 10);
            int endHours = atoi(buffer);
            GetWindowText(GetDlgItem(hwnd, IDC_END_MINUTES(day)), buffer, 10);
            int endMinutes = atoi(buffer);

            char resultBuffer[50];
            GetWindowText(GetDlgItem(hwnd, IDC_RESULT(day)), resultBuffer, 50);
            std::string result = resultBuffer;

            char overtimeBuffer[50];
            GetWindowText(GetDlgItem(hwnd, IDC_OVERTIME(day)), overtimeBuffer, 50);
            std::string overtime = overtimeBuffer;

            if(startHours != 0 || startMinutes != 0 || endHours != 0 || endMinutes != 0 || !result.empty() || !overtime.empty())
            {
                std::string date = FormatDate(current_year, current_month, day);

                file << date << " Start: " << std::setw(2) << std::setfill('0') << startHours << ":"
                     << std::setw(2) << std::setfill('0') << startMinutes << ", End: " << std::setw(2) << std::setfill('0') << endHours << ":"
                     << std::setw(2) << std::setfill('0') << endMinutes
                     << ", Work time: " << result
                     << ", Overtime: " << overtime << "\n";
            }
        }
        file.close();
    }
}

void LoadDateFromFile(HWND hwnd)
{
    std::ifstream file("timelog.txt");
    if(file.is_open())
    {
        std::string line;

        while(std::getline(file, line))
        {
            std::istringstream iss(line);
            std::string date, startStr, endStr, workTimeStr, overtimeStr;
            int startHours, startMinutes, endHours, endMinutes;
            char separator, comma;

            iss >> date >> startStr >> startHours >> separator >> startMinutes >> comma >> endStr >> endHours >> separator >> endMinutes >> comma >> workTimeStr;

            std::getline(iss, workTimeStr, ',');
            std::getline(iss, overtimeStr);

            workTimeStr = workTimeStr.substr(7);
            overtimeStr = overtimeStr.substr(11);

            // Find the day int the date string
            int day = std::stoi(date.substr(0, 2));

            SetWindowText(GetDlgItem(hwnd, IDC_START_HOURS(day)), std::to_string(startHours).c_str());
            SetWindowText(GetDlgItem(hwnd, IDC_START_MINUTES(day)), std::to_string(startMinutes).c_str());
            SetWindowText(GetDlgItem(hwnd, IDC_END_HOURS(day)), std::to_string(endHours).c_str());
            SetWindowText(GetDlgItem(hwnd, IDC_END_MINUTES(day)), std::to_string(endMinutes).c_str());
            SetWindowText(GetDlgItem(hwnd, IDC_RESULT(day)), workTimeStr.c_str());
            SetWindowText(GetDlgItem(hwnd, IDC_OVERTIME(day)), overtimeStr.c_str());
        }
        UpdateTotalOvertime(hwnd);
        file.close();
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
    case WM_CREATE:
        {
            SYSTEMTIME systemTime;
            GetLocalTime(&systemTime);
            int current_year = systemTime.wYear;
            int curren_month = systemTime.wMonth;
            int days_in_month = GetDaysInMonth(current_year, curren_month);

            for(int day = 1; day <= days_in_month; ++day)
            {
                int x = 160;
                int y = 20 + (day - 1) * 20;

                CreateDayControls(hwnd, day, x, y);
            }

            CreateWindow("BUTTON", "Save to File", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 20, 650, 100, 30, hwnd, (HMENU)IDC_SAVE_BUTTON, NULL, NULL);

            g_hTotalOvertime = CreateWindow("STATIC", "", WS_VISIBLE | WS_CHILD | SS_LEFT, 800, 660, 160, 20, hwnd, (HMENU)IDC_TOTAL_OVERTIME, NULL, NULL);

            LoadDateFromFile(hwnd);
        }
        break;

    case WM_COMMAND:
        {
            if(LOWORD(wParam) == IDC_SAVE_BUTTON)
            {
                SaveDataToFile(hwnd);
            }
            else if(HIWORD(wParam) == EN_CHANGE)
            {
                int id = LOWORD(wParam);
                int day = (id - 101) / 6;
                CalculateDurationForDay(hwnd, day);
            }
        }
        break;

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            SYSTEMTIME systemTime;
            GetLocalTime(&systemTime);

            int current_year = systemTime.wYear;
            int current_month = systemTime.wMonth;
            int days_in_month = GetDaysInMonth(current_year, current_month);

            for(int day = 1; day <= days_in_month; ++day)
            {
                int x = 20;
                int y = 20 + (day - 1) * 20;

                std::string dayOfWeek = GetDayOfWeekAbbreviation(current_year, current_month, day);
                std::string dateString = FormatDate(current_year, current_month, day);

                if(dayOfWeek == "Sat" || dayOfWeek == "Sun")
                {
                    SetTextColor(hdc, RGB(255, 0, 0));
                }
                else if(dayOfWeek == "Fri")
                {
                    SetTextColor(hdc, RGB(20, 158, 90));
                }
                else
                {
                    SetTextColor(hdc, RGB(0, 0, 0));
                }

                TextOutA(hdc, x, y, dayOfWeek.c_str(), dayOfWeek.length());
                TextOutA(hdc, x + 35, y, dateString.c_str(), dateString.length());
            }

            EndPaint(hwnd, &ps);
        }
        break;

    case WM_DESTROY:
        {
            CleanupEditControls();
            PostQuitMessage(0);
        }
        break;

    default:
        {
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
        break;
    }
    return 0;
}

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
