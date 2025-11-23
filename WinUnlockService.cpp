#include "WinUnlockService.h"
#include <strsafe.h>
#include <shlwapi.h>

#pragma comment(lib, "wtsapi32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "shlwapi.lib")

// 静态成员变量
SERVICE_STATUS CWinUnlockService::m_ServiceStatus = {0};
SERVICE_STATUS_HANDLE CWinUnlockService::m_hServiceStatusHandle = nullptr;
CWinUnlockService* CWinUnlockService::m_pThis = nullptr;

// 隐藏窗口类名
#define HIDDEN_WINDOW_CLASS L"WinUnlockServiceHiddenWindow"

CWinUnlockService::CWinUnlockService() :
    m_bServiceStop(FALSE),
    m_hServiceStopEvent(nullptr),
    m_hWnd(nullptr)
{
    m_pThis = this;
}

CWinUnlockService::~CWinUnlockService()
{
    Cleanup();
    if (m_pThis == this)
    {
        m_pThis = nullptr;
    }
}

// 服务主函数
void WINAPI CWinUnlockService::ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv)
{
    UNREFERENCED_PARAMETER(dwArgc);
    UNREFERENCED_PARAMETER(lpszArgv);

    // 注册服务控制处理程序
    m_hServiceStatusHandle = RegisterServiceCtrlHandler(SERVICE_NAME, ServiceCtrlHandler);
    if (m_hServiceStatusHandle == nullptr)
    {
        return;
    }

    // 初始化服务状态
    m_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    m_ServiceStatus.dwServiceSpecificExitCode = 0;

    // 报告服务正在启动
    m_pThis->ReportServiceStatus(SERVICE_START_PENDING, NO_ERROR, 3000);

    // 初始化服务
    if (!m_pThis->Initialize())
    {
        m_pThis->ReportServiceStatus(SERVICE_STOPPED, GetLastError(), 0);
        return;
    }

    // 报告服务正在运行
    m_pThis->ReportServiceStatus(SERVICE_RUNNING, NO_ERROR, 0);

    // 运行服务主循环
    m_pThis->Run();

    // 报告服务已停止
    m_pThis->ReportServiceStatus(SERVICE_STOPPED, NO_ERROR, 0);
}

// 服务控制处理程序
void WINAPI CWinUnlockService::ServiceCtrlHandler(DWORD dwCtrlCode)
{
    switch (dwCtrlCode)
    {
    case SERVICE_CONTROL_STOP:
        if (m_pThis)
        {
            m_pThis->ReportServiceStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);
            m_pThis->Stop();
        }
        break;

    case SERVICE_CONTROL_INTERROGATE:
        if (m_pThis)
        {
            m_pThis->ReportServiceStatus(m_ServiceStatus.dwCurrentState, NO_ERROR, 0);
        }
        break;

    default:
        break;
    }
}

// 报告服务状态
void CWinUnlockService::ReportServiceStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint)
{
    static DWORD dwCheckPoint = 1;

    m_ServiceStatus.dwCurrentState = dwCurrentState;
    m_ServiceStatus.dwWin32ExitCode = dwWin32ExitCode;
    m_ServiceStatus.dwWaitHint = dwWaitHint;

    if (dwCurrentState == SERVICE_START_PENDING)
    {
        m_ServiceStatus.dwControlsAccepted = 0;
    }
    else
    {
        m_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    }

    if ((dwCurrentState == SERVICE_RUNNING) || (dwCurrentState == SERVICE_STOPPED))
    {
        m_ServiceStatus.dwCheckPoint = 0;
    }
    else
    {
        m_ServiceStatus.dwCheckPoint = dwCheckPoint++;
    }

    SetServiceStatus(m_hServiceStatusHandle, &m_ServiceStatus);
}

