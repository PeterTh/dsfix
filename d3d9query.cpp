#include "d3d9.h"
#include "main.h"
#include "RenderstateManager.h"
#include "Settings.h"

hkIDirect3DQuery9::hkIDirect3DQuery9(IDirect3DQuery9 **ppReturnedQueryInterface) {
	m_pD3Dquery = *ppReturnedQueryInterface;
	*ppReturnedQueryInterface = this;
}

HRESULT APIENTRY hkIDirect3DQuery9::QueryInterface(REFIID riid, void** ppvObj) {
	return m_pD3Dquery->QueryInterface(riid, ppvObj);
}

ULONG APIENTRY hkIDirect3DQuery9::AddRef() {
	return m_pD3Dquery->AddRef();
}

ULONG APIENTRY hkIDirect3DQuery9::Release() {
	return m_pD3Dquery->Release();
}

HRESULT APIENTRY hkIDirect3DQuery9::GetDevice(IDirect3DDevice9** ppDevice) {
	return m_pD3Dquery->GetDevice(ppDevice);
}

D3DQUERYTYPE APIENTRY hkIDirect3DQuery9::GetType() {
	return m_pD3Dquery->GetType();
}

DWORD APIENTRY hkIDirect3DQuery9::GetDataSize() {
	return m_pD3Dquery->GetDataSize();
}

HRESULT APIENTRY hkIDirect3DQuery9::Issue(DWORD dwIssueFlags) {
	return m_pD3Dquery->Issue(dwIssueFlags);
}

HRESULT APIENTRY hkIDirect3DQuery9::GetData(void* pData, DWORD dwSize, DWORD dwGetDataFlags) {
	auto result = m_pD3Dquery->GetData(pData, dwSize, dwGetDataFlags);
	if (SUCCEEDED(result)) {
		auto pixelsDrawn = reinterpret_cast<DWORD*>(pData);
		pixelsDrawn[0] = static_cast<DWORD>(pixelsDrawn[0] / RSManager::get().getOcclusionScale());
	}
	return result;
}