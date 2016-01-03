#pragma once

#include "d3d9.h"

TCHAR* D3DFormatToString(D3DFORMAT format, bool bWithPrefix = true);
TCHAR* D3DSamplerStateTypeToString(D3DSAMPLERSTATETYPE state);
TCHAR* D3DDeclTypeToString(D3DDECLTYPE type);
TCHAR* D3DDeclUsageToString(D3DDECLUSAGE type);

// not thread safe
TCHAR* RectToString(const RECT* rect);
TCHAR* D3DMatrixToString(const D3DMATRIX* pMatrix);
