#include <winrt/Windows.Foundation.h>
#include<iostream>
#include"SimpleCapture.h"
#include"pch.h"
#include"MonitorList.h"

using namespace std;
using namespace winrt;
using namespace Windows::Foundation;

auto CreateDispatcherQueueController()
{
	namespace abi = ABI::Windows::System;

	DispatcherQueueOptions options
	{
		sizeof(DispatcherQueueOptions),
		DQTYPE_THREAD_CURRENT,
		DQTAT_COM_STA
	};

	Windows::System::DispatcherQueueController controller{ nullptr };
	check_hresult(CreateDispatcherQueueController(options, reinterpret_cast<abi::IDispatcherQueueController**>(put_abi(controller))));
	return controller;
}

int getWindows(HWND &hwnd)
{
	//得到桌面窗口
	HWND hd = GetDesktopWindow();
	//得到屏幕上第一个子窗口
	hd = GetWindow(hd, GW_CHILD);
	char s[200] = { 0 };

	//循环得到所有的子窗口
	while (hd != NULL)
	{
		memset(s, 0, 200);
		if (IsWindowVisible(hd)) {
			GetWindowText(hd, s, 200);
			if (!strcmp(s, "gzhb.pptx - PowerPoint"))
			{

				//cout << "find !!!!" << endl;
				hwnd = hd;
				/*cout<<s<<endl;*/
				//SetWindowText(hd,"My Windows");
				return 0;
			}
			//cout << s << endl;
		}


		hd = GetNextWindow(hd, GW_HWNDNEXT);
	}
	return 0;
}

int main()
{
	auto controller = CreateDispatcherQueueController();
	auto queue = controller.DispatcherQueue();
	auto success = queue.TryEnqueue([=]() -> void
	{
		cout << "queue try enqueue success" << endl;
	});
	std::unique_ptr<MonitorList> m_monitors;
	m_monitors = std::make_unique<MonitorList>(true);
	WINRT_VERIFY(success);
	HWND hwnd;
	getWindows(hwnd);
	auto d3dDevice = CreateD3DDevice();
	auto dxgiDevice = d3dDevice.as<IDXGIDevice>();
	auto m_device = CreateDirect3DDevice(dxgiDevice.get());
	//auto item = CreateCaptureItemForWindow(hwnd);
	auto item = CreateCaptureItemForMonitor(m_monitors->getMonitors()[0].MonitorHandle);
	auto m_capture = std::make_unique<SimpleCapture>(m_device, item);
	m_capture->StartCapture();
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		Sleep(1000);
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	system("pause");
	return 0;
}