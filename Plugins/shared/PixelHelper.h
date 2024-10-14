#ifndef TRSR_PIXEL_HELPER
#define TRSR_PIXEL_HELPER

#include "ofxCore.h"
#include "mathfunctions.h"
#include "coldefs.h"

template <class PIX, int nComponents, int max>
class CPixelHelper
{
public:
	CPixelHelper( long Widht, long Height, long Pitch, PIX* screenAddress )
	{
		m_imgWidth       = Widht;
		m_imgHeight      = Height;
		m_imgHorPith     = Pitch;
		m_pScreenAddress = screenAddress;
	}
	~CPixelHelper(void)
	{
	}

inline const PIX* GetPixelPtr(const long x, const long y)
{
	if ( (x < 0) || (y < 0) || (x >= m_imgWidth) || (y >= m_imgHeight) )
		return NULL;
	size_t out_addr = ((y * m_imgHorPith) + x) * nComponents;
	return &m_pScreenAddress[ out_addr ];
}

inline const RGBAValue GetPixel(const long x, const long y)
{
	RGBAValue out;
	if ( (x < 0) || (y < 0) || (x >= m_imgWidth) || (y >= m_imgHeight) )
	{
		
		return (out);
	}
	size_t out_addr = ( ( y * m_imgHorPith ) + x ) * nComponents;

	if (max == 1)
	{
		out.r = (double)(m_pScreenAddress[out_addr]);
		out.g = (double)(m_pScreenAddress[out_addr+1]);
		out.b = (double)(m_pScreenAddress[out_addr+2]);
		out.a = (double)(m_pScreenAddress[out_addr+3]);
	}
	else
	{
		out.r = (double)(m_pScreenAddress[out_addr]) / (double)max;
		out.g = (double)(m_pScreenAddress[out_addr+1]) / (double)max;
		out.b = (double)(m_pScreenAddress[out_addr+2]) / (double)max;
		out.a = (double)(m_pScreenAddress[out_addr+3]) / (double)max;
	}
	return ( m_pScreenAddress[ out_addr ] );
}

void PutPixel(const long x, const long y, RGBAValue Pixel )
{
	if ( (x < 0) || (y < 0) || (x >= m_imgWidth) || (y >= m_imgHeight) )
		return;
	size_t out_addr = ((y * m_imgHorPith) + x) * nComponents;
	if (max == 1)
	{
		m_pScreenAddress[out_addr] = Pixel.r;
		m_pScreenAddress[out_addr+1] = (PIX)(Pixel.g);
		m_pScreenAddress[out_addr+2] = (PIX)(Pixel.b);
		m_pScreenAddress[out_addr+3] = (PIX)(Pixel.a);
	}
	else
	{
		m_pScreenAddress[out_addr] = (PIX)(Pixel.r * max);
		m_pScreenAddress[out_addr + 1] = (PIX)(Pixel.g * max);
		m_pScreenAddress[out_addr + 2] = (PIX)(Pixel.b * max);
		m_pScreenAddress[out_addr + 3] = (PIX)(Pixel.a * max);
	}
	//m_pScreenAddress[ out_addr ] = Pixel;
}

protected:
	long m_imgWidth;
	long m_imgHeight;
	long m_imgHorPith;
	PIX* m_pScreenAddress;
};


template <class PIX, int nComponents, int max>
class CPixelHelperConst
{
public:
	CPixelHelperConst( long Widht, long Height, long Pitch, const PIX* screenAddress )
	{
		m_imgWidth       = Widht;
		m_imgHeight      = Height;
		m_imgHorPith     = Pitch;
		m_pScreenAddress = screenAddress;
		m_pScreenLineAddress = screenAddress;
		m_nLastIMGLine   = 0;
	}
	~CPixelHelperConst(void)
	{
	}

inline const PIX* GetPixelPtr(const long x, const long y)
{
	if ( (x < 0) || (y < 0) || (x >= m_imgWidth) || (y >= m_imgHeight) )
		return NULL;
	if ( m_nLastIMGLine != y )
	{
		m_pScreenLineAddress = m_pScreenAddress + y * (m_imgHorPith * nComponents);
		m_nLastIMGLine = y;
	}
//	size_t out_addr = ( y * m_imgHorPith ) + x;
//	return &m_pScreenAddress[ out_addr ];
	return ( &m_pScreenLineAddress[ x * nComponents] );
}

inline const RGBAValue GetPixel(const long x, const long y)
{
	RGBAValue out;
	if ( (x < 0) || (y < 0) || (x >= m_imgWidth) || (y >= m_imgHeight) )
	{
		return (out);
	}
	if ( m_nLastIMGLine != y )
	{
		m_pScreenLineAddress = m_pScreenAddress + y * (m_imgHorPith * nComponents);
		m_nLastIMGLine = y;
	}

	size_t out_addr = m_pScreenLineAddress + (x * nComponent);
	if (max == 1)
	{
		out.r = (double)(m_pScreenAddress[out_addr]);
		out.g = (double)(m_pScreenAddress[out_addr + 1]);
		out.b = (double)(m_pScreenAddress[out_addr + 2]);
		out.a = (double)(m_pScreenAddress[out_addr + 3]);
	}
	else
	{
		out.r = (double)(m_pScreenAddress[out_addr]) / (double)max;
		out.g = (double)(m_pScreenAddress[out_addr + 1]) / (double)max;
		out.b = (double)(m_pScreenAddress[out_addr + 2]) / (double)max;
		out.a = (double)(m_pScreenAddress[out_addr + 3]) / (double)max;
	}


	return ( out );
}


inline const RGBAValue GetPixelFloat(const float x, const float y)
{
	RGBAValue out;
	if ( (x < 0) || (y < 0) || (x >= m_imgWidth) || (y >= m_imgHeight) )
	{
		return (out);
	}

	long l_ix = (long)x;
	long l_iy = (long)y;
	float l_dx = x - (float)l_ix;
	float l_dy = y - (float)l_iy;

	if ( ( l_dx == 0.0f ) && ( l_dy == 0.0f ) )
		return ( GetPixel( l_ix, l_iy ) );

	RGBAValue l_col0 = GetPixel( l_ix, l_iy );
	RGBAValue l_col1 = GetPixel( l_ix + 1 , l_iy );
	RGBAValue l_col2 = GetPixel( l_ix + 1, l_iy + 1 );
	RGBAValue l_col3 = GetPixel( l_ix, l_iy + 1 );

	RGBAValue l_hor1 = interpolate2Colors( l_col0, l_col1, l_dx );
	RGBAValue l_hor2 = interpolate2Colors( l_col3, l_col2, l_dx );

	RGBAValue l_out = interpolate2Colors( l_hor1, l_hor2, l_dy );
	return ( l_out );
}


protected:
	long m_imgWidth;
	long m_imgHeight;
	long m_imgHorPith;
	const PIX* m_pScreenAddress;
private:
	const PIX* m_pScreenLineAddress;
	long m_nLastIMGLine;
};

#endif   // TRSR_PIXEL_HELPER