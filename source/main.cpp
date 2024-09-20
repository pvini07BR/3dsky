#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <malloc.h>
#include <optional>

#include <3ds.h>
#include <citro2d.h>

#include <curl/curl.h>
#include "asset_pool.h"

#include "defines.h"
#include "curl_funcs.h"
#include "widgets/feed.h"

bool toggleConsole = false;
bool loadedPosts = false;

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
    
    std::string feed_uri = "at://did:plc:z72i7hdynmk6r22z27h6tvur/app.bsky.feed.generator/whats-hot";
    //feed_uri = "at://did:plc:coqkaymd4t65envntucbpx2y/app.bsky.feed.generator/aaak7ypmzcdrq";

    std::string cursor = "";
    get_posts(feed_uri, "", feed.textBuf, &feed.posts, &cursor);

    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();

    C3D_RenderTarget* top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
    C3D_RenderTarget* bottom = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

    float scrollY = 0.0f;
    float scrollVelY = 0.0f;
    const float SCROLL_SPEED = 5.0f;

    const u32 bgColor = C2D_Color32(22, 30, 39, 255);
    const u32 lineColor = C2D_Color32(47, 64, 81, 255);

    touchPosition prevTouch;
    prevTouch.px = 0;
    prevTouch.py = 0;

    C2D_TextBuf textBuf = C2D_TextBufNew(15);
    C2D_Text loadingText;
    C2D_TextParse(&loadingText, textBuf, "Loading posts...");
    C2D_TextOptimize(&loadingText);

    AssetPool asset_pool = AssetPool();

    int16_t deltaTouch = 0;
    int frames = 0;
    while (aptMainLoop()) {
        hidScanInput();
		if (hidKeysDown() & KEY_START) break;

        touchPosition touchPos;
        hidTouchRead(&touchPos);

        if (touchPos.px != 0 && touchPos.py != 0) {
            if (prevTouch.px != 0 && prevTouch.py != 0) {
                deltaTouch = (int16_t)touchPos.py - (int16_t)prevTouch.py;
                scrollY += deltaTouch;
            }
            hidTouchRead(&prevTouch);
        } else {
            prevTouch.px = 0;
            prevTouch.py = 0;
            if (deltaTouch != 0) {
                scrollVelY = deltaTouch;
                deltaTouch = 0;
            }
            if (scrollVelY > 0.0f)
                scrollVelY -= 0.5f;
            if (scrollVelY < 0.0f)
                scrollVelY += 0.5f;
            scrollY += scrollVelY;
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
                scrollVelY = 0.0f;
                scrollY = 0.0;
            }

            if (scrollY < -feed.get_total_height() + SCREEN_HEIGHT) {
                if (!loadedPosts && !cursor.empty()) {
                    printf("Loading more posts...\n");
                    scrollVelY = 0.0f;
                    feed.reserve_more(50);
                    get_posts(feed_uri, cursor, feed.textBuf, &feed.posts, &cursor);
                    loadedPosts = true;
                }

                if (scrollY < (-feed.get_total_height() + SCREEN_HEIGHT) - 32.0f) {
                    scrollY = (-feed.get_total_height() + SCREEN_HEIGHT) - 32.0f;
                }
            } else {
                if (loadedPosts) {
                    loadedPosts = false;
                }
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

            feed.draw(SCREEN_TOP_BOTTOM_DIFF, scrollY + SCREEN_HEIGHT, &asset_pool);

            C2D_DrawLine(SCREEN_TOP_BOTTOM_DIFF-0.5, 0.0, lineColor, SCREEN_TOP_BOTTOM_DIFF-0.5, SCREEN_HEIGHT, lineColor, 1.0, 0.0);
            C2D_DrawLine(SCREEN_TOP_BOTTOM_DIFF + BOTTOM_SCREEN_WIDTH, 0.0, lineColor, SCREEN_TOP_BOTTOM_DIFF + BOTTOM_SCREEN_WIDTH, SCREEN_HEIGHT, lineColor, 1.0, 0.0);

            C2D_DrawRectSolid((float)TOP_SCREEN_WIDTH-5.0f, 0.0f, 0.0f, 5.0f, (float)SCREEN_HEIGHT, C2D_Color32(128, 128, 128, 128));
            C2D_DrawRectSolid((float)TOP_SCREEN_WIDTH-5.0f, -((scrollY/feed.get_total_height()) * (float)SCREEN_HEIGHT), 0.0f, 5.0f, 10.0f, C2D_Color32(128, 128, 128, 255));
        }

        C2D_SceneBegin(bottom);
        C2D_TargetClear(bottom, bgColor);

        // Drawing on the bottom screen

        feed.draw(0.0f, scrollY, &asset_pool);

		C3D_FrameEnd(0);

        frames++;
    }
    
    C2D_Fini();
	C3D_Fini();
    curl_global_cleanup();
    socExit();
	gfxExit();
}
