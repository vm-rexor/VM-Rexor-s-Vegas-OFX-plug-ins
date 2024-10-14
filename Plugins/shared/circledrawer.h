#pragma once

#include "../shared/coldefs.h"
#include "../shared/mathfunctions.h"
#include "ofxCore.h"


template <class PIX, int nComponents, int max>
class CCircleDrawer
{
public:
	CCircleDrawer(void)
	{
	}

	void Draw(long X, long Y, float Size, unsigned int sWidth, unsigned int sHeight, PIX* BegPtr, OfxRectI window, RGBAValue plotColor)
	{
		float squaredR = Size * Size;
		float l_radial = 2.0f;
		
		if (BegPtr == NULL)
			return;

		for (long l_y = Y - Size; l_y < (Y + Size); ++l_y)
		{
			for (long l_x = X - Size; l_x < (X + Size); ++l_x)
			{
				long l_dx = X - l_x;
				long l_dy = Y - l_y;
				float squaredRp = (float)(l_dx * l_dx + l_dy * l_dy);
				if ((squaredR - squaredRp) > l_radial)
				{
					PlotPoint(l_x, l_y, sWidth, sHeight, window, BegPtr, plotColor, 1.0f);
				}
				else
				{
					if ((squaredR - squaredRp) > 0)
					{
						float l_alpha = ((squaredR - squaredRp) / l_radial);
						PlotPoint(l_x, l_y, sWidth, sHeight, window, BegPtr, plotColor, l_alpha);
					}
				}
			}
		}
	}
	// x = 0 i y = 0 to punkt centralny obrazu
	void PlotPointCenter(long X, long Y, unsigned int sWidth, unsigned int sHeight, OfxRectI& window, PIX* BegPtr, RGBAValue& plotColor, float fAlpha)
	{
		PlotPoint((X + sWidth / 2), (Y + sHeight / 2), sWidth, sHeight, window, BegPtr, plotColor, fAlpha);
	}

	void PlotPoint(long X, long Y, unsigned int sWidth, unsigned int sHeight, OfxRectI& window, PIX* BegPtr, RGBAValue& plotColor, float fAlpha)
	{
		if ((X < 0) || (Y < 0) || (X >= sWidth) || (Y >= sHeight) || (X < window.x1) || (X >= window.x2) || (Y < window.y1) || (Y >= window.y2))
			return;
		{
			PIX* _dst = BegPtr + (sWidth * Y * nComponents) + (X * nComponents);
			if (fAlpha == 1.0f)
			{
				if (max == 1)
				{
					_dst[0] = (PIX)plotColor.r;
					_dst[1] = (PIX)plotColor.g;
					_dst[2] = (PIX)plotColor.b;
					_dst[3] = (PIX)plotColor.a;
				}
				else
				{
					_dst[0] = (PIX)(plotColor.r * max);
					_dst[1] = (PIX)(plotColor.g * max);
					_dst[2] = (PIX)(plotColor.b * max);
					_dst[3] = (PIX)(plotColor.a * max);
				}
			}
			else
			{
				if (max == 1)
				{
					_dst[0] = (PIX)InterpolateBetween2Vals((double)_dst[0], plotColor.r, fAlpha);
					_dst[1] = (PIX)InterpolateBetween2Vals((double)_dst[1], plotColor.g, fAlpha);
					_dst[2] = (PIX)InterpolateBetween2Vals((double)_dst[2], plotColor.b, fAlpha);
					_dst[3] = (PIX)InterpolateBetween2Vals((double)_dst[3], plotColor.a, fAlpha);
				}
				else
				{
					_dst[0] = (PIX)(InterpolateBetween2Vals((double)_dst[0], plotColor.r, fAlpha) * max);
					_dst[1] = (PIX)(InterpolateBetween2Vals((double)_dst[1], plotColor.g, fAlpha) * max);
					_dst[2] = (PIX)(InterpolateBetween2Vals((double)_dst[2], plotColor.b, fAlpha) * max);
					_dst[3] = (PIX)(InterpolateBetween2Vals((double)_dst[3], plotColor.a, fAlpha) * max);

				}
			}
		}
	}



};
