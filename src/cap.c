#include <cap.h>
#include <logdef.h>
#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/cursorfont.h>
#include <unistd.h> // added for sleep/usleep

// original from [https://bbs.archlinux.org/viewtopic.php?id=85378 Select a screen area with mouse and return the geometry of this area? / Programming & Scripting / Arch Linux Forums]
// build with (Ubuntu 14.04):
// gcc -Wall xrectsel.c -o xrectsel -lX11

int cap_display_size(int* w, int* h) {
	if (w && h) {
	} else {
		LOGW("bad paramers");
	}
	return 0;
}
