#include <windows.h>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>

// ���������� ����������
HWND hwndStart, hwndEnd, hwndAddButton, hwndList;
std::vector<std::string> workLog;
const char* logFileName = "worklog.txt";

// ������� ��� ���������� �������� ������� � ������
void addWorkTime() {
    // ��������� ������� ������ � ��������� ������ �� ����� �����
    char startBuffer[256], endBuffer[256];
    GetWindowTextA(hwndStart, startBuffer, 256);
    GetWindowTextA(hwndEnd, endBuffer, 256);

    std::string startTime = startBuffer;
    std::string endTime = endBuffer;

    // ���������� ������������ �����
    int startHour, startMinute, endHour, endMinute;
    sscanf(startTime.c_str(), "%d:%d", &startHour, &startMinute);
    sscanf(endTime.c_str(), "%d:%d", &endHour, &endMinute);

    double totalHours = (endHour * 60 + endMinute - (startHour * 60 + startMinute)) / 60.0;

    // ���������� ���������� � ������ ������
    std::stringstream logEntry;
    logEntry << "������: " << startTime << ", ���������: " << endTime << ", ����������: " << totalHours << " �.";
    workLog.push_back(logEntry.str());

    // ���������� ������ � ����
    SendMessageA(hwndList, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(logEntry.str().c_str()));

    // ���������� � ����
    std::ofstream outFile(logFileName, std::ios::app);
    outFile << logEntry.str() << std::endl;
    outFile.close();
}

// ������� ��� �������� ������ �� �����
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

// ���������� ��������� ����
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            // �������� ��������� ���������� (����������� �������, ����� �����, ������ � ������)
            hwndStart = CreateWindowA("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_CENTER,
                                      50, 50, 100, 25, hwnd, NULL, NULL, NULL);
            hwndEnd = CreateWindowA("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_CENTER,
                                    200, 50, 100, 25, hwnd, NULL, NULL, NULL);
            hwndAddButton = CreateWindowA("BUTTON", "��������", WS_VISIBLE | WS_CHILD,
                                          350, 50, 100, 25, hwnd, reinterpret_cast<HMENU>(1), NULL, NULL);
            hwndList = CreateWindowA("LISTBOX", "", WS_VISIBLE | WS_CHILD | WS_BORDER | LBS_STANDARD,
                                     50, 100, 400, 300, hwnd, NULL, NULL, NULL);
            loadWorkLog();
            break;

        case WM_COMMAND:
            if (LOWORD(wParam) == 1 && HIWORD(wParam) == BN_CLICKED) {
                // ��������� ������� ������ "��������"
                addWorkTime();
            }
            break;

        case WM_DESTROY:
            // ���������� ����������
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

// ������� �������� � ������� �������� ����������
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // ����������� ������ ����
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "WorkTimeApp";

    RegisterClass(&wc);

    // �������� ����
    HWND hwnd = CreateWindowEx(0, "WorkTimeApp", "���� �������� �������", WS_OVERLAPPEDWINDOW,
                               CW_USEDEFAULT, CW_USEDEFAULT, 500, 500, NULL, NULL, hInstance, NULL);

    if (hwnd == NULL) {
        return 0;
    }

    // ����������� ����
    ShowWindow(hwnd, nCmdShow);

    // ���� ��������� ���������
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return static_cast<int>(msg.wParam);
}
