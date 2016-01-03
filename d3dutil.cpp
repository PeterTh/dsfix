#include "d3dutil.h"

#include <cstdio>

TCHAR* RectToString(const RECT* rect) {
	const int BUFFERS = 64;
	static int cBuffer = 0;
	static TCHAR buffers[BUFFERS][64];
	if(!rect) return "NULL_RECT";
	sprintf_s(buffers[cBuffer], 64, "RECT[%4ld/%4ld/%4ld/%4ld]", rect->left, rect->top, rect->right, rect->bottom);
	TCHAR* ret = buffers[cBuffer];
	cBuffer++;
	cBuffer %= BUFFERS;
	return ret;
}

TCHAR* D3DMatrixToString(const D3DMATRIX* pMatrix) {
	const int BUFFERS = 64;
	static int cBuffer = 0;
	static TCHAR buffers[BUFFERS][256];
	if(!pMatrix) return "NULL_MATRIX";
	char* pos = buffers[cBuffer];
	for(int i=0; i<4; ++i) {
		pos += sprintf(pos, " | ");
		for(int j=0; j<4; ++j) {
			pos += sprintf(pos, "%6.2f ", pMatrix->m[i][j]);
		}
		pos += sprintf(pos, "\n");
	}
	TCHAR* ret = buffers[cBuffer];
	cBuffer++;
	cBuffer %= BUFFERS;
	return buffers[cBuffer];
}


