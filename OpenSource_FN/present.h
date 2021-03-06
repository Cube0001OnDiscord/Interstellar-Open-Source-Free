#pragma once
#include "imports.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_internal.h"
#include <D3D11.h>
#include "Settings.h"
#include "xorstr.h"
#include "main.h"

#pragma comment(lib, "C:\\Program Files (x86)\\Microsoft DirectX SDK (June 2010)\\Lib\\x64\\d3d11.lib")

static bool firstTime = true;
static ID3D11DeviceContext* m_pContext;
static ID3D11RenderTargetView* g_pRenderTargetView;
static IDXGISwapChain* g_pSwapChain;
static ID3D11Device* uDevice;
static WNDPROC oWndProc;
HRESULT(*PresentOriginal)(IDXGISwapChain* pthis, UINT syncInterval, UINT flags) = nullptr;
using f_present = HRESULT(__stdcall*)(IDXGISwapChain* pthis, UINT sync_interval, UINT flags);
f_present swapchainA = nullptr;

bool showmenu = true;


ImGuiWindow& BeginScene() {
	ImGui_ImplDX11_NewFrame();
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
	ImGui::Begin(("##scene"), nullptr, ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoTitleBar);

	auto& io = ImGui::GetIO();
	ImGui::SetWindowPos(ImVec2(0, 0), ImGuiCond_Always);
	ImGui::SetWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y), ImGuiCond_Always);

	return *ImGui::GetCurrentWindow();
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProcHook(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (msg == WM_KEYUP && wParam == VK_INSERT)
	{
		showmenu = !showmenu;
		ImGui::GetIO().MouseDrawCursor = showmenu;
	}
    if (msg == WM_KEYUP && wParam == VK_F1) 
	{
		settings::memoryaim = !settings::memoryaim;
	}
    if (msg == WM_KEYUP && wParam == VK_F2) 
	{
		settings::boxesp = !settings::boxesp;
	}
    if (msg == WM_KEYUP && wParam == VK_F3)
	{
		settings::snaplines = !settings::snaplines;
	}
	else if (msg == WM_QUIT && showmenu)
	{
		ExitProcess(0);
	}

	ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
	
	return CallWindowProcW(oWndProc, hWnd, msg, wParam, lParam);
}

float hsv = 0;

HRESULT presenthook(IDXGISwapChain* swap, UINT sync_interval, UINT flags)
{
	ID3D11Texture2D* pRenderTargetTexture;

	g_pSwapChain = swap;

	if (firstTime) {

		swap->GetDevice(__uuidof(ID3D11Device), (void**)&uDevice);

		if (!uDevice) return false;

		uDevice->GetImmediateContext(&m_pContext);

		if (!m_pContext) return false;


		if (SUCCEEDED(swap->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pRenderTargetTexture)))
		{
			uDevice->CreateRenderTargetView(pRenderTargetTexture, NULL, &g_pRenderTargetView);
			pRenderTargetTexture->Release();
			uDevice->Release();
		}

		HWND test = iat(FindWindowA)(E("UnrealWindow"), E("Fortnite  "));
		if (!test) test = iat(GetForegroundWindow)();
		oWndProc = reinterpret_cast<WNDPROC>(iat(SetWindowLongPtrW)(test, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WndProcHook)));

		firstTime = false;
	}


	if (!g_pRenderTargetView)
	{
		swap->GetDevice(__uuidof(ID3D11Device), (void**)&uDevice);

		if (!uDevice) return false;

		uDevice->GetImmediateContext(&m_pContext);

		if (!m_pContext) return false;

		if (SUCCEEDED(swap->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pRenderTargetTexture)))
		{
			uDevice->CreateRenderTargetView(pRenderTargetTexture, NULL, &g_pRenderTargetView);

			D3D11_TEXTURE2D_DESC backBufferDesc = { 0 };
			pRenderTargetTexture->GetDesc(&backBufferDesc);
			pRenderTargetTexture->Release();

			uDevice->Release();
		}
	}

	if (g_pRenderTargetView)
	{
		ImGui::GetIO().Fonts->AddFontFromFileTTF(E("C:\\Windows\\Fonts\\arialbd.ttf"), 12.0f);
		ImGui_ImplDX11_Init(iat(GetForegroundWindow)(), uDevice, m_pContext);
		ImGui_ImplDX11_CreateDeviceObjects();
		m_pContext->OMSetRenderTargets(1, &g_pRenderTargetView, NULL);
		auto& windowshit = BeginScene();
		windowshit.DrawList->PushClipRectFullScreen();

		if (hsv != 255)
			hsv += 1;
		else
			hsv = 0;

		if (iat(GetAsyncKeyState)(VK_ADD))
			settings::fov += 1;

		if (iat(GetAsyncKeyState)(VK_SUBTRACT))
			settings::fov -= 1;

		std::string mem = (settings::memoryaim ? E("TRUE") : E("FALSE"));
		std::string box = (settings::boxesp ? E("TRUE") : E("FALSE"));
		std::string snap = (settings::snaplines ? E("TRUE") : E("FALSE"));

		std::string options = E("Memory Aim (F1): ") + mem + E(" | Box ESP (F2): ") + box + E(" | Snaplines (F3): ") + snap + E(" | Aim Fov (+/-): ") + std::to_string(settings::fov) + E("   MEMORY AIM IS BROKEN (make a commit to fix it or wait untill i fix)");

		windowshit.DrawList->AddText(ImVec2(50, 50), ImColor::HSV(hsv / 255.f, 255, 255), E("Interstellar Open Source Free (If You Bought This You Got Scammed)"));
		windowshit.DrawList->AddText(ImVec2(50, 75), ImColor::HSV(hsv / 255.f, 255, 255), options.c_str());

		windowshit.DrawList->AddCircle(ImVec2(iat(GetSystemMetrics)(0) / 2, iat(GetSystemMetrics)(1) / 2), settings::fov, ImGui::GetColorU32({ 0.f, 0.f, 0.f, 1.f }), 20, 1.f);

		cheatinit(windowshit, iat(GetSystemMetrics)(0),  iat(GetSystemMetrics)(1));
		
		ImGui::End();
		ImGui::SetWindowPos(ImVec2(0, 0), ImGuiCond_Always);
		ImGui::SetWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y), ImGuiCond_Always);
		ImGui::SetNextWindowSize({ 500, 400 }, ImGuiCond_Always);
		ImGui::SetNextWindowCollapsed(false, ImGuiCond_Always);
		ImGui::Render();

		ImGui_ImplDX11_Shutdown();
	}

	return swapchainA(swap, sync_interval, flags);
}