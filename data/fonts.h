// don't include _all_ fonts, to reduce size
#ifdef PICOWOTA

#include "../mcufont/fonts/fixed_5x8.c"

#else // PICOWOTA

#include "../mcufont/fonts/DejaVuSerif16.c"
#include "../mcufont/fonts/DejaVuSerif32.c"
#include "../mcufont/fonts/fixed_10x20.c"

#endif // PICOWOTA
