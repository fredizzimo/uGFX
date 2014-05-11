#include "gfx.h"

static GListener    gl;
static GHandle      ghFrame1;

static void createWidgets(void) {
    GWidgetInit wi;

    // Apply some default values for GWIN
    gwinWidgetClearInit(&wi);
    wi.g.show = TRUE;

    // Apply the frame parameters    
    wi.g.width = 400;
    wi.g.height = 300;
    wi.g.y = 10;
    wi.g.x = 10;
    wi.text = "Frame 1";

    ghFrame1 = gwinFrameCreate(0, &wi, GWIN_FRAME_BORDER | GWIN_FRAME_CLOSE_BTN | GWIN_FRAME_MINMAX_BTN);
}

int main(void) {
    // Initialize the display
    gfxInit();

    // Attach the mouse input
    gwinAttachMouse(0);

    // Set the widget defaults
    gwinSetDefaultFont(gdispOpenFont("*"));
    gwinSetDefaultStyle(&WhiteWidgetStyle, FALSE);
    gdispClear(White);

    // create the widget
    createWidgets();

    // We want to listen for widget events
    geventListenerInit(&gl);
    gwinAttachListener(&gl);

    while(1) {
    	gfxSleepMilliseconds(1000);
    }

    return 0;
}
