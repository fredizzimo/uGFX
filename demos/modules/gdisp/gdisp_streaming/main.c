/*
 * Copyright (c) 2012, 2013, Joel Bodenmann aka Tectu <joel@unormal.org>
 * Copyright (c) 2012, 2013, Andrew Hannam aka inmarket
 * Derived from the 2011 IOCCC submission by peter.eastman@gmail.com
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *    * Neither the name of the <organization> nor the
 *      names of its contributors may be used to endorse or promote products
 *      derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "gfx.h"
#include <math.h>

#define Lightgrey ()
#define Midgrey ()
#define Darkgrey (HTML2COLOR(0x303030))

#define BALLCOLOR1	Red
#define BALLCOLOR2	Yellow
#define WALLCOLOR	HTML2COLOR(0x303030)
#define BACKCOLOR	HTML2COLOR(0xC0C0C0)
#define FLOORCOLOR	HTML2COLOR(0x606060)
#define SHADOW		(255-255*0.2)

int main(void) {
	coord_t		width, height, x, y, i, l, m, n, floor;
	color_t		colour;
	float		ii, spin, o, spinspeed, h, f, g;

	gfxInit();
	gdispSetOrientation(GDISP_ROTATE_90);

	width = gdispGetWidth();
	height = gdispGetHeight();

	i=height/5+height%2+1;
	ii = 1.0/i;
	floor=height/5-1;
	spin=0.0;
	l=width/2;
	m=height/4;
	n=.01*width;
	o=0.0;
	spinspeed=0.1;

	while(1) {
		// Draw one frame
		gdispStreamStart(0, 0, width, height);
		for (y=0; h = (m-y)*ii, y<height; y++) {
			for (x=0; x < width; x++) {
				g=(l-x)*ii;
				f=-.3*g+.954*h;
				if (g*g < 1-h*h) {
					/* The inside of the ball */
					if ((((int)(9-spin+(.954*g+.3*h)*invsqrt(1-f*f))+(int)(2+f*2))&1))
						colour = BALLCOLOR1;
					else
						colour = BALLCOLOR2;
				} else {
					// The background (walls and floor)
					if (y > height-floor) {
						if (x < height-y || height-y > width-x)
							colour = WALLCOLOR;
						else
							colour = FLOORCOLOR;
					} else if (x<floor || x>width-floor)
						colour = WALLCOLOR;
					else
						colour = BACKCOLOR;

					// The ball shadow is darker
					if (g*(g+.4)+h*(h+.1) < 1)
						colour = gdispBlendColor(colour, Black, SHADOW);
				}
				gdispStreamColor(colour);	/* pixel to the LCD */
			}
		}
		gdispStreamStop();

		// Motion
		spin += spinspeed;
		m += o;
		o = m > height-1.75*floor ? -.04*height : o+.002*height;
		n = (l+=n)<i || l>width-i ? spinspeed=-spinspeed,-n : n;
	}
}
