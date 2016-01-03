/*	Direct3D9 Interface */

#include <windows.h>
#include "main.h"
#include "d3d9.h"
#include "d3dutil.h"

#include "Settings.h"
#include "RenderstateManager.h"

HRESULT APIENTRY hkIDirect3D9::QueryInterface(REFIID riid,  void **ppvObj) {
	SDLOG(1, "hkIDirect3D9::QueryInterface\n");
	return m_pD3Dint->QueryInterface(riid,  ppvObj);
}

ULONG APIENTRY hkIDirect3D9::AddRef() {
	return m_pD3Dint->AddRef();
}

HRESULT APIENTRY hkIDirect3D9::CheckDepthStencilMatch(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat) {
	return m_pD3Dint->CheckDepthStencilMatch(Adapter, DeviceType, AdapterFormat, RenderTargetFormat, DepthStencilFormat);
}

HRESULT APIENTRY hkIDirect3D9::CheckDeviceFormat(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat) {
	return m_pD3Dint->CheckDeviceFormat(Adapter, DeviceType, AdapterFormat, Usage, RType, CheckFormat);
}

HRESULT APIENTRY hkIDirect3D9::CheckDeviceFormatConversion(UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT SourceFormat,D3DFORMAT TargetFormat) {
	return m_pD3Dint->CheckDeviceFormatConversion(Adapter, DeviceType, SourceFormat, TargetFormat);
}

HRESULT APIENTRY hkIDirect3D9::CheckDeviceMultiSampleType(UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT SurfaceFormat,BOOL Windowed,D3DMULTISAMPLE_TYPE MultiSampleType,DWORD* pQualityLevels) {
	return m_pD3Dint->CheckDeviceMultiSampleType(Adapter, DeviceType, SurfaceFormat, Windowed, MultiSampleType, pQualityLevels);
}

HRESULT APIENTRY hkIDirect3D9::CheckDeviceType(UINT Adapter, D3DDEVTYPE CheckType, D3DFORMAT DisplayFormat, D3DFORMAT BackBufferFormat, BOOL Windowed) {
	return m_pD3Dint->CheckDeviceType(Adapter, CheckType, DisplayFormat, BackBufferFormat, Windowed);
}

HRESULT APIENTRY hkIDirect3D9::CreateDevice(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS *pPresentationParameters, IDirect3DDevice9 **ppReturnedDeviceInterface) {
	SDLOG(0, "CreateDevice ------ Adapter %u\n", Adapter);
	if(!pPresentationParameters) {
		HRESULT hRet = m_pD3Dint->CreateDevice(Settings::get().getD3DAdapterOverride() >= 0 ? Adapter : Settings::get().getD3DAdapterOverride(), DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);
		if(SUCCEEDED(hRet)) hkIDirect3DDevice9 *ret = new hkIDirect3DDevice9(ppReturnedDeviceInterface, pPresentationParameters, this);
		return hRet;
	}

	D3DPRESENT_PARAMETERS adjusted = RSManager::get().adjustPresentationParameters(pPresentationParameters);
	HRESULT hRet;
	if(Settings::get().getD3DAdapterOverride() >= 0) {
		SDLOG(0, " - Adapter override to %d\n", Settings::get().getD3DAdapterOverride());
		Adapter = Settings::get().getD3DAdapterOverride();
	}
	//if(Settings::get().getEnableTripleBuffering()) {
	//	D3DDISPLAYMODEEX modeEx;
	//	D3DDISPLAYMODEEX *pModeEx = NULL;
	//	if(!adjusted.Windowed) {
	//		pModeEx = &modeEx;
	//		modeEx.Size = sizeof(D3DDISPLAYMODEEX);
	//		modeEx.Format = adjusted.BackBufferFormat;
	//		modeEx.Height = adjusted.BackBufferHeight;
	//		modeEx.Width = adjusted.BackBufferWidth;
	//		modeEx.RefreshRate = adjusted.FullScreen_RefreshRateInHz;
	//		modeEx.ScanLineOrdering = D3DSCANLINEORDERING_PROGRESSIVE;
	//	}
	//	hRet = ((IDirect3D9Ex*)m_pD3Dint)->CreateDeviceEx(Adapter, DeviceType, hFocusWindow, BehaviorFlags, &adjusted, pModeEx, (IDirect3DDevice9Ex**)ppReturnedDeviceInterface);
	//} else {
		hRet = m_pD3Dint->CreateDevice(Adapter, DeviceType, hFocusWindow, BehaviorFlags, &adjusted, ppReturnedDeviceInterface);
	//}
	if(SUCCEEDED(hRet)) {
		hkIDirect3DDevice9 *ret = new hkIDirect3DDevice9(ppReturnedDeviceInterface, &adjusted, this);
	}
	return hRet;
}

HRESULT APIENTRY hkIDirect3D9::EnumAdapterModes(UINT Adapter,D3DFORMAT Format,UINT Mode,D3DDISPLAYMODE* pMode) {
	return m_pD3Dint->EnumAdapterModes(Adapter, Format, Mode, pMode);
}

UINT APIENTRY hkIDirect3D9::GetAdapterCount() {
	return m_pD3Dint->GetAdapterCount();
}

HRESULT APIENTRY hkIDirect3D9::GetAdapterDisplayMode(UINT Adapter, D3DDISPLAYMODE *pMode) {
	return m_pD3Dint->GetAdapterDisplayMode(Adapter, pMode);
}

HRESULT APIENTRY hkIDirect3D9::GetAdapterIdentifier(UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER9 *pIdentifier) {
	return m_pD3Dint->GetAdapterIdentifier(Adapter, Flags, pIdentifier);
}

UINT APIENTRY hkIDirect3D9::GetAdapterModeCount(UINT Adapter,D3DFORMAT Format) {
	return m_pD3Dint->GetAdapterModeCount(Adapter, Format);
}

HMONITOR APIENTRY hkIDirect3D9::GetAdapterMonitor(UINT Adapter) {
	return m_pD3Dint->GetAdapterMonitor(Adapter);
}

HRESULT APIENTRY hkIDirect3D9::GetDeviceCaps(UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS9 *pCaps) {
	return m_pD3Dint->GetDeviceCaps(Adapter, DeviceType, pCaps);
}

HRESULT APIENTRY hkIDirect3D9::RegisterSoftwareDevice(void *pInitializeFunction) {
	return m_pD3Dint->RegisterSoftwareDevice(pInitializeFunction);
}

ULONG APIENTRY hkIDirect3D9::Release() {
	return m_pD3Dint->Release();
}