#ifndef TRSR_IMAGE_BLURER
#define TRSR_IMAGE_BLURER

#include "PixelHelper.h"
#include "mathfunctions.h"
#include <vector>

template <class PIX, int nComponents, int max>
class CImageBlurer: public CPixelHelperConst
{
public:
	CImageBlurer( long Widht, long Height, long Pitch, const PIX* screenAddress )
		: CPixelHelperConst( Widht, Height, Pitch, screenAddress )
	{
	    m_pLinesR = new CMedianCntDbl[ Height ];
	    m_pLinesG = new CMedianCntDbl[ Height ];
	    m_pLinesB = new CMedianCntDbl[ Height ];
	    m_pLinesA = new CMedianCntDbl[ Height ];
	    m_plastXB  = new long[ Height ];
	    m_plastXE  = new long[ Height ];
	    for (size_t i = 0; i < Height; ++ i)
	    {
	      m_plastXB[i] = - Widht;
	      m_plastXE[i] = - Widht;
	    }
	    m_lastblurX = 0;
	    m_lastblurY = 0;
	}
	~CImageBlurer(void)
	{
	  if ( m_pLinesR != NULL )
	    delete [] m_pLinesR;
	  if ( m_pLinesG != NULL )
	    delete [] m_pLinesG;
	  if ( m_pLinesB != NULL )
	    delete [] m_pLinesB;
	  if ( m_pLinesA != NULL )
	    delete [] m_pLinesA;
	  if ( m_plastXB != NULL )
	    delete [] m_plastXB;
	  if ( m_plastXE != NULL )
	    delete [] m_plastXE;
      // MessageBox(0,"destr", " ", 0);
	}

	inline RGBAValue GetBlurredImagePixel( const long& x, const long& y, const int& blurX, const int& blurY )
	{
		RGBAValue l_out;
		if ( ( x < 0 ) || ( x >= m_imgWidth ) || ( y < 0 ) || ( y > m_imgHeight ) )
			return l_out;
		if ( m_lastblurX != blurX )
		{
			//char l_test[256];
			//sprintf(l_test, "m_lastblurX: %i blurX: %i ", m_lastblurX, blurX );
			//MessageBox(0,l_test, " ", 0);
		  for (size_t i = 0; i < m_imgHeight; ++ i)
		  {
		    m_plastXB[i] = - m_imgWidth;
		    m_plastXE[i] = - m_imgWidth;
		    m_pLinesR[i].Reset();
		    m_pLinesG[i].Reset();
		    m_pLinesB[i].Reset();
		    m_pLinesA[i].Reset();
		  }
    		m_lastblurX = blurX;
		}

		//calculate new begin and end x
		long l_begX = x - blurX / 2;
		long l_endX = l_begX + blurX;

		CMedianCntDbl l_r;
		CMedianCntDbl l_g;
		CMedianCntDbl l_b;
		CMedianCntDbl l_a;

		for (long ly = 0; ly < blurY; ++ ly)
		{
		    size_t l_sY = ( y + ly - blurY / 2 );
			if ( ( l_sY < 0 ) || ( l_sY >= m_imgHeight ) )
				continue;
			CMedianCntDbl* l_tr = &m_pLinesR[l_sY];
			CMedianCntDbl* l_tg = &m_pLinesG[l_sY];
			CMedianCntDbl* l_tb = &m_pLinesB[l_sY];
			CMedianCntDbl* l_ta = &m_pLinesA[l_sY];
			long*     l_pLastXB = &m_plastXB[l_sY];
			long*     l_pLastXE = &m_plastXE[l_sY];

//			ULONG l_prevMed = l_tb->GetMedian();

			//char l_test[256];
			//sprintf(l_test, "dx: %i x: %i lx: %i", dx, x, *l_pLastX);
			//MessageBox(0,l_test, " ", 0);

			if ( ( *l_pLastXB < l_begX ) && ( *l_pLastXE < l_endX ) && ( *l_pLastXE > l_begX ) )
			{
				// optimized differential
				for (long lx = *l_pLastXB; lx < l_begX; ++ lx)
				{
					RGBAValue l_v = GetPixel( lx , l_sY ) ;
					  l_tr->PopVal( l_v.r );
					  l_tg->PopVal( l_v.g );
					  l_tb->PopVal( l_v.b );
					  l_ta->PopVal( l_v.a );
				}
				for (long lx = *l_pLastXE; lx < l_endX; ++ lx)
				{
					RGBAValue l_v = GetPixel( lx , l_sY  ) ;
					l_tr->PutVal( l_v.r );
					l_tg->PutVal( l_v.g );
					l_tb->PutVal( l_v.b );
					l_ta->PutVal( l_v.a );
				}
			}
			else
			{
				// non optimized, normal 
				l_tr->Reset();
				l_tg->Reset();
				l_tb->Reset();
				l_ta->Reset();
				for (long lx = l_begX; lx < l_endX; ++ lx)
				{
					RGBAValue l_v = GetPixel( lx , l_sY  ) ;
					l_tr->PutVal( l_v.r );
					l_tg->PutVal( l_v.g );
					l_tb->PutVal( l_v.b );
					l_ta->PutVal( l_v.a );
				}
			}

			m_plastXB[l_sY] = l_begX;  // update last x value
			m_plastXE[l_sY] = l_endX;  // update last x value
		    // and sum medians
		    l_r.AppendCounter( l_tr->GetSum(), l_tr->GetCnt() );
		    l_g.AppendCounter( l_tg->GetSum(), l_tg->GetCnt() );
		    l_b.AppendCounter( l_tb->GetSum(), l_tb->GetCnt() );
		    l_a.AppendCounter( l_ta->GetSum(), l_ta->GetCnt() );
		}
		//bytePixel l_out(0,0,0,0);
		l_out.r = l_r.GetMedian();
		l_out.g = l_g.GetMedian();
		l_out.b = l_b.GetMedian();
		l_out.a = l_a.GetMedian();

		return ( l_out );
	}


	inline RGBAValue GetStarBlurredImagePixel( const long& x, const long& y, const int& blurX, const int& blurY )
	{
		CMedianCntDbl l_r;
		CMedianCntDbl l_g;
		CMedianCntDbl l_b;
		CMedianCntDbl l_a;
		long ly = 0;
		long lx = 0;
		for (; ly < blurY; ++ ly)
		{
			RGBAValue l_v = GetPixel( ( x + lx - blurX / 2 ) , ( y + ly - blurY / 2 )  ) ;
			l_r.PutVal( l_v.r );
			l_g.PutVal( l_v.g );
			l_b.PutVal( l_v.b );
			l_a.PutVal( l_v.a );
		}
		for (; lx < blurX; ++ lx)
		{
			RGBAValue l_v = GetPixel( ( x + lx - blurX / 2 ) , ( y + ly - blurY / 2 )  ) ;
			l_r.PutVal( l_v.r );
			l_g.PutVal( l_v.g );
			l_b.PutVal( l_v.b );
			l_a.PutVal( l_v.a );
		}
		bytePixel l_out;
		l_out.r = l_r.GetMedian();
		l_out.g = l_g.GetMedian();
		l_out.b = l_b.GetMedian();
		l_out.a = l_a.GetMedian();
		return ( l_out );
	}


private:
	CMedianCntDbl* m_pLinesR;
	CMedianCntDbl* m_pLinesG;
	CMedianCntDbl* m_pLinesB;
	CMedianCntDbl* m_pLinesA;

	long* m_plastXB;
	long* m_plastXE;
	int   m_lastblurX;
	int   m_lastblurY;

};

#endif   // TRSR_IMAGE_BLURER