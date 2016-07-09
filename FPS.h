// Dark Souls FPS fix by Clement Barnier (Nwks)
///////////////////////////////////////////////

#pragma once

void initFPSTimer();
void applyFPSPatch();

// returns time since startup in milliseconds
double getElapsedTime(void);
