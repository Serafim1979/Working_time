#include <windows.h>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <ctime>

// Глобальные переменные
HWND hwndYear, hwndMonth, hwndUpdateButton;
std::vector<HWND> hwndDays, hwndBeginTimes, hwndEndTimes, hwndWorkHours, hwndOvertimes;
const int maxDaysInMonth = 31;
const double normalWorkHours = 8.0;
const double lunchBreakMinutes = 45.0;
const char* logFileName = "worklog.txt";

// Функция для вычисления отработанных часов с учетом обеденного времени
double calculateHours(const std::string& startTime, const std::string& endTime) {
    int startHour = 0, startMinute = 0, endHour = 0, endMinute = 0;
    sscanf(startTime.c_str(), "%d:%d", &startHour, &startMinute);
    sscanf(endTime.c_str(), "%d:%d", &endHour, &endMinute);

    double startTotalMinutes = startHour * 60 + startMinute;
    double endTotalMinutes = endHour * 60 + endMinute;
    double totalWorkedMinutes = endTotalMinutes - startTotalMinutes;

    totalWorkedMinutes -= lunchBreakMinutes;

    return totalWorkedMinutes / 60.0;
}

// Функция для вычисления сверхурочных часов
double calculateOvertime(double totalHours) {
    if (totalHours > normalWorkHours) {
        return totalHours - normalWorkHours;
    }
    return 0.0;
}

// Функция для получения количества дней в месяце
int getDaysInMonth(int month, int year) {
    if (month == 2) {
        // Проверка на високосный год
        if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) {
            return 29;
        }
        return 28;
    }
    if (month == 4 || month == 6 || month == 9 || month == 11) {
        return 30;
    }
    return 31;
}

// Функция для обновления интерфейса в зависимости от месяца и года
void updateInterface(HWND hwnd) {
    char yearBuffer[5], monthBuffer[3];
    GetWindowTextA(hwndYear, yearBuffer, 5);
    GetWindowTextA(hwndMonth, monthBuffer, 3);

    int year = atoi(yearBuffer);
    int month = atoi(monthBuffer);

    int daysInMonth = getDaysInMonth(month, year);

    // Скрытие всех дней
    for (int i = 0; i < maxDaysInMonth; ++i) {
        ShowWindow(hwndDays[i], SW_HIDE);
        ShowWindow(hwndBeginTimes[i], SW_HIDE);
        ShowWindow(hwndEndTimes[i], SW_HIDE);
        ShowWindow(hwndWorkHours[i], SW_HIDE);
        ShowWindow(hwndOvertimes[i], SW_HIDE);
    }

    // Отображение необходимых дней
    for (int i = 0; i < daysInMonth; ++i) {
        char dayText[11];
        sprintf(dayText, "%02d/%02d/%04d", i + 1, month, year);
        SetWindowTextA(hwndDays[i], dayText);
        ShowWindow(hwndDays[i], SW_SHOW);
        ShowWindow(hwndBeginTimes[i], SW_SHOW);
        ShowWindow(hwndEndTimes[i], SW_SHOW);
        ShowWindow(hwndWorkHours[i], SW_SHOW);
        ShowWindow(hwndOvertimes[i], SW_SHOW);
    }
}

