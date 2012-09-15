#include "VSSAO.h"

#include <string>
#include <sstream>
#include <vector>
using namespace std;

#include "Settings.h"

VSSAO::VSSAO(IDirect3DDevice9 *device, int width, int height, unsigned strength) 
	: Effect(device), width(width), height(height) {
	
	// Setup the defines for compiling the effect
    vector<D3DXMACRO> defines;
    stringstream s;

    // Setup pixel size macro
    s << "float2(1.0 / " << width << ", 1.0 / " << height << ")";
    string pixelSizeText = s.str();
    D3DXMACRO pixelSizeMacro = { "PIXEL_SIZE", pixelSizeText.c_str() };
    defines.push_back(pixelSizeMacro);
	
	D3DXMACRO strengthMacros[] = {
		{ "SSAO_STRENGTH_LOW", "1" },
		{ "SSAO_STRENGTH_MEDIUM", "1" },
		{ "SSAO_STRENGTH_HIGH", "1" }
	};
	defines.push_back(strengthMacros[strength]);

    D3DXMACRO null = { NULL, NULL };
    defines.push_back(null);

	DWORD flags = D3DXFX_NOT_CLONEABLE | D3DXSHADER_OPTIMIZATION_LEVEL3;

	// Load effect from file
	SDLOG(0, "VSSAO load\n");	
	ID3DXBuffer* errors;
	HRESULT hr = D3DXCreateEffectFromFile(device, GetDirectoryFile("dsfix\\VSSAO.fx"), &defines.front(), NULL, flags, NULL, &effect, &errors);
	if(hr != D3D_OK) SDLOG(0, "ERRORS:\n %s\n", errors->GetBufferPointer());
	

	// Load thickness texture
	D3DXCreateTextureFromFile(device, GetDirectoryFile("dsfix\\vssaothicknessmodel.png"), &thicknessTex);

	// Create buffers
	device->CreateTexture(width, height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &buffer1Tex, NULL);
    buffer1Tex->GetSurfaceLevel(0, &buffer1Surf);
	device->CreateTexture(width, height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &buffer2Tex, NULL);
    buffer2Tex->GetSurfaceLevel(0, &buffer2Surf);

	// get handles
	depthTexHandle = effect->GetParameterByName(NULL, "depthTex2D");
	frameTexHandle = effect->GetParameterByName(NULL, "frameTex2D");
    prevPassTexHandle = effect->GetParameterByName(NULL, "prevPassTex2D");
    thicknessTexHandle = effect->GetParameterByName(NULL, "thicknessTex1D");
}

VSSAO::~VSSAO() {
	SAFERELEASE(effect);
	SAFERELEASE(buffer1Surf);
	SAFERELEASE(buffer1Tex);
	SAFERELEASE(buffer2Surf);
	SAFERELEASE(buffer2Tex);
	SAFERELEASE(thicknessTex);
}

void VSSAO::go(IDirect3DTexture9 *frame, IDirect3DTexture9 *depth, IDirect3DSurface9 *dst) {
	device->SetVertexDeclaration(vertexDeclaration);
	
    mainSsaoPass(depth, buffer1Surf);
	
	for(size_t i = 0; i<1; ++i) {
		hBlurPass(depth, buffer1Tex, buffer2Surf);
		vBlurPass(depth, buffer2Tex, buffer1Surf);
	}

	combinePass(frame, buffer1Tex, dst);
}

void VSSAO::mainSsaoPass(IDirect3DTexture9* depth, IDirect3DSurface9* dst) {
	device->SetRenderTarget(0, dst);
    device->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

    // Setup variables.
    effect->SetTexture(depthTexHandle, depth);
    effect->SetTexture(thicknessTexHandle, thicknessTex);

    // Do it!
    UINT passes;
	effect->Begin(&passes, 0);
	effect->BeginPass(0);
	quad(width, height);
	effect->EndPass();
	effect->End();
}

void VSSAO::hBlurPass(IDirect3DTexture9 *depth, IDirect3DTexture9* src, IDirect3DSurface9* dst) {
	device->SetRenderTarget(0, dst);

    // Setup variables.
    effect->SetTexture(prevPassTexHandle, src);
    effect->SetTexture(depthTexHandle, depth);
	
    // Do it!
    UINT passes;
	effect->Begin(&passes, 0);
	effect->BeginPass(1);
	quad(width, height);
	effect->EndPass();
	effect->End();
}

void VSSAO::vBlurPass(IDirect3DTexture9 *depth, IDirect3DTexture9* src, IDirect3DSurface9* dst) {
	device->SetRenderTarget(0, dst);

    // Setup variables.
    effect->SetTexture(prevPassTexHandle, src);
    effect->SetTexture(depthTexHandle, depth);
	
    // Do it!
    UINT passes;
	effect->Begin(&passes, 0);
	effect->BeginPass(2);
	quad(width, height);
	effect->EndPass();
	effect->End();
}

void VSSAO::combinePass(IDirect3DTexture9* frame, IDirect3DTexture9* ao, IDirect3DSurface9* dst) {
	device->SetRenderTarget(0, dst);

    // Setup variables.
    effect->SetTexture(prevPassTexHandle, ao);
    effect->SetTexture(frameTexHandle, frame);
	
    // Do it!
    UINT passes;
	effect->Begin(&passes, 0);
	effect->BeginPass(3);
	quad(width, height);
	effect->EndPass();
	effect->End();
}
