#pragma once

#include "ofxCore.h"
#include "../shared/coldefs.h"

template <class PIX, int nComponents, int max>
class ColorFiller
{
public:
	ColorFiller()
	{}
	FillSubArea(PIX* imgArea, unsigned int iWidth, unsigned int iHeight, OfxRectI window, RGBAValue color)
	{
		for (size_t y = window.y1; y < window.y2; ++y)
		{
			PIX* l_dest = imgArea + (y * iWidth * nComponents) + (window.x1 * nComponents);
			for (size_t x = window.x1; x < window.x2; ++x)
			{
				if (max == 1)
				{
					l_dest[0] = (PIX) color.r;
					l_dest[1] = (PIX) color.g;
					l_dest[2] = (PIX) color.b;
					l_dest[3] = (PIX) color.a;
					l_dest += nComponents;
				}
				else
				{
					l_dest[0] = (PIX) (color.r * max);
					l_dest[1] = (PIX) (color.g * max);
					l_dest[2] = (PIX) (color.b * max);
					l_dest[3] = (PIX) (color.a * max);
					l_dest += nComponents;
				}
			}
		}
	}


};

