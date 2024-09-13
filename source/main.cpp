#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <sstream>

#include <3ds.h>
#include <citro2d.h>

#include "defines.h"
#include "widgets/feed.h"

bool toggleConsole = false;

int main(int argc, char **argv) {
    gfxInitDefault();

    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();

    if (toggleConsole) {
        consoleInit(GFX_TOP, NULL);
    }

    C3D_RenderTarget* top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
    C3D_RenderTarget* bottom = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

    float scrollY = 0.0f;
    const float SCROLL_SPEED = 5.0f;

    const u32 bgColor = C2D_Color32(22, 30, 39, 255);
    const u32 lineColor = C2D_Color32(47, 64, 81, 255);

    Feed feed = Feed(0.5f);

    touchPosition prevTouch;
    prevTouch.px = 0;
    prevTouch.py = 0;
    
    while (aptMainLoop()) {
        hidScanInput();
		if (hidKeysDown() & KEY_START) break;

        touchPosition touchPos;
        hidTouchRead(&touchPos);

        if (touchPos.px != 0 && touchPos.py != 0) {
            if (prevTouch.px != 0 && prevTouch.py != 0) {
                int16_t deltaTouch = (int16_t)touchPos.py - (int16_t)prevTouch.py;
                scrollY += deltaTouch;
            }
            hidTouchRead(&prevTouch);
        } else {
            prevTouch.px = 0;
            prevTouch.py = 0;
        }

        if (feed.get_total_height() > SCREEN_HEIGHT) {
            circlePosition pos;
            hidCircleRead(&pos);

            if (pos.dy > 10 || pos.dy < -10)
                scrollY += ((float)pos.dy / 150.0f) * SCROLL_SPEED;
            else {
                if (hidKeysHeld() & KEY_DOWN) {
                    scrollY -= SCROLL_SPEED;
                }
                if (hidKeysHeld() & KEY_UP) {
                    scrollY += SCROLL_SPEED;
                }
            }

            if (scrollY > 0.0) {
                scrollY = 0.0;
            }

            if (scrollY < -feed.get_total_height() + SCREEN_HEIGHT) {
                scrollY = -feed.get_total_height() + SCREEN_HEIGHT;
            }
        }

        if (hidKeysDown() & KEY_L) {
            toggleConsole = !toggleConsole;
            if (toggleConsole) {
                consoleInit(GFX_TOP, NULL);
            }
        }

        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        
        if (!toggleConsole) {
            C2D_SceneBegin(top);
            C2D_TargetClear(top, bgColor);

            // Drawing on the top screen

            feed.draw(SCREEN_TOP_BOTTOM_DIFF, scrollY + SCREEN_HEIGHT);

            C2D_DrawLine(SCREEN_TOP_BOTTOM_DIFF-0.5, 0.0, lineColor, SCREEN_TOP_BOTTOM_DIFF-0.5, SCREEN_HEIGHT, lineColor, 1.0, 0.0);
            C2D_DrawLine(SCREEN_TOP_BOTTOM_DIFF + BOTTOM_SCREEN_WIDTH, 0.0, lineColor, SCREEN_TOP_BOTTOM_DIFF + BOTTOM_SCREEN_WIDTH, SCREEN_HEIGHT, lineColor, 1.0, 0.0);
        }

        C2D_SceneBegin(bottom);
        C2D_TargetClear(bottom, bgColor);

        // Drawing on the bottom screen

        feed.draw(0.0f, scrollY);

		C3D_FrameEnd(0);
    }
    
    C2D_Fini();
	C3D_Fini();
	gfxExit();
}