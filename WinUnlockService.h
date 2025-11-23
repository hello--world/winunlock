#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <wtsapi32.h>
#include <winsvc.h>
#include <string>

// 服务名称和显示名称
#define SERVICE_NAME L"WinUnlockService"
#define SERVICE_DISPLAY_NAME L"WinUnlock RDP Auto-Unlock Service"
#define SERVICE_DESCRIPTION L"Monitors RDP session disconnections and automatically unlocks the workstation"

// 注册表路径（用于与服务通信）
#define REGISTRY_KEY_PATH L"SOFTWARE\\WinUnlock"
#define REGISTRY_VALUE_RDP_DISCONNECTED L"RdpDisconnected"
#define REGISTRY_VALUE_UNLOCK_REQUEST L"UnlockRequest"

// Windows服务类
class CWinUnlockService
{
public:
    CWinUnlockService();
    ~CWinUnlockService();

    // 服务主函数
    static void WINAPI ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv);
    static void WINAPI ServiceCtrlHandler(DWORD dwCtrlCode);

    // 服务安装和卸载
    static BOOL InstallService();
    static BOOL UninstallService();

    // 服务运行
    BOOL Run();
    void Stop();

private:
    // 服务状态
    static SERVICE_STATUS m_ServiceStatus;
    static SERVICE_STATUS_HANDLE m_hServiceStatusHandle;
    static CWinUnlockService* m_pThis;

    // 服务控制
    BOOL m_bServiceStop;
    HANDLE m_hServiceStopEvent;

    // RDP会话监控
    HWND m_hWnd;
    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    BOOL RegisterSessionNotification();
    void UnregisterSessionNotification();
    void OnSessionChange(WPARAM wParam, LPARAM lParam);
    void HandleRdpDisconnect();

    // 解锁相关
    BOOL TriggerUnlock();
    BOOL SetUnlockRequestFlag();
    BOOL ClearUnlockRequestFlag();

    // 辅助函数
    BOOL Initialize();
    void Cleanup();
    void ReportServiceStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint);
};

