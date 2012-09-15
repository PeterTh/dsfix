#include <windows.h>

#include "d3d9.h"
#include "main.h"

IDirect3D9 *APIENTRY hkDirect3DCreate9(UINT SDKVersion) {
	IDirect3D9 *d3dint = oDirect3DCreate9(SDKVersion);

	if(d3dint != NULL) {
		hkIDirect3D9 *ret = new hkIDirect3D9(&d3dint);
	}
	return d3dint;
}