// Функция для обработки ввода времени
void processTimeInput() {
    char beginBuffer[6], endBuffer[6];
    for (int i = 0; i < hwndBeginTimes.size(); ++i) {
        GetWindowTextA(hwndBeginTimes[i], beginBuffer, 6);
        GetWindowTextA(hwndEndTimes[i], endBuffer, 6);

        std::string beginTime = beginBuffer;
        std::string endTime = endBuffer;

        if (!beginTime.empty() && !endTime.empty()) {
            // Проверка и исправление формата времени
            if (beginTime.find(':') == std::string::npos) {
                if (beginTime.length() == 1 || beginTime.length() == 2) {
                    beginTime += ":00";
                } else if (beginTime.length() == 3) {
                    beginTime.insert(1, ":");
                } else if (beginTime.length() == 4) {
                    beginTime.insert(2, ":");
                }
            }
            if (endTime.find(':') == std::string::npos) {
                if (endTime.length() == 1 || endTime.length() == 2) {
                    endTime += ":00";
                } else if (endTime.length() == 3) {
                    endTime.insert(1, ":");
                } else if (endTime.length() == 4) {
                    endTime.insert(2, ":");
                }
            }

            double workedHours = calculateHours(beginTime, endTime);
            double overtimeHours = calculateOvertime(workedHours);

            char workedHoursBuffer[10], overtimeHoursBuffer[10];
            sprintf(workedHoursBuffer, "%.2f ч.", workedHours);
            sprintf(overtimeHoursBuffer, "%.2f ч.", overtimeHours);

            SetWindowTextA(hwndWorkHours[i], workedHoursBuffer);
            SetWindowTextA(hwndOvertimes[i], overtimeHoursBuffer);
        }
    }
}

// Обработчик сообщений окна
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            // Создание полей для ввода года и месяца
            hwndYear = CreateWindowA("EDIT", "2024", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_CENTER,
                                     50, 20, 50, 25, hwnd, NULL, NULL, NULL);
            hwndMonth = CreateWindowA("EDIT", "6", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_CENTER,
                                      110, 20, 30, 25, hwnd, NULL, NULL, NULL);
            hwndUpdateButton = CreateWindowA("BUTTON", "Обновить", WS_VISIBLE | WS_CHILD,
                                             150, 20, 100, 25, hwnd, reinterpret_cast<HMENU>(1), NULL, NULL);

            for (int i = 0; i < maxDaysInMonth; ++i) {
                char dayText[11];
                sprintf(dayText, "%02d/06/2024", i + 1); // Примерный формат дня

                hwndDays.push_back(CreateWindowA("STATIC", dayText, WS_CHILD,
                                                 50, 60 + i * 30, 100, 25, hwnd, NULL, NULL, NULL));
                hwndBeginTimes.push_back(CreateWindowA("EDIT", "", WS_CHILD | WS_BORDER | ES_CENTER,
                                                       160, 60 + i * 30, 50, 25, hwnd, NULL, NULL, NULL));
                hwndEndTimes.push_back(CreateWindowA("EDIT", "", WS_CHILD | WS_BORDER | ES_CENTER,
                                                     220, 60 + i * 30, 50, 25, hwnd, NULL, NULL, NULL));
                hwndWorkHours.push_back(CreateWindowA("STATIC", "", WS_CHILD,
                                                      280, 60 + i * 30, 70, 25, hwnd, NULL, NULL, NULL));
                hwndOvertimes.push_back(CreateWindowA("STATIC", "", WS_CHILD,
                                                      360, 60 + i * 30, 70, 25, hwnd, NULL, NULL, NULL));
            }

            updateInterface(hwnd);
            break;

        case WM_COMMAND:
            if (LOWORD(wParam) == 1 && HIWORD(wParam) == BN_CLICKED) {
                // Обработка нажатия кнопки "Обновить"
                updateInterface(hwnd);
            } else {
                processTimeInput();
            }
            break;

        case WM_DESTROY:
            // Завершение приложения
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

// Функция создания и запуска оконного приложения
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Регистрация класса окна
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "WorkTimeApp";

    RegisterClass(&wc);

    // Создание окна
    HWND hwnd = CreateWindowEx(0, "WorkTimeApp", "Учет рабочего времени", WS_OVERLAPPEDWINDOW,
                               CW_USEDEFAULT, CW_USEDEFAULT, 500, 1000, NULL, NULL, hInstance, NULL);

    if (hwnd == NULL) {
        return 0;
    }

    // Отображение окна
    ShowWindow(hwnd, nCmdShow);

    // Цикл обработки сообщений
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return static_cast<int>(msg.wParam);
}