// 初始化服务
BOOL CWinUnlockService::Initialize()
{
    // 创建停止事件
    m_hServiceStopEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
    if (m_hServiceStopEvent == nullptr)
    {
        return FALSE;
    }

    // 创建隐藏窗口用于接收WTS消息
    WNDCLASSW wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = HIDDEN_WINDOW_CLASS;

    if (RegisterClassW(&wc) == 0 && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
    {
        return FALSE;
    }

    m_hWnd = CreateWindowW(
        HIDDEN_WINDOW_CLASS,
        L"",
        0,
        0, 0, 0, 0,
        HWND_MESSAGE,
        nullptr,
        GetModuleHandle(nullptr),
        nullptr
    );

    if (m_hWnd == nullptr)
    {
        return FALSE;
    }

    // 注册会话通知
    if (!RegisterSessionNotification())
    {
        return FALSE;
    }

    return TRUE;
}

// 清理资源
void CWinUnlockService::Cleanup()
{
    // 取消注册会话通知
    UnregisterSessionNotification();

    // 销毁窗口
    if (m_hWnd != nullptr)
    {
        DestroyWindow(m_hWnd);
        m_hWnd = nullptr;
    }

    // 关闭事件
    if (m_hServiceStopEvent != nullptr)
    {
        CloseHandle(m_hServiceStopEvent);
        m_hServiceStopEvent = nullptr;
    }
}

// 运行服务主循环
BOOL CWinUnlockService::Run()
{
    MSG msg;
    BOOL bRet;

    while (!m_bServiceStop)
    {
        // 处理窗口消息（包括WTS会话变更消息）
        bRet = PeekMessage(&msg, m_hWnd, 0, 0, PM_REMOVE);
        if (bRet)
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // 检查停止事件
        if (WaitForSingleObject(m_hServiceStopEvent, 100) == WAIT_OBJECT_0)
        {
            m_bServiceStop = TRUE;
            break;
        }
    }

    return TRUE;
}

// 停止服务
void CWinUnlockService::Stop()
{
    m_bServiceStop = TRUE;
    if (m_hServiceStopEvent != nullptr)
    {
        SetEvent(m_hServiceStopEvent);
    }
}

// 窗口过程
LRESULT CALLBACK CWinUnlockService::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_WTSSESSION_CHANGE)
    {
        if (m_pThis)
        {
            m_pThis->OnSessionChange(wParam, lParam);
        }
        return 0;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

// 注册会话通知
BOOL CWinUnlockService::RegisterSessionNotification()
{
    if (m_hWnd == nullptr)
    {
        return FALSE;
    }

    return WTSRegisterSessionNotification(m_hWnd, NOTIFY_FOR_ALL_SESSIONS);
}

// 取消注册会话通知
void CWinUnlockService::UnregisterSessionNotification()
{
    if (m_hWnd != nullptr)
    {
        WTSUnRegisterSessionNotification(m_hWnd);
    }
}

// 处理会话变更
void CWinUnlockService::OnSessionChange(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);

    switch (wParam)
    {
    case WTS_SESSION_DISCONNECT:
    case WTS_REMOTE_DISCONNECT:
        // RDP会话断开
        HandleRdpDisconnect();
        break;

    case WTS_SESSION_LOCK:
        // 会话锁定（可能是RDP断开导致的）
        // 注意：RDP断开时，Windows可能会先锁定会话
        break;

    case WTS_SESSION_UNLOCK:
        // 会话解锁
        ClearUnlockRequestFlag();
        break;

    default:
        break;
    }
}

// 处理RDP断开
void CWinUnlockService::HandleRdpDisconnect()
{
    // 检查是否有活动的RDP会话
    // 如果没有其他RDP会话，则触发解锁
    BOOL bHasActiveRdpSession = FALSE;
    PWTS_SESSION_INFOW pSessionInfo = nullptr;
    DWORD dwCount = 0;

    if (WTSEnumerateSessionsW(WTS_CURRENT_SERVER_HANDLE, 0, 1, &pSessionInfo, &dwCount))
    {
        for (DWORD i = 0; i < dwCount; i++)
        {
            // 检查是否是RDP会话（会话类型为WTSActive）
            if (pSessionInfo[i].State == WTSActive)
            {
                // 检查是否是远程会话（会话ID > 0 且不是控制台会话）
                if (pSessionInfo[i].SessionId != 0 && pSessionInfo[i].SessionId != WTS_CURRENT_SESSION)
                {
                    // 检查会话名称是否以"RDP"开头（RDP会话通常以"RDP-Tcp#"命名）
                    LPWSTR pWinStationName = nullptr;
                    DWORD dwBytesReturned = 0;
                    
                    if (WTSQuerySessionInformationW(
                        WTS_CURRENT_SERVER_HANDLE,
                        pSessionInfo[i].SessionId,
                        WTSWinStationName,
                        &pWinStationName,
                        &dwBytesReturned))
                    {
                        if (pWinStationName != nullptr && wcsncmp(pWinStationName, L"RDP", 3) == 0)
                        {
                            bHasActiveRdpSession = TRUE;
                            WTSFreeMemory(pWinStationName);
                            break;
                        }
                        if (pWinStationName != nullptr)
                        {
                            WTSFreeMemory(pWinStationName);
                        }
                    }
                }
            }
        }

        WTSFreeMemory(pSessionInfo);
    }

    // 如果没有活动的RDP会话，触发解锁
    if (!bHasActiveRdpSession)
    {
        // 等待一小段时间，确保会话完全断开
        Sleep(1000);
        TriggerUnlock();
    }
}

