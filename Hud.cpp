#include "Hud.h"

#include <string>
#include <sstream>
#include <vector>
using namespace std;

#include "Settings.h"

HUD::HUD(IDirect3DDevice9 *device, int width, int height) 
	: Effect(device), width(width), height(height) {

	DWORD flags = D3DXFX_NOT_CLONEABLE;

	// Load effect from file
	SDLOG(0, "Hud Effect load\n");
	ID3DXBuffer* errors;
	HRESULT hr = D3DXCreateEffectFromFile(device, GetDirectoryFile("dsfix\\HUD.fx"), NULL, NULL, flags, NULL, &effect, &errors);
	if(hr != D3D_OK) SDLOG(0, "ERRORS:\n %s\n", errors->GetBufferPointer());
		
	// get handles
	frameTexHandle = effect->GetParameterByName(NULL, "frameTex2D");
	opacityHandle = effect->GetParameterByName(NULL, "opacity");
}

HUD::~HUD() {
	SAFERELEASE(effect);
}

void HUD::go(IDirect3DTexture9 *input, IDirect3DSurface9 *dst) {
	device->SetVertexDeclaration(vertexDeclaration);
	device->SetRenderTarget(0, dst);
    effect->SetTexture(frameTexHandle, input);

	float scale = Settings::get().getHudScaleFactor();
	float iscale = 1.0f-scale;
	
    UINT passes;

	// upper left
	effect->SetFloat(opacityHandle, Settings::get().getHudTopLeftOpacity());
	effect->Begin(&passes, 0);
	effect->BeginPass(0);
	rect(0.0f, 0.0f, 1.0f, 0.21f, 
		 0.0f, 0.0f, 1.0f*scale, 0.21f*scale); 
	effect->EndPass();
	effect->End();

	// lower left
	effect->SetFloat(opacityHandle, Settings::get().getHudBottomLeftOpacity());
	effect->Begin(&passes, 0);
	effect->BeginPass(0);
	if(Settings::get().getEnableMinimalHud()) {
		rect(0.145f, 0.527f, 0.074f, 0.204f,
			 0.1f*scale, 0.77f + 0.2f*iscale, 0.074f*scale, 0.204f*scale); 
		rect(0.145f, 0.731f, 0.074f, 0.204f,
			 0.1f*scale + 0.074f*scale + 0.01f, 0.77f + 0.2f*iscale, 0.074f*scale, 0.204f*scale); 
	} else {
		rect(0.0f, 0.5f, 0.5f, 0.5f, 
			 0.0f, 0.5f + 0.5f*iscale, 0.5f*scale, 0.5f*scale); 
	}
	effect->EndPass();
	effect->End();

	// lower right
	effect->SetFloat(opacityHandle, Settings::get().getHudBottomRightOpacity());
	effect->Begin(&passes, 0);
	effect->BeginPass(0);
	rect(0.8f, 0.8f, 0.2f, 0.2f, 
		 0.8f + 0.2f*iscale, 0.8f + 0.2f*iscale, 0.2f*scale, 0.2f*scale); 
	effect->EndPass();
	effect->End();

	// center
	effect->SetFloat(opacityHandle, 1.0f);
	effect->Begin(&passes, 0);
	effect->BeginPass(0);
	rect(0.37f, 0.22f, 0.4f, 0.5f, 
		 0.37f + 0.15f*iscale, 0.22f + 0.15f*iscale, 0.4f*scale, 0.5f*scale); 
	effect->EndPass();
	effect->End();

}

void HUD::rect(float srcLeft, float srcTop, float srcWidth, float srcHeight,
			   float trgLeft, float trgTop, float trgWidth, float trgHeight) {
	trgTop = -(trgTop*2.0f-1.0f);
	trgLeft = trgLeft*2.0f-1.0f;
	float trgRight = trgLeft + trgWidth*2.0f;
	float trgBottom = trgTop - trgHeight*2.0f;
	float srcRight = srcLeft + srcWidth;
	float srcBottom = srcTop + srcHeight;
	float quad[4][5] = {
			{ trgLeft,  trgTop, 0.5f, srcLeft,  srcTop    },
			{ trgRight, trgTop, 0.5f, srcRight, srcTop    },
			{ trgLeft,     trgBottom, 0.5f, srcLeft,  srcBottom },
			{ trgRight,    trgBottom, 0.5f, srcRight, srcBottom }
	};
	device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, quad, sizeof(quad[0]));
}
