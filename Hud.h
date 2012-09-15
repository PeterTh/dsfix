#pragma once

#include "Effect.h"
#include "main.h"

class HUD : public Effect {
public:
    HUD(IDirect3DDevice9 *device, int width, int height);
    virtual ~HUD();

	void go(IDirect3DTexture9 *input, IDirect3DSurface9 *dst);

private:
	int width, height;

	ID3DXEffect *effect;
	
	D3DXHANDLE frameTexHandle;
	D3DXHANDLE opacityHandle;

	void rect(float srcLeft, float srcTop, float srcWidth, float srcHeight,
			  float trgLeft, float trgTop, float trgWidth, float trgHeight);
};
