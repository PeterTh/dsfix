#pragma once

#include <map>
#include <set>
#include <vector>

#include "d3d9.h"
#include "SMAA.h"
#include "FXAA.h"
#include "SSAO.h"
#include "GAUSS.h"
#include "HUD.h"

class RSManager {
	static RSManager instance;

	bool inited;
	D3DVIEWPORT9 viewport;
	IDirect3DDevice9 *d3ddev;

	double lastPresentTime;
	bool lowFPSmode;

	bool doAA;
	SMAA* smaa;
	FXAA* fxaa;
	
	bool doSsao;
	SSAO* ssao;

	bool doDofGauss;
	GAUSS* gauss;
	
	bool doHud;
	HUD* hud;

	IDirect3DTexture9* rgbaBuffer1Tex;
	IDirect3DSurface9* rgbaBuffer1Surf;
	IDirect3DSurface9* depthStencilSurf;
	
	IDirect3DSurface9* zSurf;

	bool hideHud;
	bool onHudRT, pausedHudRT;

	bool paused;
	
	std::set<int> dumpedTextures;

	unsigned texIndex, mainRenderTexIndex, mainRenderSurfIndex;
	typedef std::map<IDirect3DTexture9*, int> TexIntMap;
	TexIntMap texIndices, mainRenderTexIndices;
	typedef std::map<IDirect3DSurface9*, int> SurfIntMap;
	SurfIntMap mainRenderSurfIndices;

	bool captureNextFrame, capturing, hudStarted, takeScreenshot;
	unsigned dumpCaptureIndex;

	void dumpSurface(const char* name, IDirect3DSurface9* surface);

	#define TEXTURE(_name, _hash) \
	private: \
	static const UINT32 texture##_name##Hash = _hash; \
	IDirect3DTexture9* texture##_name; \
	bool isTexture##_name(IDirect3DBaseTexture9* pTexture) { return texture##_name && ((IDirect3DTexture9*)pTexture) == texture##_name; };
	#include "Textures.def"
	#undef TEXTURE
	bool isTextureText(IDirect3DBaseTexture9* pTexture);
	const char* getTextureName(IDirect3DBaseTexture9* pTexture);

	unsigned numKnownTextures, foundKnownTextures;
	unsigned skippedPresents;

	// RenderDoneDetectionProgress
	// basically, when the game switches rendertargets after setting texture 0 to 3 in order, but no others, 
	// we assume we just finished rendering the hud-less image. This variable keeps track of the number of "correct" texture settings.
	unsigned rddp;

	// NumRenderTargetSwitches
	// we use the number of switches between rendertargets to figure out where we are in the pipeline. Yeah, it's flaky
	unsigned nrts;

	// Count the number of times the 2 upper DoF rendertargets were set in doft[1] and doft[2]
	unsigned doft[3];
	unsigned isDof(unsigned width, unsigned height);

	// HudDoneDetectionProgress
	// sequence: 5xHudHealthbar, 2-3xCategoryIconsSoulCount, followed by any other texture signals end of normal Hud drawing
	// TODO: handle cursed
	unsigned hddp;

	// main rendertarget for this frame
	IDirect3DSurface9* mainRT;
	unsigned mainRTuses;

	void registerKnowTexture(LPCVOID pSrcData, UINT SrcDataSize, LPDIRECT3DTEXTURE9 pTexture);
	IDirect3DTexture9* getSurfTexture(IDirect3DSurface9* pSurface);

	// Render state store/restore
	void storeRenderState();
	void restoreRenderState();
	IDirect3DVertexDeclaration9* prevVDecl;
	IDirect3DSurface9* prevDepthStencilSurf;
	IDirect3DSurface9* prevRenderTarget;
	IDirect3DTexture9* prevRenderTex;
	IDirect3DStateBlock9* prevStateBlock;

    struct MemData
    {
        char* buffer;
        long size;
    };
    
    std::map<UINT32, MemData> cachedTexFiles;

	bool disassembleShader(CONST DWORD *pFunction, LPD3DXBUFFER *ppBuffer);
	void dumpShader(UINT32 hash, const char *directory, LPD3DXBUFFER pBuffer);
	bool getOverrideShader(UINT32 hash, const char *directory, LPD3DXBUFFER *ppBuffer);

private:
    ~RSManager();

public:
	static RSManager& get() {
		return instance;
	}

