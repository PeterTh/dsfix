#include <windows.h>

#include "d3d9.h"
#include "main.h"
#include "Settings.h"

namespace {
	/* Direct3DCreate9 seems to be called twice when the game first launches, and then again every single time 
	   a video plays. For some reason, Steam's GameOverlayRenderer.dll (which performs its own hooking of
	   Direct3DCreate9 among other things) crashes when videos play. We only need the hook when the game first
	   launches anyway, so only create hkIDirect3D9s twice to work around that crash. */
	int hkDirect3DCreate9CallCount = 0;
}

IDirect3D9 *APIENTRY hkDirect3DCreate9(UINT SDKVersion) {
	IDirect3D9 *d3dint = NULL;
	//if(Settings::get().getEnableTripleBuffering()) Direct3DCreate9Ex(SDKVersion, (IDirect3D9Ex**)&d3dint);
	//else d3dint = oDirect3DCreate9(SDKVersion);
	d3dint = oDirect3DCreate9(SDKVersion);

	if(d3dint != NULL && hkDirect3DCreate9CallCount < 2) {
		hkIDirect3D9 *ret = new hkIDirect3D9(&d3dint);
		++hkDirect3DCreate9CallCount;
	}
	return d3dint;
}
