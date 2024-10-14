#ifndef TRSR_MATHFCTIONS
#define TRSR_MATHFCTIONS
#include <math.h>



typedef union tbtw
{
	WORD w;
	struct tb
	{
		BYTE l;
		BYTE h;
	}b;
}btw;


typedef union twtd
{
	DWORD D;
	struct tw
	{
		WORD l;
		WORD h;
	}w;
}wtd;


typedef struct tSFloatPixel
{
	float Red;
	float Green;
	float Blue;
	float Alpha;
} SFloatPixel;

typedef struct tsPixel
{
	BYTE r;
	BYTE g;
	BYTE b;
	BYTE a;
} bPixel;

#define NMAX_VEC_CAP 32

class CMedianCounter
{
public:
	CMedianCounter()
	{
		m_iValSum = 0;
		m_iValCnt = 0;
	};
	BYTE GetMedian()
	{
		if (m_iValCnt > 0)
		{
			return ( (BYTE)( m_iValSum / m_iValCnt )  );
		}
		else
			return 0 ;
	};
	void Reject(BYTE rate)
	{
		tbtw med;
		med.b.h = 0;
		med.b.l = ( (BYTE)( m_iValSum / m_iValCnt ) );
		//BYTE mediana = ( (BYTE)( m_iValSum / m_iValCnt ) );
		BYTE diffcnt = 0;
		int rrate = 255 - rate;
		for (size_t x = 0; x < m_iValCnt; ++x)
		{
			if ( abs( med.w - m_values[x] ) > rrate )
			{
				m_iValSum -= m_values[x];
				diffcnt ++;
				if (diffcnt == ( m_iValCnt -1 ) )
					break;
			}
		}
		m_iValCnt -= diffcnt;

	}

	void RejectMax(BYTE count)
	{
		BYTE mediana = ( (BYTE)( m_iValSum / m_iValCnt ) );
		BYTE diffcnt = 0;
		BYTE tcount;
		if (m_iValCnt <= count)
			tcount = m_iValCnt -1;
		else
			tcount = count;
		for (size_t y = 0; y < tcount; ++y)
		{
			//get max val
			BYTE max0 = 0;
			size_t max_p = 0;
			for (size_t x = 0; x < m_iValCnt; ++x)
			{
				BYTE outVl;
				if ( mediana > m_values[x] )
					outVl = mediana > m_values[x];
				else
					outVl = m_values[x] - mediana;
				if ( outVl > max0 )
				{
					max0 = m_values[x];
					max_p = x;
				}
			}
			
			// eliminate max val
			m_iValSum -= (ULONG)max0;
			for (size_t x = max_p; x < m_iValCnt; ++x)
			{
				m_values[x] = m_values[x+1];
			}
			m_iValCnt --;
			
		}
	}

	void PutVal(BYTE v)
	{
		if ( m_iValCnt < NMAX_VEC_CAP )
		{
			m_iValSum += (ULONG)v;
			m_values[m_iValCnt] = v;
			m_iValCnt ++;
		}
	}

private:
	ULONG m_iValSum;
	size_t m_iValCnt;
	BYTE  m_values[NMAX_VEC_CAP];



};


class CMedianCnt
{
public:
	CMedianCnt()
	{
		m_iValSum = 0;
		m_iValCnt = 0;
	};
	/// add value to counter
	void PutVal(BYTE v)
	{
		m_iValSum += (ULONG)v;
		m_iValCnt ++;
	}
	/// gets/subs value from counter
	void PopVal(BYTE v)
	{
		m_iValSum -= (ULONG)v;
		m_iValCnt --;
	}
	BYTE GetMedian()
	{
		if (m_iValCnt > 0)
		{
			return ( (BYTE)( m_iValSum / m_iValCnt )  );
		}
		else
			return 0 ;
	};
private:
	ULONG m_iValSum;
	size_t m_iValCnt;
};


