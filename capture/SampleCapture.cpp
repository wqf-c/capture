#include <unknwn.h>
#include"SimpleCapture.h"
#include"direct3d11.interop.h"
#include <fstream>
#include<iostream>
#include"SavePic.h"

using namespace std;
using namespace winrt;
namespace winrt {
	using namespace Windows;
	using namespace Windows::Foundation;
	using namespace Windows::System;
	using namespace Windows::Graphics;
	using namespace Windows::Graphics::Capture;
	using namespace Windows::Graphics::DirectX;
	using namespace Windows::Graphics::DirectX::Direct3D11;
	using namespace Windows::Foundation::Numerics;
}







using std::cout;
using std::endl;



SimpleCapture::SimpleCapture(
	IDirect3DDevice const& device,
	GraphicsCaptureItem const& item)
{
	m_item = item;
	m_device = device;

	// Set up 
	/*auto d3dDevice = GetDXGIInterfaceFromObject<ID3D11Device>(m_device);
	d3dDevice->GetImmediateContext(m_d3dContext.put());*/

	auto size = m_item.Size();

	/*m_swapChain = CreateDXGISwapChain(
		d3dDevice,
		static_cast<uint32_t>(size.Width),
		static_cast<uint32_t>(size.Height),
		static_cast<DXGI_FORMAT>(DirectXPixelFormat::B8G8R8A8UIntNormalized),
		2);*/

	// Create framepool, define pixel format (DXGI_FORMAT_B8G8R8A8_UNORM), and frame size. 
	m_framePool = Direct3D11CaptureFramePool::Create(
		m_device,
		DirectXPixelFormat::B8G8R8A8UIntNormalized,
		2,
		size);
	m_session = m_framePool.CreateCaptureSession(m_item);
	m_lastSize = size;
	m_frameArrived = m_framePool.FrameArrived(auto_revoke, { this, &SimpleCapture::OnFrameArrived });

	WINRT_ASSERT(m_session != nullptr);
}

// Start sending capture frames
void SimpleCapture::StartCapture()
{
	cout << "start capture" << endl;
	CheckClosed();
	m_session.StartCapture();
}

//ICompositionSurface SimpleCapture::CreateSurface(
//	Compositor const& compositor)
//{
//	CheckClosed();
//	//return CreateCompositionSurfaceForSwapChain(compositor, m_swapChain.get());
//}

// Process captured frames
void SimpleCapture::Close()
{
	auto expected = false;
	if (m_closed.compare_exchange_strong(expected, true))
	{
		m_frameArrived.revoke();
		m_framePool.Close();
		m_session.Close();

		m_swapChain = nullptr;
		m_framePool = nullptr;
		m_session = nullptr;
		m_item = nullptr;
	}
}

void SimpleCapture::OnFrameArrived(
	Direct3D11CaptureFramePool const& sender,
	winrt::Windows::Foundation::IInspectable const&)
{

	auto newSize = false;

	{
		auto frame = sender.TryGetNextFrame();
		auto frameContentSize = frame.ContentSize();

		if (frameContentSize.Width != m_lastSize.Width ||
			frameContentSize.Height != m_lastSize.Height)
		{
			// The thing we have been capturing has changed size.
			// We need to resize our swap chain first, then blit the pixels.
			// After we do that, retire the frame and then recreate our frame pool.
			newSize = true;
			m_lastSize = frameContentSize;
			/*m_swapChain->ResizeBuffers(
				2,
				static_cast<uint32_t>(m_lastSize.Width),
				static_cast<uint32_t>(m_lastSize.Height),
				static_cast<DXGI_FORMAT>(DirectXPixelFormat::B8G8R8A8UIntNormalized),
				0);*/
		}

		{
			auto frameSurface = GetDXGIInterfaceFromObject<ID3D11Texture2D>(frame.Surface());
			string str = "test" + std::to_string(count) + ".bmp";
			count++;
			char* szStr = (char *)str.c_str();
			WCHAR wszClassName[256];
			memset(wszClassName, 0, sizeof(wszClassName));
			MultiByteToWideChar(CP_ACP, 0, szStr, strlen(szStr) + 1, wszClassName,
				sizeof(wszClassName) / sizeof(wszClassName[0]));
			SaveTextureToBmp(wszClassName, frameSurface.get());
			/*com_ptr<ID3D11Texture2D> backBuffer;
			check_hresult(m_swapChain->GetBuffer(0, guid_of<ID3D11Texture2D>(), backBuffer.put_void()));

			m_d3dContext->CopyResource(backBuffer.get(), frameSurface.get());*/
		}
	}
/*
	DXGI_PRESENT_PARAMETERS presentParameters = { 0 };
	m_swapChain->Present1(1, 0, &presentParameters);*/

	if (newSize)
	{
		m_framePool.Recreate(
			m_device,
			DirectXPixelFormat::B8G8R8A8UIntNormalized,
			2,
			m_lastSize);
	}
}


//void SimpleCapture::TakeSnapshot(winrt::com_ptr<ID3D11Texture2D> const& frame)
//{
//	
//	auto m_pixelFormat = DirectXPixelFormat::B8G8R8A8UIntNormalized;
//	DirectX::ScratchImage im;
//	winrt::check_hresult(CaptureTexture(GetDXGIInterfaceFromObject<ID3D11Device>(m_device).get(),
//		m_d3dContext.get(), frame.get(), im));
//	const auto& realImage = *im.GetImage(0, 0, 0);
//	if (m_pixelFormat == DirectXPixelFormat::R16G16B16A16Float)
//	{
//		winrt::check_hresult(SaveToWICFile(realImage, WIC_FLAGS_NONE,
//			GUID_ContainerFormatWmp, L"output.jxr"));
//	}
//	else // BGRA8
//	{
//		winrt::check_hresult(SaveToWICFile(realImage, WIC_FLAGS_NONE,
//			GUID_ContainerFormatPng, L"output.png"));
//	}
//}
