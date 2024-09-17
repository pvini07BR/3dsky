#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <malloc.h>

#include <3ds.h>
#include <citro2d.h>

#include <curl/curl.h>

#include "defines.h"
#include "curl_funcs.h"
#include "widgets/feed.h"

bool toggleConsole = false;

int main() {
    gfxInitDefault();
    if (toggleConsole) {
        consoleInit(GFX_TOP, NULL);
    }

    Result ret = 0;
    u32 soc_sharedmem_size = 0x100000;
    u32 *soc_sharedmem = (u32*)memalign(0x1000, soc_sharedmem_size);
    if (soc_sharedmem == NULL) {
        printf("Failed to allocate SOC sharedmem.\n");
        return -1;
    } else {
        ret = socInit(soc_sharedmem, soc_sharedmem_size);

        if (R_FAILED(ret)) {
            printf("socInit failed: 0x%08x.\n", (unsigned int)ret);
            return -1;
        } else {
            printf("SOC succesfully initialized.\n");
        }
    }

    curl_global_init(CURL_GLOBAL_DEFAULT);

    printf("Fetching posts from Discover feed...\n");

    Feed feed = Feed(0.5f);

    PostFetching pf;
    pf.posts = &feed.posts;
	svcCreateEvent(&pf.eventHandle, ResetType::RESET_ONESHOT);

    s32 prio = 0;
    svcGetThreadPriority(&prio, CUR_THREAD_HANDLE);
    Thread thread = threadCreate(get_posts, (void*)&pf, 4 * 1024, prio-1, -2, false);

    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();

    C3D_RenderTarget* top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
    C3D_RenderTarget* bottom = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

    float scrollY = 0.0f;
    const float SCROLL_SPEED = 5.0f;

    const u32 bgColor = C2D_Color32(22, 30, 39, 255);
    const u32 lineColor = C2D_Color32(47, 64, 81, 255);

    touchPosition prevTouch;
    prevTouch.px = 0;
    prevTouch.py = 0;

    C2D_TextBuf textBuf = C2D_TextBufNew(17);

    C2D_Text loadingText;
    C2D_TextParse(&loadingText, textBuf, "Loading posts...");
    C2D_TextOptimize(&loadingText);

    bool finishedLoading = false;
    
    int frames = 0;
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

            if (!finishedLoading) {
                C2D_DrawText(&loadingText, C2D_WithColor | C2D_AlignCenter | C2D_AtBaseline , (float)TOP_SCREEN_WIDTH/2.0f, (float)SCREEN_HEIGHT/2.0f, 0.0f, 1.0f, 1.0f, C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF));
            }

            C2D_DrawLine(SCREEN_TOP_BOTTOM_DIFF-0.5, 0.0, lineColor, SCREEN_TOP_BOTTOM_DIFF-0.5, SCREEN_HEIGHT, lineColor, 1.0, 0.0);
            C2D_DrawLine(SCREEN_TOP_BOTTOM_DIFF + BOTTOM_SCREEN_WIDTH, 0.0, lineColor, SCREEN_TOP_BOTTOM_DIFF + BOTTOM_SCREEN_WIDTH, SCREEN_HEIGHT, lineColor, 1.0, 0.0);
        }

        C2D_SceneBegin(bottom);
        C2D_TargetClear(bottom, bgColor);

        // Drawing on the bottom screen

        feed.draw(0.0f, scrollY);

        if (!finishedLoading) {
            C2D_DrawText(&loadingText, C2D_WithColor | C2D_AlignCenter | C2D_AtBaseline , (float)BOTTOM_SCREEN_WIDTH/2.0f, (float)SCREEN_HEIGHT/2.0f, 0.0f, 1.0f, 1.0f, C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF));
        }

		C3D_FrameEnd(0);

        if (!finishedLoading && frames == 3) {
           svcWaitSynchronization(pf.eventHandle, U64_MAX);
           svcClearEvent(pf.eventHandle);
           finishedLoading = true;
        }

        frames++;
    }

    threadJoin(thread, U64_MAX);
    svcCloseHandle(pf.eventHandle);
    
    C2D_Fini();
	C3D_Fini();
    curl_global_cleanup();
    socExit();
	gfxExit();
}
