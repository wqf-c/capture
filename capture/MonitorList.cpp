#include "MonitorList.h"

std::vector<MonitorInfo> EnumerateAllMonitors(bool includeAllMonitors)
{
	std::vector<MonitorInfo> monitors;
	EnumDisplayMonitors(nullptr, nullptr, [](HMONITOR hmon, HDC, LPRECT, LPARAM lparam)
	{
		auto& monitors = *reinterpret_cast<std::vector<MonitorInfo>*>(lparam);
		monitors.push_back(MonitorInfo(hmon));

		return TRUE;
	}, reinterpret_cast<LPARAM>(&monitors));
	if (monitors.size() > 1 && includeAllMonitors)
	{
		monitors.push_back(MonitorInfo(nullptr, L"All Displays"));
	}

	winrt::com_ptr<ID2D1Bitmap1> d2dBitmap;
	
	return monitors;
}

std::vector<MonitorInfo> MonitorList::getMonitors() {
	return m_monitors;
}

MonitorList::MonitorList(bool includeAllMonitors)
{
	m_includeAllMonitors = includeAllMonitors;
	m_monitors = EnumerateAllMonitors(m_includeAllMonitors);
}
