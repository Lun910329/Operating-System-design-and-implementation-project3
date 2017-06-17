//
// inodeBitmap Walker (c) Yating Zhou (aka syscl)
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
//
// define kDEBUG, 1 = turn on
//              , 0 = turn off(default) *
//
#define kDEBUG 1

#if kDEBUG
#define DEBUG_PREFIX "Debug: "
#define DBG(arg...) do { printf(DEBUG_PREFIX arg); } while(0)
#else
#define DBG(arg...) do {} while(0)
#endif


int main(int argc, char **argv)
{
	DBG("InodeBitmapWalker ===>\n");
	zonewalker();
	DBG("InodeBitmapWalker <===\n");
	return 0;
}
