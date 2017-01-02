#pragma once

#include "d3d9.h"

interface hkIDirect3DQuery9 : public IDirect3DQuery9
{
	hkIDirect3DQuery9(IDirect3DQuery9 **ppReturnedQueryInterface);

	IDirect3DQuery9* m_pD3Dquery;

	STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);
	STDMETHOD(GetDevice)(THIS_ IDirect3DDevice9** ppDevice);
	STDMETHOD_(D3DQUERYTYPE, GetType)(THIS);
	STDMETHOD_(DWORD, GetDataSize)(THIS);
	STDMETHOD(Issue)(THIS_ DWORD dwIssueFlags);
	STDMETHOD(GetData)(THIS_ void* pData, DWORD dwSize, DWORD dwGetDataFlags);
};