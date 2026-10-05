#include "common.h"

#include "records.h"
#include "cdload.h"
#include "overlay.h"
#include "gamestate.h"
#include "gfx.h"
#include "heap.h"
#include "main.h"
#include "memcard.h"
#include "message.h"
#include "object.h"
#include "pad.h"
#include "sound.h"
#include "psyq/libcd.h"
#include "psyq/libetc.h"
#include "psyq/libgpu.h"
#include "psyq/libgs.h"
#include "psyq/libgte.h"
#include "psyq/libsnd.h"

/* .sdata (this file is -G8). main reaches main_boot_rect and main_object through $gp; records_60hz, which
 * it reads with %hi/%lo, is defined in another file. */
s32 main_screen_pos = 1;                           /* screen position flag (one of the NTSC patch's two bytes) */
RECT main_boot_rect = { 0, 0, 320, 480 };         /* where the boot screen goes */
Object *main_object = NULL;              /* the current object */

/* Boots the hardware, shows the boot screen, initializes every module, then loops forever: runs the
 * current object (heap_objects.try_run returns it, or NULL once it has ended) and ends the frame. With
 * no object, it resets the gfx, the object list, the heap's state-2 blocks and gamestate, and
 * overlay_create_object creates a new one. */
int main(void) {
    RECT rect;
    GsIMAGE image;
    u8 param[8];

    if (records_60hz != 0) {
        SetVideoMode(MODE_NTSC);
    } else {
        SetVideoMode(MODE_PAL);
    }
    ResetCallback();
    VSync(0);
    SetDispMask(0);
    ResetGraph(0);
    gfx_module.reset();
    gfx_module.funcs.set_vsync_callback();
    rect.x = 0;
    rect.y = 0;
    rect.w = 640;
    rect.h = 511;
    ClearImage(&rect, 0, 0, 0);
    DrawSync(0);
    GsInitGraph(320, 240, 1, 1, 0);
    GsInit3D();
    SsInit();
    InitGeom();
    gfx_module.funcs.init_display(320, 640, 1, 0);
    PutDispEnv(&gfx_module.dispenvs[0]);
    VSync(0);
    GsGetTimInfo(main_file_base + 1, &image);
    VSync(0);
    LoadImage(&main_boot_rect, image.pixel);
    DrawSync(0);
    VSync(0);
    SetDispMask(1);
    CdInit();
    CdSetDebug(0);
    SetGraphDebug(0);
    param[0] = 0x80;
    while (CdControl(0xE, param, 0) == 0) {
        PLATFORM_WAIT();
    }
    VSync(3);
    CdControlB(9, 0, 0);
    heap_funcs.init();
    sound_module.init();
    pad_random.seed(0);
    memcard_funcs.init();
    pad_state.init(0, 0x12);
    gamestate_data.funcs.new_game();
    message_module.load_font();
    for (;;) {
        if (main_object == NULL) {
            gfx_module.funcs.free_packet_buffers();
            gfx_module.reset();
            heap_objects.clear();
            heap_funcs.free_state(2);
            gamestate_data.funcs.change_map();
            main_object = overlay_create_object();
        }
        main_object = heap_objects.try_run(main_object);
        gfx_module.funcs.end_frame(PTR_TO_S32(main_object));
        pad_state.update();
        pad_random.next();
        cdload_module.update();
        sound_module.update_loading();
    }
}

/* .rodata: a const this small would go to .sdata at -G8, so the PS1 build names the section. */
#ifndef PC_PORT
#define MAIN_RODATA __attribute__((section(".rodata")))
#else
#define MAIN_RODATA
#endif

u8 *const main_overlay_base MAIN_RODATA = SLOT_PTR(1, u8 *, 0x80082CB0);

u32 *const main_file_base MAIN_RODATA = SLOT_PTR(2, u32 *, 0x800A5DE0);