// 触发解锁
BOOL CWinUnlockService::TriggerUnlock()
{
    // 设置解锁请求标志
    if (!SetUnlockRequestFlag())
    {
        return FALSE;
    }

    // 锁定工作站以触发凭据提供程序
    // 注意：这需要服务有适当的权限
    LockWorkStation();

    // 等待一小段时间让锁定生效
    Sleep(500);

    // 凭据提供程序应该检测到标志并自动解锁
    // 这里我们只是触发，实际的解锁由凭据提供程序完成

    return TRUE;
}

// 设置解锁请求标志
BOOL CWinUnlockService::SetUnlockRequestFlag()
{
    HKEY hKey = nullptr;
    LONG lResult = RegCreateKeyExW(
        HKEY_LOCAL_MACHINE,
        REGISTRY_KEY_PATH,
        0,
        nullptr,
        REG_OPTION_NON_VOLATILE,
        KEY_WRITE,
        nullptr,
        &hKey,
        nullptr
    );

    if (lResult != ERROR_SUCCESS)
    {
        return FALSE;
    }

    DWORD dwValue = 1;
    lResult = RegSetValueExW(
        hKey,
        REGISTRY_VALUE_UNLOCK_REQUEST,
        0,
        REG_DWORD,
        (LPBYTE)&dwValue,
        sizeof(dwValue)
    );

    RegCloseKey(hKey);
    return (lResult == ERROR_SUCCESS);
}

// 清除解锁请求标志
BOOL CWinUnlockService::ClearUnlockRequestFlag()
{
    HKEY hKey = nullptr;
    LONG lResult = RegOpenKeyExW(
        HKEY_LOCAL_MACHINE,
        REGISTRY_KEY_PATH,
        0,
        KEY_WRITE,
        &hKey
    );

    if (lResult != ERROR_SUCCESS)
    {
        return FALSE;
    }

    lResult = RegDeleteValueW(hKey, REGISTRY_VALUE_UNLOCK_REQUEST);
    RegCloseKey(hKey);

    return (lResult == ERROR_SUCCESS || lResult == ERROR_FILE_NOT_FOUND);
}

// 安装服务
BOOL CWinUnlockService::InstallService()
{
    WCHAR szPath[MAX_PATH] = {0};
    SC_HANDLE hSCManager = nullptr;
    SC_HANDLE hService = nullptr;
    BOOL bResult = FALSE;

    // 获取可执行文件路径
    if (GetModuleFileNameW(nullptr, szPath, MAX_PATH) == 0)
    {
        return FALSE;
    }

    // 打开服务控制管理器
    hSCManager = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
    if (hSCManager == nullptr)
    {
        return FALSE;
    }

    // 创建服务
    hService = CreateServiceW(
        hSCManager,
        SERVICE_NAME,
        SERVICE_DISPLAY_NAME,
        SERVICE_ALL_ACCESS,
        SERVICE_WIN32_OWN_PROCESS,
        SERVICE_AUTO_START,
        SERVICE_ERROR_NORMAL,
        szPath,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr
    );

    if (hService != nullptr)
    {
        // 设置服务描述
        SERVICE_DESCRIPTION sd;
        sd.lpDescription = (LPWSTR)SERVICE_DESCRIPTION;
        ChangeServiceConfig2W(hService, SERVICE_CONFIG_DESCRIPTION, &sd);

        bResult = TRUE;
        CloseServiceHandle(hService);
    }
    else if (GetLastError() == ERROR_SERVICE_EXISTS)
    {
        // 服务已存在，尝试启动它
        hService = OpenServiceW(hSCManager, SERVICE_NAME, SERVICE_START);
        if (hService != nullptr)
        {
            bResult = TRUE;
            CloseServiceHandle(hService);
        }
    }

    CloseServiceHandle(hSCManager);
    return bResult;
}

