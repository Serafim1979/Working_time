#include <windows.h>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>

// Глобальные переменные
HWND hwndStart, hwndEnd, hwndAddButton, hwndList;
std::vector<std::string> workLog;
const char* logFileName = "worklog.txt";

// Функция для добавления рабочего времени в список
void addWorkTime() {
    // Получение времени начала и окончания работы из полей ввода
    char startBuffer[256], endBuffer[256];
    GetWindowTextA(hwndStart, startBuffer, 256);
    GetWindowTextA(hwndEnd, endBuffer, 256);

    std::string startTime = startBuffer;
    std::string endTime = endBuffer;

    // Вычисление отработанных часов
    int startHour, startMinute, endHour, endMinute;
    sscanf(startTime.c_str(), "%d:%d", &startHour, &startMinute);
    sscanf(endTime.c_str(), "%d:%d", &endHour, &endMinute);

    double totalHours = (endHour * 60 + endMinute - (startHour * 60 + startMinute)) / 60.0;

    // Добавление информации в журнал работы
    std::stringstream logEntry;
    logEntry << "Начало: " << startTime << ", Окончание: " << endTime << ", Отработано: " << totalHours << " ч.";
    workLog.push_back(logEntry.str());

    // Обновление списка в окне
    SendMessageA(hwndList, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(logEntry.str().c_str()));

    // Сохранение в файл
    std::ofstream outFile(logFileName, std::ios::app);
    outFile << logEntry.str() << std::endl;
    outFile.close();
}

// Функция для загрузки данных из файла
void loadWorkLog() {
    std::ifstream inFile(logFileName);
    if (inFile.is_open()) {
        std::string line;
        while (std::getline(inFile, line)) {
            workLog.push_back(line);
            SendMessageA(hwndList, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(line.c_str()));
        }
        inFile.close();
    }
}

// Обработчик сообщений окна
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            // Создание элементов управления (статических текстов, полей ввода, кнопки и списка)
            hwndStart = CreateWindowA("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_CENTER,
                                      50, 50, 100, 25, hwnd, NULL, NULL, NULL);
            hwndEnd = CreateWindowA("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_CENTER,
                                    200, 50, 100, 25, hwnd, NULL, NULL, NULL);
            hwndAddButton = CreateWindowA("BUTTON", "Добавить", WS_VISIBLE | WS_CHILD,
                                          350, 50, 100, 25, hwnd, reinterpret_cast<HMENU>(1), NULL, NULL);
            hwndList = CreateWindowA("LISTBOX", "", WS_VISIBLE | WS_CHILD | WS_BORDER | LBS_STANDARD,
                                     50, 100, 400, 300, hwnd, NULL, NULL, NULL);
            loadWorkLog();
            break;

        case WM_COMMAND:
            if (LOWORD(wParam) == 1 && HIWORD(wParam) == BN_CLICKED) {
                // Обработка нажатия кнопки "Добавить"
                addWorkTime();
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
                               CW_USEDEFAULT, CW_USEDEFAULT, 500, 500, NULL, NULL, hInstance, NULL);

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
