#pragma once

#include "framework.h"
#include "Menu.hpp"
#include "roboto.ttf.h"

namespace Hook 
{
	typedef long(__stdcall* Present)(IDXGISwapChain*, UINT, UINT);
	static Present oPresent = NULL;

	ID3D11Device* pDevice = NULL;
	ID3D11DeviceContext* pContext = NULL;
	ID3D11RenderTargetView* mainRenderTargetView;

	long __stdcall hkPresent11(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
	{
		static bool init = false;

		if (!init)
		{
			if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&pDevice)))
			{
				ImGui::CreateContext();

				DXGI_SWAP_CHAIN_DESC desc;
				pSwapChain->GetDesc(&desc);

				pDevice->GetImmediateContext(&pContext);

				ID3D11Texture2D* pBackBuffer;
				pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
				pDevice->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetView);
				pBackBuffer->Release();

				Menu::BeginInput(desc.OutputWindow);
				Menu::BeginStyle();

				ImGuiIO& io = ImGui::GetIO();
				io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;

				ImGui_ImplWin32_Init(desc.OutputWindow);
				ImGui_ImplDX11_Init(pDevice, pContext);

				m_pFont = io.Fonts->AddFontFromMemoryCompressedTTF(Roboto_compressed_data, Roboto_compressed_size, 14.0f, nullptr, io.Fonts->GetGlyphRangesCyrillic());

				init = true;
			}
		}

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		Menu::Draw();

		ImGui::Render();
		pContext->OMSetRenderTargets(1, &mainRenderTargetView, NULL);
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		return oPresent(pSwapChain, SyncInterval, Flags);
	}

	bool RenderBind()
	{
		if (kiero::init(kiero::RenderType::D3D11) == kiero::Status::Success)
		{
			kiero::bind(8, (void**)&oPresent, hkPresent11);
			return true;
		}

		return false;
	}
}
