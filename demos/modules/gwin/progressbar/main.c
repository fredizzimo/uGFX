#include "gfx.h"

GHandle 	ghProgressbar;

static void _createWidget(void) {
	GWidgetInit	wi;
 
	wi.customDraw = 0;
	wi.customParam = 0;
	wi.customStyle = 0;
	wi.g.show = TRUE;
 
	wi.g.y = 10; wi.g.x = 10; wi.g.width = 200; wi.g.height = 20; wi.text = "Progress 1";
	ghProgressbar = gwinProgressbarCreate(NULL, &wi);
}

int main(void) {
	gfxInit();

	gwinSetDefaultFont(gdispOpenFont("UI2"));
	gwinSetDefaultStyle(&WhiteWidgetStyle, FALSE);
	gdispClear(White);

	_createWidget();

	gwinProgressbarSetResolution(ghProgressbar, 10);
	gwinProgressbarStart(ghProgressbar, 500);

	gfxSleepMilliseconds(3000);
	gwinProgressbarReset(ghProgressbar);

	//gwinProgressbarSetPosition(ghProgressbar, 42);
	//gwinProgressbarIncrement(ghProgressbar);
	//gwinProgressbarDecrement(ghProgressbar);

	while (1) {
		gfxSleepMilliseconds(500);
	}

	return 0;
}