// 卸载服务
BOOL CWinUnlockService::UninstallService()
{
    SC_HANDLE hSCManager = nullptr;
    SC_HANDLE hService = nullptr;
    BOOL bResult = FALSE;

    // 打开服务控制管理器
    hSCManager = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
    if (hSCManager == nullptr)
    {
        return FALSE;
    }

    // 打开服务
    hService = OpenServiceW(hSCManager, SERVICE_NAME, SERVICE_ALL_ACCESS);
    if (hService != nullptr)
    {
        // 停止服务
        SERVICE_STATUS_PROCESS ssp;
        DWORD dwBytesNeeded;
        if (QueryServiceStatusEx(hService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded))
        {
            if (ssp.dwCurrentState != SERVICE_STOPPED)
            {
                ControlService(hService, SERVICE_CONTROL_STOP, (LPSERVICE_STATUS)&ssp);
                Sleep(1000);
            }
        }

        // 删除服务
        if (DeleteService(hService))
        {
            bResult = TRUE;
        }

        CloseServiceHandle(hService);
    }

    CloseServiceHandle(hSCManager);
    return bResult;
}

// 主函数
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(nCmdShow);

    // 获取命令行参数
    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);

    // 检查命令行参数
    if (argc > 1 && argv != nullptr)
    {
        if (wcscmp(argv[1], L"/install") == 0 || wcscmp(argv[1], L"-install") == 0)
        {
            // 安装服务
            if (CWinUnlockService::InstallService())
            {
                MessageBoxW(nullptr, L"服务安装成功", L"WinUnlock Service", MB_OK | MB_ICONINFORMATION);
                LocalFree(argv);
                return 0;
            }
            else
            {
                MessageBoxW(nullptr, L"服务安装失败", L"WinUnlock Service", MB_OK | MB_ICONERROR);
                LocalFree(argv);
                return 1;
            }
        }
        else if (wcscmp(argv[1], L"/uninstall") == 0 || wcscmp(argv[1], L"-uninstall") == 0)
        {
            // 卸载服务
            if (CWinUnlockService::UninstallService())
            {
                MessageBoxW(nullptr, L"服务卸载成功", L"WinUnlock Service", MB_OK | MB_ICONINFORMATION);
                LocalFree(argv);
                return 0;
            }
            else
            {
                MessageBoxW(nullptr, L"服务卸载失败", L"WinUnlock Service", MB_OK | MB_ICONERROR);
                LocalFree(argv);
                return 1;
            }
        }
        LocalFree(argv);
    }

    // 准备服务表
    SERVICE_TABLE_ENTRYW serviceTable[] =
    {
        { (LPWSTR)SERVICE_NAME, ServiceMain },
        { nullptr, nullptr }
    };

    // 启动服务控制调度器
    if (!StartServiceCtrlDispatcherW(serviceTable))
    {
        // 如果不是作为服务运行，可能是从命令行直接运行
        // 在这种情况下，显示错误消息
        DWORD dwError = GetLastError();
        if (dwError == ERROR_FAILED_SERVICE_CONTROLLER_CONNECT)
        {
            MessageBoxW(nullptr, 
                L"此程序必须作为Windows服务运行。\n\n"
                L"要安装服务，请使用以下命令：\n"
                L"WinUnlockService.exe /install\n\n"
                L"要卸载服务，请使用：\n"
                L"WinUnlockService.exe /uninstall",
                L"WinUnlock Service",
                MB_OK | MB_ICONINFORMATION);
        }
        return 1;
    }

    return 0;
}