	RSManager() : smaa(NULL), fxaa(NULL), ssao(NULL), gauss(NULL), rgbaBuffer1Surf(NULL), rgbaBuffer1Tex(NULL),
			inited(false), doAA(true), doSsao(true), doDofGauss(true), doHud(true), captureNextFrame(false), capturing(false), hudStarted(false), takeScreenshot(false), hideHud(false),
			mainRenderTexIndex(0), mainRenderSurfIndex(0), dumpCaptureIndex(0), numKnownTextures(0), foundKnownTextures(0), skippedPresents(0) {
		#define TEXTURE(_name, _hash) ++numKnownTextures;
		#include "Textures.def"
		#undef TEXTURE
	}
	
	void setD3DDevice(IDirect3DDevice9 *pD3Ddev) { d3ddev = pD3Ddev; }

	void initResources();
	void releaseResources();
    void prefetchTextures();

	void setViewport(const D3DVIEWPORT9& vp) {
		viewport = vp;
	}

	bool isViewport(const RECT& r) {
		return (r.left == viewport.X) && (r.top == viewport.Y) && (r.bottom == viewport.Height) && (r.right == viewport.Width);
	}

	D3DPRESENT_PARAMETERS adjustPresentationParameters(const D3DPRESENT_PARAMETERS *pPresentationParameters);
	void enableSingleFrameCapture();
	void enableTakeScreenshot();
	bool takingScreenshot() { return takeScreenshot; }
	void toggleAA() { doAA = !doAA; }
	void toggleVssao() { doSsao = !doSsao; }
	void toggleHideHud() { hideHud = !hideHud; }
	void toggleChangeHud() { doHud = !doHud; }
	void toggleDofGauss() { doDofGauss = !doDofGauss; }
	void reloadVssao();
	void reloadHbao();
	void reloadScao();
	void reloadGauss();
	void reloadAA();

	bool allowStateChanges() { return !onHudRT; }

	INT16 hudVertices[32];
	void reloadHudVertices();
	
	void registerMainRenderTexture(IDirect3DTexture9* pTexture);
	void registerMainRenderSurface(IDirect3DSurface9* pSurface);
	unsigned getTextureIndex(IDirect3DTexture9* ppTexture);
	void registerD3DXCreateTextureFromFileInMemory(LPCVOID pSrcData, UINT SrcDataSize, LPDIRECT3DTEXTURE9 pTexture);
	void registerD3DXCompileShader(LPCSTR pSrcData, UINT srcDataLen, const D3DXMACRO *pDefines, LPD3DXINCLUDE pInclude, LPCSTR pFunctionName, LPCSTR pProfile, DWORD Flags, LPD3DXBUFFER * ppShader, LPD3DXBUFFER * ppErrorMsgs, LPD3DXCONSTANTTABLE * ppConstantTable);
	
	HRESULT redirectSetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget);

	void finishHudRendering();
	void pauseHudRendering();
	void resumeHudRendering();

	HRESULT redirectStretchRect(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestSurface, CONST RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter);
	HRESULT redirectSetTexture(DWORD Stage, IDirect3DBaseTexture9 * pTexture);
	HRESULT redirectSetDepthStencilSurface(IDirect3DSurface9* pNewZStencil);
	HRESULT redirectPresent(CONST RECT * pSourceRect, CONST RECT * pDestRect, HWND hDestWindowOverride, CONST RGNDATA * pDirtyRegion);

	void frameTimeManagement();
	void togglePaused() { paused = !paused; }
	bool isPaused() { return paused; }

	HRESULT redirectDrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinIndex, UINT NumVertices, UINT PrimitiveCount, CONST void* pIndexData, D3DFORMAT IndexDataFormat, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride);
	HRESULT redirectDrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride);
	HRESULT redirectD3DXCreateTextureFromFileInMemoryEx(LPDIRECT3DDEVICE9 pDevice, LPCVOID pSrcData, UINT SrcDataSize, UINT Width, UINT Height, UINT MipLevels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, DWORD Filter, DWORD MipFilter, D3DCOLOR ColorKey, D3DXIMAGE_INFO* pSrcInfo, PALETTEENTRY* pPalette, LPDIRECT3DTEXTURE9* ppTexture);
	HRESULT redirectSetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value);
	HRESULT redirectSetRenderState(D3DRENDERSTATETYPE State, DWORD Value);
	HRESULT redirectCreatePixelShader(CONST DWORD *pfunction, IDirect3DPixelShader9 **ppShader);
	HRESULT redirectCreateVertexShader(CONST DWORD *pfunction, IDirect3DVertexShader9 **ppShader);
};