TCHAR* D3DFormatToString(D3DFORMAT format, bool bWithPrefix) {
    TCHAR* pstr = NULL;
    switch(format) {
		case D3DFMT_UNKNOWN: pstr = TEXT("D3DFMT_UNKNOWN"); break;
		case D3DFMT_R8G8B8: pstr = TEXT("D3DFMT_R8G8B8"); break;
		case D3DFMT_A8R8G8B8: pstr = TEXT("D3DFMT_A8R8G8B8"); break;
		case D3DFMT_X8R8G8B8: pstr = TEXT("D3DFMT_X8R8G8B8"); break;
		case D3DFMT_R5G6B5: pstr = TEXT("D3DFMT_R5G6B5"); break;
		case D3DFMT_X1R5G5B5: pstr = TEXT("D3DFMT_X1R5G5B5"); break;
		case D3DFMT_A1R5G5B5: pstr = TEXT("D3DFMT_A1R5G5B5"); break;
		case D3DFMT_A4R4G4B4: pstr = TEXT("D3DFMT_A4R4G4B4"); break;
		case D3DFMT_R3G3B2: pstr = TEXT("D3DFMT_R3G3B2"); break;
		case D3DFMT_A8: pstr = TEXT("D3DFMT_A8"); break;
		case D3DFMT_A8R3G3B2: pstr = TEXT("D3DFMT_A8R3G3B2"); break;
		case D3DFMT_X4R4G4B4: pstr = TEXT("D3DFMT_X4R4G4B4"); break;
		case D3DFMT_A2B10G10R10: pstr = TEXT("D3DFMT_A2B10G10R10"); break;
		case D3DFMT_A8B8G8R8: pstr = TEXT("D3DFMT_A8B8G8R8"); break;
		case D3DFMT_X8B8G8R8: pstr = TEXT("D3DFMT_X8B8G8R8"); break;
		case D3DFMT_G16R16: pstr = TEXT("D3DFMT_G16R16"); break;
		case D3DFMT_A2R10G10B10: pstr = TEXT("D3DFMT_A2R10G10B10"); break;
		case D3DFMT_A16B16G16R16: pstr = TEXT("D3DFMT_A16B16G16R16"); break;
		case D3DFMT_A8P8: pstr = TEXT("D3DFMT_A8P8"); break;
		case D3DFMT_P8: pstr = TEXT("D3DFMT_P8"); break;
		case D3DFMT_L8: pstr = TEXT("D3DFMT_L8"); break;
		case D3DFMT_A8L8: pstr = TEXT("D3DFMT_A8L8"); break;
		case D3DFMT_A4L4: pstr = TEXT("D3DFMT_A4L4"); break;
		case D3DFMT_V8U8: pstr = TEXT("D3DFMT_V8U8"); break;
		case D3DFMT_L6V5U5: pstr = TEXT("D3DFMT_L6V5U5"); break;
		case D3DFMT_X8L8V8U8: pstr = TEXT("D3DFMT_X8L8V8U8"); break;
		case D3DFMT_Q8W8V8U8: pstr = TEXT("D3DFMT_Q8W8V8U8"); break;
		case D3DFMT_V16U16: pstr = TEXT("D3DFMT_V16U16"); break;
		case D3DFMT_A2W10V10U10: pstr = TEXT("D3DFMT_A2W10V10U10"); break;
		case D3DFMT_UYVY: pstr = TEXT("D3DFMT_UYVY"); break;
		case D3DFMT_R8G8_B8G8: pstr = TEXT("D3DFMT_R8G8_B8G8"); break;
		case D3DFMT_YUY2: pstr = TEXT("D3DFMT_YUY2"); break;
		case D3DFMT_G8R8_G8B8: pstr = TEXT("D3DFMT_G8R8_G8B8"); break;
		case D3DFMT_DXT1: pstr = TEXT("D3DFMT_DXT1"); break;
		case D3DFMT_DXT2: pstr = TEXT("D3DFMT_DXT2"); break;
		case D3DFMT_DXT3: pstr = TEXT("D3DFMT_DXT3"); break;
		case D3DFMT_DXT4: pstr = TEXT("D3DFMT_DXT4"); break;
		case D3DFMT_DXT5: pstr = TEXT("D3DFMT_DXT5"); break;
		case D3DFMT_D16_LOCKABLE: pstr = TEXT("D3DFMT_D16_LOCKABLE"); break;
		case D3DFMT_D32: pstr = TEXT("D3DFMT_D32"); break;
		case D3DFMT_D15S1: pstr = TEXT("D3DFMT_D15S1"); break;
		case D3DFMT_D24S8: pstr = TEXT("D3DFMT_D24S8"); break;
		case D3DFMT_D24X8: pstr = TEXT("D3DFMT_D24X8"); break;
		case D3DFMT_D24X4S4: pstr = TEXT("D3DFMT_D24X4S4"); break;
		case D3DFMT_D16: pstr = TEXT("D3DFMT_D16"); break;
		case D3DFMT_D32F_LOCKABLE: pstr = TEXT("D3DFMT_D32F_LOCKABLE"); break;
		case D3DFMT_D24FS8: pstr = TEXT("D3DFMT_D24FS8"); break;
		case D3DFMT_D32_LOCKABLE: pstr = TEXT("D3DFMT_D32_LOCKABLE"); break;
		case D3DFMT_S8_LOCKABLE: pstr = TEXT("D3DFMT_S8_LOCKABLE"); break;
		case D3DFMT_L16: pstr = TEXT("D3DFMT_L16"); break;
		case D3DFMT_VERTEXDATA: pstr = TEXT("D3DFMT_VERTEXDATA"); break;
		case D3DFMT_INDEX16: pstr = TEXT("D3DFMT_INDEX16"); break;
		case D3DFMT_INDEX32: pstr = TEXT("D3DFMT_INDEX32"); break;
		case D3DFMT_Q16W16V16U16: pstr = TEXT("D3DFMT_Q16W16V16U16"); break;
		case D3DFMT_MULTI2_ARGB8: pstr = TEXT("D3DFMT_MULTI2_ARGB8"); break;
		case D3DFMT_R16F: pstr = TEXT("D3DFMT_R16F"); break;
		case D3DFMT_G16R16F: pstr = TEXT("D3DFMT_G16R16F"); break;
		case D3DFMT_A16B16G16R16F: pstr = TEXT("D3DFMT_A16B16G16R16F"); break;
		case D3DFMT_R32F: pstr = TEXT("D3DFMT_R32F"); break;
		case D3DFMT_G32R32F: pstr = TEXT("D3DFMT_G32R32F"); break;
		case D3DFMT_A32B32G32R32F: pstr = TEXT("D3DFMT_A32B32G32R32F"); break;
		case D3DFMT_CxV8U8: pstr = TEXT("D3DFMT_CxV8U8"); break;
		case D3DFMT_A1: pstr = TEXT("D3DFMT_A1"); break;
		case D3DFMT_A2B10G10R10_XR_BIAS: pstr = TEXT("D3DFMT_A2B10G10R10_XR_BIAS"); break;
		case D3DFMT_BINARYBUFFER: pstr = TEXT("D3DFMT_BINARYBUFFER"); break;
		default:                     pstr = TEXT("       Unknown format"); break;
    }
    if( bWithPrefix )
        return pstr;
    else
        return pstr + lstrlen( TEXT("D3DFMT_") );
}

