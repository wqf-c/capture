#pragma once
#include <initguid.h>
#include <windows.h>
#undef GetCurrentTime
#include<stdio.h>
#include<iostream>
#include <d3d11_3.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <wincodec.h>   // WIC

#include <wrl.h>


using namespace Microsoft::WRL;



void SaveTextureToBmp(PCWSTR FileName, ID3D11Texture2D* Texture)
{

	HRESULT hr;

	// First verify that we can map the texture
	D3D11_TEXTURE2D_DESC desc;
	Texture->GetDesc(&desc);

	// translate texture format to WIC format. We support only BGRA and ARGB.
	GUID wicFormatGuid;
	switch (desc.Format) {
	case DXGI_FORMAT_R8G8B8A8_UNORM:
		wicFormatGuid = GUID_WICPixelFormat32bppRGBA;
		break;
	case DXGI_FORMAT_B8G8R8A8_UNORM:
		wicFormatGuid = GUID_WICPixelFormat32bppBGRA;
		break;
	default:
		printf("Unsupported DXGI_FORMAT: %d. Only RGBA and BGRA are supported.", desc.Format);
		return;
	}

	// Get the device context
	ComPtr<ID3D11Device> d3dDevice;
	Texture->GetDevice(&d3dDevice);
	ComPtr<ID3D11DeviceContext> d3dContext;
	d3dDevice->GetImmediateContext(&d3dContext);

	// map the texture
	ComPtr<ID3D11Texture2D> mappedTexture;
	D3D11_MAPPED_SUBRESOURCE mapInfo;
	mapInfo.RowPitch;
	hr = d3dContext->Map(
		Texture,
		0,  // Subresource
		D3D11_MAP_READ,
		0,  // MapFlags
		&mapInfo);

	if (FAILED(hr)) {
		// If we failed to map the texture, copy it to a staging resource
		if (hr == E_INVALIDARG) {
			D3D11_TEXTURE2D_DESC desc2;
			desc2.Width = desc.Width;
			desc2.Height = desc.Height;
			desc2.MipLevels = desc.MipLevels;
			desc2.ArraySize = desc.ArraySize;
			desc2.Format = desc.Format;
			desc2.SampleDesc = desc.SampleDesc;
			desc2.Usage = D3D11_USAGE_STAGING;
			desc2.BindFlags = 0;
			desc2.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
			desc2.MiscFlags = 0;

			ComPtr<ID3D11Texture2D> stagingTexture;
			hr = d3dDevice->CreateTexture2D(&desc2, nullptr, &stagingTexture);
			if (FAILED(hr)) {
				printf("Failed to create staging texture");
				return;
			}

			// copy the texture to a staging resource
			d3dContext->CopyResource(stagingTexture.Get(), Texture);

			// now, map the staging resource
			hr = d3dContext->Map(
				stagingTexture.Get(),
				0,
				D3D11_MAP_READ,
				0,
				&mapInfo);
			if (FAILED(hr)) {
				printf("Failed to map staging texture");
				return;
				
			}

			mappedTexture = std::move(stagingTexture);
		}
		else {
			printf("Failed to map texture.");
			return;
		}
	}
	else {
		mappedTexture = Texture;
	}
	/*auto unmapResource = Finally([&] {
		d3dContext->Unmap(mappedTexture.Get(), 0);
	});*/

	ComPtr<IWICImagingFactory> wicFactory;
	hr = CoCreateInstance(
		CLSID_WICImagingFactory,
		nullptr,
		CLSCTX_INPROC_SERVER,
		__uuidof(wicFactory),
		reinterpret_cast<void**>(wicFactory.GetAddressOf()));
	if (FAILED(hr)) {
		printf("Failed to create instance of WICImagingFactory");
		return;
	}

	ComPtr<IWICBitmapEncoder> wicEncoder;
	hr = wicFactory->CreateEncoder(
		GUID_ContainerFormatBmp,
		nullptr,
		&wicEncoder);
	if (FAILED(hr)) {
		printf("Failed to create BMP encoder");
		return;
	}

	ComPtr<IWICStream> wicStream;
	hr = wicFactory->CreateStream(&wicStream);
	if (FAILED(hr)) {
		printf("Failed to create IWICStream");
		return;
	}

	hr = wicStream->InitializeFromFilename(FileName, GENERIC_WRITE);
	if (FAILED(hr)) {
		printf("Failed to initialize stream from file name");
		return;
	}

	hr = wicEncoder->Initialize(wicStream.Get(), WICBitmapEncoderNoCache);
	if (FAILED(hr)) {
		printf("Failed to initialize bitmap encoder");
		return;
	}

	// Encode and commit the frame
	{
		ComPtr<IWICBitmapFrameEncode> frameEncode;
		wicEncoder->CreateNewFrame(&frameEncode, nullptr);
		if (FAILED(hr)) {
			printf("Failed to create IWICBitmapFrameEncode");
			return;

		}

		hr = frameEncode->Initialize(nullptr);
		if (FAILED(hr)) {
			printf("Failed to initialize IWICBitmapFrameEncode\n");
			return;
		}


		hr = frameEncode->SetPixelFormat(&wicFormatGuid);
		if (FAILED(hr)) {
			printf("SetPixelFormat failed.\n");
			return;
		}

		hr = frameEncode->SetSize(desc.Width, desc.Height);
		if (FAILED(hr)) {
			printf("SetSize(...) failed.\n");
			return;

		}

		hr = frameEncode->WritePixels(
			desc.Height,
			mapInfo.RowPitch,
			desc.Height * mapInfo.RowPitch,
			reinterpret_cast<BYTE*>(mapInfo.pData));
		if (FAILED(hr)) {
			printf("frameEncode->WritePixels(...) failed.\n");
			return;
		}

		hr = frameEncode->Commit();
		if (FAILED(hr)) {
			printf("Failed to commit frameEncode\n");
			return;
		}
	}

	hr = wicEncoder->Commit();
	if (FAILED(hr)) {
		printf("Failed to commit encoder\n");
	}
	d3dContext->Unmap(mappedTexture.Get(), 0);
}