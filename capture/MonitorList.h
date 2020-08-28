#pragma once
#include<Windows.h>
#include<WinUser.h>
#include<windef.h>
#include<iostream>
#include<vector>
#include"pch.h"

struct MonitorInfo
{
	MonitorInfo(HMONITOR monitorHandle)
	{
		MonitorHandle = monitorHandle;
		MONITORINFOEX monitorInfo = { sizeof(monitorInfo) };
		winrt::check_bool(GetMonitorInfo(MonitorHandle, &monitorInfo));
		//char* ==> wstring
		int length = MultiByteToWideChar(CP_ACP, 0, monitorInfo.szDevice, -1, NULL, 0);
		WCHAR* buf = new WCHAR[length + 1];
		ZeroMemory(buf, (length + 1) * sizeof(WCHAR));
		MultiByteToWideChar(CP_ACP, 0, monitorInfo.szDevice, -1, buf, length);
		std::wstring strRet(buf);
		delete[] buf;

		std::wstring displayName(strRet);
		DisplayName = displayName;
	}
	MonitorInfo(HMONITOR monitorHandle, std::wstring const& displayName)
	{
		MonitorHandle = monitorHandle;
		DisplayName = displayName;
	}

	HMONITOR MonitorHandle;
	std::wstring DisplayName;

	bool operator==(const MonitorInfo& monitor) { return MonitorHandle == monitor.MonitorHandle; }
	bool operator!=(const MonitorInfo& monitor) { return !(*this == monitor); }
};

class MonitorList
{
public:
	MonitorList(bool includeAllMonitors);

	const std::vector<MonitorInfo> GetCurrentMonitors() { return m_monitors; }
	std::vector<MonitorInfo> getMonitors();
	std::vector<MonitorInfo> m_monitors;
	bool m_includeAllMonitors = false;
};