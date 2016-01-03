#include "GAUSS.h"

#include <string>
#include <sstream>
#include <vector>
using namespace std;

#include "Settings.h"

GAUSS::GAUSS(IDirect3DDevice9 *device, int width, int height) 
	: Effect(device), width(width), height(height) {
	SDLOG(0, "Gauss construct\n");
	// Setup the defines for compiling the effect
    vector<D3DXMACRO> defines;
    stringstream s;

    // Setup pixel size macro
    s << "float2(1.0 / " << width << ", 1.0 / " << height << ")";
    string pixelSizeText = s.str();
    D3DXMACRO pixelSizeMacro = { "PIXEL_SIZE", pixelSizeText.c_str() };
    defines.push_back(pixelSizeMacro);

    D3DXMACRO null = { NULL, NULL };
    defines.push_back(null);

	DWORD flags = D3DXFX_NOT_CLONEABLE;

	// Load effect from file
	SDLOG(0, "Gauss load\n");
	ID3DXBuffer* errors;
	HRESULT hr = D3DXCreateEffectFromFile(device, GetDirectoryFile("dsfix\\GAUSS.fx"), &defines.front(), NULL, flags, NULL, &effect, &errors);
	if(hr != D3D_OK) SDLOG(0, "ERRORS:\n %s\n", errors->GetBufferPointer());

	// Create buffers
	device->CreateTexture(width, height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &buffer1Tex, NULL);
    buffer1Tex->GetSurfaceLevel(0, &buffer1Surf);
	
	// get handles
	frameTexHandle = effect->GetParameterByName(NULL, "frameTex2D");
}

GAUSS::~GAUSS() {
	SAFERELEASE(effect);
	SAFERELEASE(buffer1Surf);
	SAFERELEASE(buffer1Tex);
}

void GAUSS::go(IDirect3DTexture9 *input, IDirect3DSurface9 *dst) {
	device->SetVertexDeclaration(vertexDeclaration);
	
    UINT passes;

    // Horizontal blur
	device->SetRenderTarget(0, buffer1Surf);
    effect->SetTexture(frameTexHandle, input);
	effect->Begin(&passes, 0);
	effect->BeginPass(0);
	quad(width, height);
	effect->EndPass();
	effect->End();

    // Vertical blur
	device->SetRenderTarget(0, dst);
	effect->SetTexture(frameTexHandle, buffer1Tex);
	effect->Begin(&passes, 0);
	effect->BeginPass(1);
	quad(width, height);
	effect->EndPass();
	effect->End();
}