TCHAR* D3DSamplerStateTypeToString(D3DSAMPLERSTATETYPE state) {
    switch(state) {
		case D3DSAMP_ADDRESSU: return "D3DSAMP_ADDRESSU";
		case D3DSAMP_ADDRESSV: return "D3DSAMP_ADDRESSV";
		case D3DSAMP_ADDRESSW: return "D3DSAMP_ADDRESSW";
		case D3DSAMP_BORDERCOLOR: return "D3DSAMP_BORDERCOLOR";
		case D3DSAMP_MAGFILTER: return "D3DSAMP_MAGFILTER";
		case D3DSAMP_MINFILTER: return "D3DSAMP_MINFILTER";
		case D3DSAMP_MIPFILTER: return "D3DSAMP_MIPFILTER";
		case D3DSAMP_MIPMAPLODBIAS: return "D3DSAMP_MIPMAPLODBIAS";
		case D3DSAMP_MAXMIPLEVEL: return "D3DSAMP_MAXMIPLEVEL";
		case D3DSAMP_MAXANISOTROPY: return "D3DSAMP_MAXANISOTROPY";
		case D3DSAMP_SRGBTEXTURE: return "D3DSAMP_SRGBTEXTURE";
		case D3DSAMP_ELEMENTINDEX: return "D3DSAMP_ELEMENTINDEX";
		case D3DSAMP_DMAPOFFSET: return "D3DSAMP_DMAPOFFSET";
		case D3DSAMP_FORCE_DWORD: return "D3DSAMP_FORCE_DWORD";
	}
	return "Unknown Sampler State Type";
}

TCHAR* D3DDeclTypeToString(D3DDECLTYPE type) {
	switch(type) {
		case D3DDECLTYPE_FLOAT1: return "D3DDECLTYPE_FLOAT1";
		case D3DDECLTYPE_FLOAT2: return "D3DDECLTYPE_FLOAT2";
		case D3DDECLTYPE_FLOAT3: return "D3DDECLTYPE_FLOAT3";
		case D3DDECLTYPE_FLOAT4: return "D3DDECLTYPE_FLOAT4";
		case D3DDECLTYPE_D3DCOLOR: return "D3DDECLTYPE_D3DCOLOR";
		case D3DDECLTYPE_UBYTE4: return "D3DDECLTYPE_UBYTE4";
		case D3DDECLTYPE_SHORT2: return "D3DDECLTYPE_SHORT2";
		case D3DDECLTYPE_SHORT4: return "D3DDECLTYPE_SHORT4";
		case D3DDECLTYPE_UBYTE4N: return "D3DDECLTYPE_UBYTE4N";
		case D3DDECLTYPE_SHORT2N: return "D3DDECLTYPE_SHORT2N";
		case D3DDECLTYPE_SHORT4N: return "D3DDECLTYPE_SHORT4N";
		case D3DDECLTYPE_USHORT2N: return "D3DDECLTYPE_USHORT2N";
		case D3DDECLTYPE_USHORT4N: return "D3DDECLTYPE_USHORT4N";
		case D3DDECLTYPE_UDEC3: return "D3DDECLTYPE_UDEC3";
		case D3DDECLTYPE_DEC3N: return "D3DDECLTYPE_DEC3N";
		case D3DDECLTYPE_FLOAT16_2: return "D3DDECLTYPE_FLOAT16_2";
		case D3DDECLTYPE_FLOAT16_4: return "D3DDECLTYPE_FLOAT16_4";
		case D3DDECLTYPE_UNUSED: return "D3DDECLTYPE_UNUSED";
	}
	return "Unknown D3D decl type";
}

TCHAR* D3DDeclUsageToString(D3DDECLUSAGE type) {
	switch(type) {
		case D3DDECLUSAGE_POSITION: return "D3DDECLUSAGE_POSITION";
		case D3DDECLUSAGE_BLENDWEIGHT: return "D3DDECLUSAGE_BLENDWEIGHT";
		case D3DDECLUSAGE_BLENDINDICES: return "D3DDECLUSAGE_BLENDINDICES";
		case D3DDECLUSAGE_NORMAL: return "D3DDECLUSAGE_NORMAL";
		case D3DDECLUSAGE_PSIZE: return "D3DDECLUSAGE_PSIZE";
		case D3DDECLUSAGE_TEXCOORD: return "D3DDECLUSAGE_TEXCOORD";
		case D3DDECLUSAGE_TANGENT: return "D3DDECLUSAGE_TANGENT";
		case D3DDECLUSAGE_BINORMAL: return "D3DDECLUSAGE_BINORMAL";
		case D3DDECLUSAGE_TESSFACTOR: return "D3DDECLUSAGE_TESSFACTOR";
		case D3DDECLUSAGE_POSITIONT: return "D3DDECLUSAGE_POSITIONT";
		case D3DDECLUSAGE_COLOR: return "D3DDECLUSAGE_COLOR";
		case D3DDECLUSAGE_FOG: return "D3DDECLUSAGE_FOG";
		case D3DDECLUSAGE_DEPTH: return "D3DDECLUSAGE_DEPTH";
		case D3DDECLUSAGE_SAMPLE: return "D3DDECLUSAGE_SAMPLE";
	}
	return "Unknown D3D decl usage";
}