class CMedianCntUL
{
public:
	CMedianCntUL()
	{
		m_iValSum = 0;
		m_iValCnt = 0;
	};
	/// add value to counter
	void PutVal(ULONG v)
	{
		m_iValSum +=  v ;
		m_iValCnt ++;
	}
	/// gets/subs value from counter
	void PopVal(ULONG v)
	{
		if ( ( m_iValCnt > 0) && (m_iValSum >= v))
		{
			m_iValSum -= v;
			m_iValCnt --;
		}
	}
	/// reset class
	void Reset()
	{
		m_iValSum = 0;
		m_iValCnt = 0;
	};
	/// returns medium value
	ULONG GetMedian()
	{
		if (m_iValCnt > 0)
		{
			return ( ( m_iValSum / m_iValCnt ) );
		}
		else
			return 0 ;
	};
	/// service method
	ULONG  GetSum() { return m_iValSum; };
	/// service method
	size_t GetCnt() { return m_iValCnt; };
	
	/// service method
	void AppendCounter( const ULONG Sum, const size_t Count )
	{
		m_iValSum += Sum;
		m_iValCnt += Count;
	}
private:
	ULONG m_iValSum;
	size_t m_iValCnt;
};


class CMedianCntDbl
{
public:
	CMedianCntDbl()
	{
		m_iValSum = 0;
		m_iValCnt = 0;
	};
	/// add value to counter
	void PutVal(double v)
	{
		m_iValSum += v;
		m_iValCnt++;
	}
	/// gets/subs value from counter
	void PopVal(double v)
	{
		if ((m_iValCnt > 0) && (m_iValSum >= v))
		{
			m_iValSum -= v;
			m_iValCnt--;
		}
	}
	/// reset class
	void Reset()
	{
		m_iValSum = 0;
		m_iValCnt = 0;
	};
	/// returns medium value
	double GetMedian()
	{
		if (m_iValCnt > 0)
		{
			return ((m_iValSum / m_iValCnt));
		}
		else
			return 0;
	};
	/// service method
	double  GetSum() { return m_iValSum; };
	/// service method
	size_t GetCnt() { return m_iValCnt; };

	/// service method
	void AppendCounter(const double Sum, const size_t Count)
	{
		m_iValSum += Sum;
		m_iValCnt += Count;
	}
private:
	double m_iValSum;
	size_t m_iValCnt;
};


inline BYTE FLOAT2BYTE(float val)
{
	return ( (BYTE)(val * 255.0f ) );
}


inline float BYTE2FLOAT(BYTE val)
{
	return ( (float)val * ( 1/255.0f ) );
}

inline float WORD2FLOAT(WORD val)
{
	return ( (float)val * ( 1/65535.0f ) );
}

inline float avg3f( float& a, float& b, float& c)
{
	return ( ( a + b + c ) / 3.0f );
}

//weighted average (weight from 0 to 1)
inline float wavg3f( float& a, float& b, float& c, float& wa, float& wb, float& wc )
{
	return ( ( a * wa + b * wb + c * wc ) / (wa + wb + wc) );
}

template< class T >
inline T max3T( T& a, T& b, T& c)
{
	if (a > b)
	{
		if ( a > c )
		{
			return ( a );
		}
		else
		{
			return ( c );
		}
	}
	else
	{
		if ( b > c )
		{
			return ( b );
		}
		else
		{
			return ( c );
		}
	}
}

template< class T >
inline T max3TV( T a, T b, T c)
{
	if (a > b)
	{
		if ( a > c )
		{
			return ( a );
		}
		else
		{
			return ( c );
		}
	}
	else
	{
		if ( b > c )
		{
			return ( b );
		}
		else
		{
			return ( c );
		}
	}
}


template< class T >
inline T min3T( T& a, T& b, T& c)
{
	if (a < b)
	{
		if ( a < c )
		{
			return ( a );
		}
		else
		{
			return ( c );
		}
	}
	else
	{
		if ( b < c )
		{
			return ( b );
		}
		else
		{
			return ( c );
		}
	}
}

template< class T >
inline T max2T( T& a, T& b )
{
	if (a > b)
	{
		return ( a );
	}
	else
	{
		return ( b );
	}
}



template < class T >
inline T Clamp(T val, T Min, T Max)
{
	if ( val < Min )
		return ( Min );
	if ( val > Max )
		return ( Max );
	return ( val );
}



inline long InterpolateBetween2Valsl(const long in0, const long in1, float Progress)
{
	long ldy = in1 - in0;
	long _out = in0 + (long)( ( (float)ldy ) * Progress );
	return ( _out );
}

template < class T >
inline T InterpolateBetween2Vals(const T in0, const T in1, float Progress)
{
	T ldy = in1 - in0;
	T _out = in0 + (T)( ( (float)ldy ) * Progress );
	return ( _out );
}



inline int InterpolateBetween2Valsi(const int in0, const int in1, float Progress)
{
	int ldy = in1 - in0;
	int _out = in0 + (long)( ( (float)ldy ) * Progress );
	return ( _out );
}

inline BYTE InterpolateBetween3Valsb(const BYTE in0, const BYTE in1, const BYTE in2, const BYTE valSter, const WORD midvalster)
{
	tbtw valSterr;
	valSterr.b.h = 0;
	valSterr.b.l = valSter;

	if ( ( valSterr.w < midvalster ) && ( valSterr.w > 0 ) )
	{
		tbtw iin1;
		tbtw iin0;
		iin1.b.h = 0;
		iin0.b.h = 0;
		iin1.b.l = in1;
		iin0.b.l = in0;
		float ster = (float)valSterr.w / (float)midvalster;
		int ldy = (int)iin1.w - (int)iin0.w;
		return ( iin0.w + (WORD)( ( (float)ldy ) * ster ) );

	//	ster = 0.5f;
	//	BYTE _out = InterpolateBetween2Valsb( in0, in1, ster );
	//	return ( _out );
	}
	else
	{
		tbtw iin1;
		tbtw iin2;
		iin1.b.h = 0;
		iin2.b.h = 0;
		iin1.b.l = in1;
		iin2.b.l = in2;
		float ster = ( (float)valSterr.w - (float)midvalster ) / (float)midvalster;
//		ster = 0.5f;
//		BYTE _out = InterpolateBetween2Valsb( in1, in2, ster );
//		return ( _out );
		int ldy = (int)iin2.w - (int)iin1.w;
     	return ( iin1.w + (WORD)( ( (float)ldy ) * ster ) );
//		int ldy = (int)in1 - (int)in2;
//     	return ( in0 - (BYTE)( ( (float)ldy ) * ster ) );
	}
}


inline bool IsPointInsideRect( RECT& rect, POINT& point)
{
	return ( ( rect.left <= point.x ) && ( rect.right > point.x ) && ( rect.top <= point.y ) && ( rect.bottom > point.y ) );

}

inline void MakeLocalCoords( RECT& rect, POINT& point )
{
	point.x -= rect.left;
	point.y -= rect.top;
}

inline long RandomVal( DWORD& valNum, DWORD& seed0, DWORD& seed1, DWORD seed2 )
{
	long _out = valNum << 14 ^ valNum ^ seed0 + valNum >> 10 ^ seed1;
	return ( _out * seed1 + seed2 ^ ( valNum << 16 ) + _out >> 16 );
}

inline DWORD DRandomVal()
{
	DWORD valNum = (DWORD)rand();
	DWORD sd1 = 0x01234123;
	DWORD sd2 = 0x82F72310;
	DWORD sd3 = 0xA15172C6;

	return ( (DWORD)RandomVal( valNum, sd1 , sd2, sd3 ) );
}

// random val between 0 -> 65535
inline long ARandVal(  DWORD& valNum, DWORD& seed0, DWORD& seed1, DWORD seed2 )
{
	return ( RandomVal( valNum, seed0, seed1, seed2 ) & 0xFFFF );
}

// returns 0.0  -> 1.0
inline float fARandVal( float& progress, DWORD& seed0, DWORD& seed1, DWORD seed2 )
{
	DWORD l_valNum = (DWORD)( (double)progress * (double)0xFFFFF );
	DWORD l_out = RandomVal( l_valNum, seed0, seed1, seed2 ) & 0xFFFFF;
	return ( (float)( (double)l_out * 1.0f / ( (double)0xFFFFF ) ) ); 
}

// returns 0.0  -> 1.0
inline float fARandValN( DWORD& valNum, DWORD& seed0, DWORD& seed1, DWORD seed2 )
{
	DWORD l_out = RandomVal( valNum, seed0, seed1, seed2 ) & 0xFFFFF;
	return ( (float)( (double)l_out * 1.0f / ( (double)0xFFFFF ) ) ); 
}

// random val between -65535 -> 65535
inline long WRandVal(  DWORD& valNum, DWORD& seed0, DWORD& seed1, DWORD seed2 )
{
	long _out = RandomVal( valNum, seed0, seed1, seed2 );
	if ( _out > 0 )
		return ( _out & 0xFFFF );
	else
		return ( - ( _out & 0xFFFF ) );
}

inline int max3( int& a, int& b, int& c)
{
	return max3T<int>(a,b,c);
}


inline int min3( int& a, int& b, int& c)
{
	return min3T<int>(a,b,c);
}

template< class T >
inline T clampT(T a, T min, T max)
{
	if (a < min)
		return (min);
	if (a > max)
		return (max);
	return (a);
}




inline void RGB2HSV( float R, float G, float B, float& H, float& S, float& V)
{
	float f = 0.0;
	float i = 0.0;
	V = max3T<float>( R, G, B );
	float tmppx = min3T<float>( R, G, B );
    if (tmppx == V)
	{
        H = 0.0;
        S = 0.0;
    }
    else 
	{
        f = (R == tmppx) ? G - B : ( ( G == tmppx ) ? B - R : R - G);
        i = (R == tmppx) ? 3 : ( (G == tmppx ) ? 5 : 1);
        H = fmod( ( i - f / ( V - tmppx ) )*60, 360 );
        S = ( ( V - tmppx ) / V );
    }
}

inline void RGB2HSVB(BYTE& R, BYTE& G, BYTE& B, float& H, float& S, float& V)
{
	return RGB2HSV(  BYTE2FLOAT(R), BYTE2FLOAT(G),  BYTE2FLOAT(B), H, S, V);

}

inline float clamp(float a, float min, float max)
{
	return clampT<float>(a,min,max);
}

inline void SinusCosinus (const float& angle, float& vsin, float& vcos)
{
	vsin = sinf(angle);
	vcos = cosf(angle);
}
#define M_PI       3.14159265358979323846

inline float ArcTanXY(const float& x, const float& y)
{
	return atan2f( x, y ) + M_PI;
}

inline float Tangens(const float& alpha)
{
	return tanf( alpha - M_PI);
}

inline double DTangens(const float& alpha)
{
	return tan( M_PI - alpha );
}

inline float SquareRoot(const float& x)
{
	return sqrtf( x );
}


inline float ConvertArcToTwoPi(float& inRad)
{
	float _outRad = inRad;
	for (;;)
	{
		if ( ( _outRad >= 0.0f ) && ( _outRad < 2 * M_PI ))
			return ( _outRad );
		if ( _outRad < 0.0f )
		{
			_outRad += 2.0 * M_PI;
		}
		else if ( _outRad >= 2 * M_PI )
		{
			_outRad -= 2.0 * M_PI;
		}
	}
}

inline float Deg2Rad(float& inDeg)
{
	return ( ( inDeg * ( 1.0f / 180.0f ) ) * M_PI ) ;
}


// arc <0-2pi)    arcA, arcB <-inf, inf>
inline bool IsBetween2Angles(float& arc, float& arcA, float& arcB)
{
	if ( arcA > arcB )
		return false;
	if ( ( arcA >= 0.0f ) && ( arcA < 2 * M_PI ) && ( arcB < 2 * M_PI ) )
		return ( ( arc >= arcA ) && ( arc <= arcB ) ) ;
	float l_arcA = arcA, l_arcB = arcB;
	for (;;)
	{
		if ( ( l_arcA >= 0.0f ) && ( l_arcA < 2 * M_PI ))
			break;
		if ( l_arcA < 0.0f )
		{
			l_arcA += 2 * M_PI;
			l_arcB += 2 * M_PI;
		}
		else if ( l_arcA >= 2 * M_PI )
		{
			l_arcA -= 2 * M_PI;
			l_arcB -= 2 * M_PI;
		}
	}
	if ( ( l_arcA >= 0.0f ) && ( l_arcA < 2 * M_PI ) && ( l_arcB < 2 * M_PI ) )
		return ( ( arc >= l_arcA ) && ( arc <= l_arcB ) ) ;
	
	if ( l_arcB >= 2 * M_PI )
	{
		if ( arc >= l_arcA )
			return true;
		if ( arc <= ( l_arcB - 2 * M_PI ) ) 
			return true;
	}
	return false;
}



#endif  // TRSR_MATHFCTIONS