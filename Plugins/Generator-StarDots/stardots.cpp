/*
OFX Genereator example plugin, a plugin that illustrates the use of the OFX Support library.

Portions Copyright (C) 2010-2011 Sony Creative Software Inc.

Portions Copyright (C) 2004-2005 The Open Effects Association Ltd
Author Bruno Nicoletti bruno@thefoundry.co.uk

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.
* Neither the name The Open Effects Association Ltd, nor the names of its 
contributors may be used to endorse or promote products derived from this
software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The Open Effects Association Ltd
1 Wardour St
London W1D 6PA
England


*/


#include <stdio.h>
#include <limits>
#include "ofxsImageEffect.h"
#include "ofxsMultiThread.h"

#include "../include/ofxsProcessing.H"
#include "../shared/mathFunctions.h"
#include "../shared/coldefs.h"
#include "../shared/circledrawer.h"
#include "../shared/ColorFiller.h"



////////////////////////////////////////////////////////////////////////////////
// base class for the noise

/** @brief  Base class used to blend two images together */
class StardotsGeneratorBase : public OFX::ImageProcessor {
protected :

  int        _tilewidth;

  RGBAValue  bcol;
  RGBAValue  scol;

  long   camX;
  long   camY;
  long   camZ;

  float  angleX;
  float  angleY;
  float  angleZ;

  DWORD  dotsNum;
  float  starScale;

  DWORD  CamDistance;

  DWORD  Seed0;
  DWORD  Seed1;
  DWORD  Seed2;
  DWORD  Seed3;
  DWORD  Seed4;
  DWORD  Seed5;





public :
  /** @brief no arg ctor */
  StardotsGeneratorBase(OFX::ImageEffect &instance)
    : OFX::ImageProcessor(instance)
    , _tilewidth(8)

    , bcol(0.0, 0.0, 0.0, 1.0)
    , scol(1.0, 1.0, 1.0, 1.0)
    , camX(16383)
    , camY(16383)
    , camZ(0)
    , angleX( 0.0)
    , angleY(0.0)
    , angleZ(0.0)
    , dotsNum(16384)
    , starScale(2.0f)
    , CamDistance(20000)
    , Seed0(0x34134FD2)
    , Seed1(0x94444106)
    , Seed2(0xFF075C21)
    , Seed3(0x921F6487)
    , Seed4(0x111A71C3)
    , Seed5(0x701683A5)
  {
  }


  /** @brief set back color */
  void setBackColor(double r, double g, double b, double a) { bcol.r = r; bcol.g = g; bcol.b = b; bcol.a = a; }

  /** @brief set back color */
  void setStarColor(double r, double g, double b, double a) { scol.r = r; scol.g = g; scol.b = b; scol.a = a; }

  /** @brief set number of stars */
  void setDotsNum(DWORD num) { dotsNum = num; }

  /** @brief set camera position */
  void setCamPos(long x, long y, long z) { camX = x; camY = y; camZ = z; }

  /** @brief set camera angles */
  void setCamAngle(float ax, float ay, float az) { angleX = ax; angleY = ay; angleZ = az; }

  /** @brief set camera distance */
  void setCamDist(DWORD dist) { CamDistance = dist; }

  /** @brief set maximum star size */
  void setStarScale(float maxStar) { starScale = maxStar; }

};

/** @brief templated class to blend between two images */
template <class PIX, int nComponents, int max>
class StardotsGenerator : public StardotsGeneratorBase {
public :
  // ctor
  StardotsGenerator(OFX::ImageEffect &instance) 
    : StardotsGeneratorBase(instance)
  {}


  // and do some processing
  void multiThreadProcessImages(OfxRectI procWindow)
  {

      const OfxRectI l_bnds = _dstImg->getBounds();
      PIX* l_pDstPic = (PIX*)_dstImg->getPixelAddress(l_bnds.x1, l_bnds.y1);
      unsigned int l_imgWidth = (l_bnds.x2 - l_bnds.x1);
      unsigned int l_imgHeight = (l_bnds.y2 - l_bnds.y1);

      ULONG lHWidth = l_imgWidth / 2;
      ULONG lHHeight = l_imgHeight / 2;

      const int distMultiplier = 4;
      float l_DistB = (float)CamDistance * (float)distMultiplier;
      float l_DDistB = 1.0f / (l_DistB - (float)CamDistance);
      long  l_distM = (long)CamDistance / 8;

      float l_fZs, l_fZc;
      float l_fYs, l_fYc;
      float l_fXs, l_fXc;

      SinusCosinus(Deg2Rad(angleZ), l_fZs, l_fZc);
      SinusCosinus(Deg2Rad(angleY), l_fYs, l_fYc);
      SinusCosinus(Deg2Rad(angleX), l_fXs, l_fXc);

      ColorFiller<PIX, nComponents, max > l_fill;
      l_fill.FillSubArea(l_pDstPic, l_imgWidth, l_imgHeight, procWindow, (bcol));
      CCircleDrawer<PIX, nComponents, max >  l_circle;

      for (DWORD pts = 0; pts < dotsNum; ++pts)
      {
          long _x = ARandVal(pts, Seed0, Seed1, Seed2);
          long _y = ARandVal(pts, Seed1, Seed2, Seed0);
          long _z = ARandVal(pts, Seed2, Seed0, Seed1);

          // transform to camera XYZ position
          long lx = _x - camX;
          long ly = _y - camY;
          long lz = _z - camZ;
          // transform angles...

          //angle Z
          if (angleZ > 0.0f)
          {
              register long l_lx = (long)(((double)lx * (double)l_fZc) - ((double)ly * (double)l_fZs));
              register long l_ly = (long)(((double)lx * (double)l_fZs) + ((double)ly * (double)l_fZc));
              lx = l_lx;
              ly = l_ly;
          }
          // angle Y
          if (angleY > 0.0f)
          {
              register long l_lx = (long)(((double)lx * (double)l_fYc) - ((double)lz * (double)l_fYs));
              register long l_lz = (long)(((double)lx * (double)l_fYs) + ((double)lz * (double)l_fYc));
              lx = l_lx;
              lz = l_lz;
          }

          // angle X
          if (angleX > 0.0f)
          {
              register long l_ly = (long)(((double)ly * (double)l_fXc) - ((double)lz * (double)l_fXs));
              register long l_lz = (long)(((double)ly * (double)l_fXs) + ((double)lz * (double)l_fXc));
              ly = l_ly;
              lz = l_lz;
          }

          // transform to 2d and draw
          if ((lz > 0) && (lz < l_DistB))
          {

              float zScale = (float)CamDistance / ((float)lz + (float)CamDistance);
              float flx = (float)lx * zScale;
              float fly = (float)ly * zScale;

              lx = (long)((float)lx * zScale);
              ly = (long)((float)ly * zScale);
              if ((abs(lx) < lHWidth) && (abs(ly) < lHHeight))
              {

                  if (lz > CamDistance)
                  {

                      float l_iii = 1.0f - ((float)lz - (float)CamDistance) * l_DDistB;
                      RGBAValue l_int = interpolate2Colors(bcol, scol, l_iii);
                      l_circle.PlotPoint(lx + lHWidth, ly + lHHeight, l_imgWidth, l_imgHeight, procWindow, l_pDstPic, (l_int), 1.0f);
                  }
                  else
                  {
                      float l_plSize = 0.8f + starScale * (1.0f - (float)lz / (float)CamDistance);
                      if (lz > l_distM)
                      {
                          l_circle.Draw((lx + lHWidth), (ly + lHHeight), l_plSize, l_imgWidth, l_imgHeight, l_pDstPic, procWindow, (scol));
                      }
                      else
                      {
                          float l_iii = (float)lz / (float)l_distM;
                          RGBAValue l_int = interpolate2Colors(bcol, scol, l_iii);
                          l_circle.Draw((lx + lHWidth), (ly + lHHeight), l_plSize, l_imgWidth, l_imgHeight, l_pDstPic, procWindow, (l_int));
                      }
                  }
              }

          }

      }



  }

};

/** @brief templated class to blend between two images */
template <class PIX, int nComponents, int max>
class StardotsGeneratorBGRA : public StardotsGeneratorBase {
public :
  // ctor
  StardotsGeneratorBGRA(OFX::ImageEffect &instance) 
    : StardotsGeneratorBase(instance)
  {}

  // and do some processing
  void multiThreadProcessImages(OfxRectI procWindow)
  {

    const OfxRectI l_bnds = _dstImg->getBounds();
    PIX* l_pDstPic = (PIX*)_dstImg->getPixelAddress(l_bnds.x1, l_bnds.y1);
    unsigned int l_imgWidth = (l_bnds.x2 - l_bnds.x1);
    unsigned int l_imgHeight = (l_bnds.y2 - l_bnds.y1);

    ULONG lHWidth = l_imgWidth / 2;
    ULONG lHHeight = l_imgHeight / 2;

    const int distMultiplier = 4;
    float l_DistB = (float)CamDistance * (float)distMultiplier;
    float l_DDistB = 1.0f / (l_DistB - (float)CamDistance);
    long  l_distM = (long)CamDistance / 8;

    float l_fZs, l_fZc;
    float l_fYs, l_fYc;
    float l_fXs, l_fXc;

    SinusCosinus(Deg2Rad(angleZ), l_fZs, l_fZc);
    SinusCosinus(Deg2Rad(angleY), l_fYs, l_fYc);
    SinusCosinus(Deg2Rad(angleX), l_fXs, l_fXc);

    ColorFiller<PIX, nComponents, max > l_fill;
    l_fill.FillSubArea(l_pDstPic, l_imgWidth, l_imgHeight, procWindow, col2BGRA(bcol));
    CCircleDrawer<PIX, nComponents, max >  l_circle;

    for (DWORD pts = 0; pts < dotsNum; ++pts)
    {
        long _x = ARandVal(pts, Seed0, Seed1, Seed2);
        long _y = ARandVal(pts, Seed1, Seed2, Seed0);
        long _z = ARandVal(pts, Seed2, Seed0, Seed1);

        // transform to camera XYZ position
        long lx = _x - camX;
        long ly = _y - camY;
        long lz = _z - camZ;
        // transform angles...

        //angle Z
        if (angleZ > 0.0f)
        {
            register long l_lx = (long)(((double)lx * (double)l_fZc) - ((double)ly * (double)l_fZs));
            register long l_ly = (long)(((double)lx * (double)l_fZs) + ((double)ly * (double)l_fZc));
            lx = l_lx;
            ly = l_ly;
        }
        // angle Y
        if (angleY > 0.0f)
        {
            register long l_lx = (long)(((double)lx * (double)l_fYc) - ((double)lz * (double)l_fYs));
            register long l_lz = (long)(((double)lx * (double)l_fYs) + ((double)lz * (double)l_fYc));
            lx = l_lx;
            lz = l_lz;
        }

        // angle X
        if (angleX > 0.0f)
        {
            register long l_ly = (long)(((double)ly * (double)l_fXc) - ((double)lz * (double)l_fXs));
            register long l_lz = (long)(((double)ly * (double)l_fXs) + ((double)lz * (double)l_fXc));
            ly = l_ly;
            lz = l_lz;
        }

        // transform to 2d and draw
        if ((lz > 0) && (lz < l_DistB))
        {

            float zScale = (float)CamDistance / ((float)lz + (float)CamDistance);
            float flx = (float)lx * zScale;
            float fly = (float)ly * zScale;

            lx = (long)((float)lx * zScale);
            ly = (long)((float)ly * zScale);
            if ((abs(lx) < lHWidth) && (abs(ly) < lHHeight))
            {

                if (lz > CamDistance)
                {

                    float l_iii = 1.0f - ((float)lz - (float)CamDistance) * l_DDistB;
                    RGBAValue l_int = interpolate2Colors(bcol, scol, l_iii);
                    //l_circle.PlotPoint(lx + lHWidth, ly + lHHeight,, l_imgWidth, l_imgHeight, procWindow, l_pDstPic, col2BGRA(l_int), 1.0f);
                    l_circle.PlotPoint(lx + lHWidth, ly + lHHeight, l_imgWidth, l_imgHeight, procWindow, l_pDstPic, col2BGRA(l_int), 1.0f);
//                    l_circle.PlotPoint(lx + lHWidth, ly + lHHeight, l_imgWidth, l_imgHeight, procWindow, l_pDstPic, col2BGRA(scol), 1.0f);
                }
                else
                {
                    float l_plSize = 0.8f + starScale * (1.0f - (float)lz / (float)CamDistance);
                    if (lz > l_distM)
                    {
                        l_circle.Draw((lx + lHWidth), (ly + lHHeight), l_plSize, l_imgWidth, l_imgHeight, l_pDstPic, procWindow, col2BGRA(scol));
                    }
                    else
                    {
                        float l_iii = (float)lz / (float)l_distM;
                        RGBAValue l_int = interpolate2Colors(bcol, scol, l_iii);
//                        l_circle.Draw((lx + lHWidth), (ly + lHHeight), l_plSize, l_imgWidth, l_imgHeight, l_pDstPic, procWindow, col2BGRA(scol));
                        l_circle.Draw((lx + lHWidth), (ly + lHHeight), l_plSize, l_imgWidth, l_imgHeight, l_pDstPic, procWindow, col2BGRA(l_int) );
                    }
                }
            }

        }

    }





  }

};

////////////////////////////////////////////////////////////////////////////////
/** @brief The plugin that does our work */
class StardotsPlugin : public OFX::ImageEffect 
{
protected :
  // do not need to delete these, the ImageEffect is managing them for us
  OFX::Clip *dstClip_;

  OFX::DoubleParam   *width_;

  OFX::RGBAParam  *bcolor_;
  OFX::RGBAParam  *stcolor_;

  OFX::DoubleParam    *camX_;
  OFX::DoubleParam    *camY_;
  OFX::DoubleParam    *camZ_;

  OFX::DoubleParam* angleX_;
  OFX::DoubleParam* angleY_;
  OFX::DoubleParam* angleZ_;

  OFX::IntParam  *dotsNum_;
  OFX::DoubleParam* CamDistance_;


 // OFX::Double3DParam* camPos_;


public :
  /** @brief ctor */
  StardotsPlugin(OfxImageEffectHandle handle)
    : ImageEffect(handle)
    , dstClip_(0)
    , width_(0)
    , bcolor_(0)
    , stcolor_(0)
    , camX_(0)
    , camY_(0)
    , camZ_(0)
    , angleX_(0)
    , angleY_(0)
    , angleZ_(0)
    , dotsNum_(0)
    , CamDistance_(0)
    //, camPos_(0)
  {
    dstClip_  = fetchClip("Output");
    width_                = fetchDoubleParam("Width");



    bcolor_ =fetchRGBAParam("Background");
    stcolor_ = fetchRGBAParam("Stars");

    //camPos_ = fetchDouble3DParam("CamXYZ");


    dotsNum_ = fetchIntParam("StarsCnt");

    camX_ = fetchDoubleParam("CamX");
    camY_ = fetchDoubleParam("CamY");
    camZ_ = fetchDoubleParam("CamZ");

    angleX_ = fetchDoubleParam("AngleX");
    angleY_ = fetchDoubleParam("AngleY");
    angleZ_ = fetchDoubleParam("AngleZ");

    CamDistance_ = fetchDoubleParam("CamD");

  }

  /* Override the render */
  virtual void render(const OFX::RenderArguments &args);

  /* override changedParam */
  virtual void changedParam(const OFX::InstanceChangedArgs &args, const std::string &paramName);

  /* Override the clip preferences, we need to say we are setting the frame varying flag */
  virtual void getClipPreferences(OFX::ClipPreferencesSetter &clipPreferences);

  /* set up and run a processor */
  void setupAndProcess(StardotsGeneratorBase &, const OFX::RenderArguments &args);

  /** @brief The get RoD action.  We flag an infinite rod */
  bool getRegionOfDefinition(const OFX::RegionOfDefinitionArguments &args, OfxRectD &rod);

  /** @brief Vegas requires conversion of keyframe data */
  void upliftVegasKeyframes(const OFX::SonyVegasUpliftArguments &upliftInfo);
};


////////////////////////////////////////////////////////////////////////////////
/** @brief render for the filter */

////////////////////////////////////////////////////////////////////////////////
// basic plugin render function, just a skelington to instantiate templates from


/* set up and run a processor */
void
StardotsPlugin::setupAndProcess(StardotsGeneratorBase &processor, const OFX::RenderArguments &args)
{
  // get a dst image
  std::auto_ptr<OFX::Image>  dst(dstClip_->fetchImage(args.time));
  OFX::BitDepthEnum         dstBitDepth    = dst->getPixelDepth();
  OFX::PixelComponentEnum   dstComponents  = dst->getPixelComponents();
  OfxRectI                  dstBounds      = dst->getBounds();
  RGBAValue color;

  // set the images
  processor.setDstImg(dst.get());

  // set the render window
  processor.setRenderWindow(args.renderWindow);

  // set parameter values
  double width = width_->getValueAtTime(args.time);

  processor.setStarScale((float)(width * ((dstBounds.y2 - dstBounds.y1) / 50)));

  stcolor_->getValueAtTime(args.time, color.r, color.g, color.b, color.a);
  processor.setStarColor(color.r, color.g, color.b, color.a);
  bcolor_->getValueAtTime(args.time, color.r, color.g, color.b, color.a);
  processor.setBackColor(color.r, color.g, color.b, color.a);

  int l_val = 1;
  dotsNum_->getValueAtTime(args.time, l_val);
  if (l_val == 0)
  {
      l_val = 1;
  }
  processor.setDotsNum(l_val * 3000 + 2048);

  double lx;
  double ly;
  double lz;
  //camPos_->getValueAtTime(args.time, lx, ly, lz );
  camX_->getValueAtTime(args.time, lx);
  camY_->getValueAtTime(args.time, ly);
  camZ_->getValueAtTime(args.time, lz);

  processor.setCamPos((long)(lx * 20000.0), (long)(ly * 20000.0), (long)(lz * 50000.0));

  angleX_->getValueAtTime(args.time, lx);
  angleY_->getValueAtTime(args.time, ly);
  angleZ_->getValueAtTime(args.time, lz);

  processor.setCamAngle(lx*360.0, ly * 360.0, lz * 360.0);

  double lv;
  CamDistance_->getValueAtTime(args.time, lv);
  processor.setCamDist((DWORD)(lv * 20000.0 + 10000));

  // Call the base class process member, this will call the derived templated process code
  processor.process();
}

/* Override the clip preferences, we need to say we are setting the frame varying flag */
void 
StardotsPlugin::getClipPreferences(OFX::ClipPreferencesSetter &clipPreferences)
{
  clipPreferences.setOutputFrameVarying(true);
}

/** @brief The get RoD action.  We flag an infinite rod */
bool 
StardotsPlugin::getRegionOfDefinition(const OFX::RegionOfDefinitionArguments &args, OfxRectD &rod)
{
  // we can generate noise anywhere on the image plan, so set our RoD to be infinite
  rod.x1 = rod.y1 = -std::numeric_limits<double>::infinity(); // kOfxFlagInfiniteMin;
  rod.x2 = rod.y2 = std::numeric_limits<double>::infinity(); // kOfxFlagInfiniteMax;
  return true;
}

// the overridden render function
void
StardotsPlugin::render(const OFX::RenderArguments &args)
{
  // instantiate the render code based on the pixel depth of the dst clip
  OFX::BitDepthEnum       dstBitDepth    = dstClip_->getPixelDepth();
  OFX::PixelComponentEnum dstComponents  = dstClip_->getPixelComponents();
  OFX::PixelOrderEnum     dstOrder       = dstClip_->getPixelOrder();

  // do the rendering
  if(dstComponents == OFX::ePixelComponentRGBA) 
  {
      if(dstOrder == OFX::ePixelOrderBGRA)
      {
        switch(dstBitDepth) 
        {
        case OFX::eBitDepthUByte : 
            {      
              StardotsGeneratorBGRA<unsigned char, 4, 255> fred(*this);
              setupAndProcess(fred, args);
            }
            break;

        case OFX::eBitDepthUShort : 
          {
            StardotsGeneratorBGRA<unsigned short, 4, 65535> fred(*this);
            setupAndProcess(fred, args);
          }                          
          break;

        case OFX::eBitDepthFloat : 
          {
            StardotsGeneratorBGRA<float, 4, 1> fred(*this);
            setupAndProcess(fred, args);
          }
          break;
        }
      }
      else
      {
        switch(dstBitDepth) 
        {
        case OFX::eBitDepthUByte : 
            {      
              StardotsGenerator<unsigned char, 4, 255> fred(*this);
              setupAndProcess(fred, args);
            }
            break;

        case OFX::eBitDepthUShort : 
          {
            StardotsGenerator<unsigned short, 4, 65535> fred(*this);
            setupAndProcess(fred, args);
          }                          
          break;

        case OFX::eBitDepthFloat : 
          {
            StardotsGenerator<float, 4, 1> fred(*this);
            setupAndProcess(fred, args);
          }
          break;
        }
      }
  }
  else 
  {
    switch(dstBitDepth) 
    {
    case OFX::eBitDepthUByte : 
      {
        StardotsGenerator<unsigned char, 1, 255> fred(*this);
        setupAndProcess(fred, args);
      }
      break;

    case OFX::eBitDepthUShort : 
      {
        StardotsGenerator<unsigned short, 1, 65536> fred(*this);
        setupAndProcess(fred, args);
      }                          
      break;

    case OFX::eBitDepthFloat : 
      {
        StardotsGenerator<float, 1, 1> fred(*this);
        setupAndProcess(fred, args);
      }                          
      break;
    }
  } 
}

// we have changed a param
void
StardotsPlugin::changedParam(const OFX::InstanceChangedArgs &args, const std::string &paramName)
{
}


typedef float           SFDIBNUM;
typedef SFDIBNUM       *PSFDIBNUM;
typedef const SFDIBNUM *PCSFDIBNUM;

typedef struct tSFDIBPIXEL
{
    union
    {
        SFDIBNUM    afl[4];
        struct
        {
            SFDIBNUM    b;    // Blue value
            SFDIBNUM    g;    // Green value
            SFDIBNUM    r;    // Red value
            SFDIBNUM    a;    // Alpha value
        };
    };

    inline void Init(SFDIBNUM _r, SFDIBNUM _g, SFDIBNUM _b, SFDIBNUM _a)
    {
        r = _r;
        g = _g;
        b = _b;
        a = _a;
    }

    void MulAlpha()
    {
        // pre-multiply the alpha channel into this pixel
        r *= a;
        g *= a;
        b *= a;
    }
    void UnMulAlpha()
    {
        // UN-pre-multiply the alpha channel into this pixel
        if (0.0 != a)
        {
            r /= a;
            g /= a;
            b /= a;
        }
    }
} SFDIBPIXEL;
//hungarian: pxl

typedef SFDIBPIXEL *PSFDIBPIXEL;
typedef const SFDIBPIXEL *PCSFDIBPIXEL;

typedef struct tCHECKR_PROPS
{
    SFDIBPIXEL  foreColor;
    SFDIBPIXEL  backColor;
    double      dTileWidth, dTileHeight;
    double      dBlendAmountX, dBlendAmountY;
    double      dTileOffsetX, dTileOffsetY;
    bool        bSymmetricTiles;
    bool        bProportionalBlend;
} CHECKR_PROPS;

/** @brief Vegas requires conversion of keyframe data */
void StardotsPlugin::upliftVegasKeyframes(const OFX::SonyVegasUpliftArguments &upliftInfo)
{
    void*  pvData0 = upliftInfo.getKeyframeData     (0);
    int    ccData0 = upliftInfo.getKeyframeDataSize (0);
    double dData0  = upliftInfo.getKeyframeTime     (0);

    if(ccData0 != sizeof(CHECKR_PROPS))
        return;

    CHECKR_PROPS* pProps0 = (CHECKR_PROPS*)pvData0;

    width_->setValue(pProps0->dTileWidth);
    //height_->setValue(pProps0->dTileHeight);
    //square_->setValue(pProps0->bSymmetricTiles);
    //color1_->setValue(pProps0->foreColor.r, pProps0->foreColor.g, pProps0->foreColor.b, pProps0->foreColor.a);
    //color2_->setValue(pProps0->backColor.r, pProps0->backColor.g, pProps0->backColor.b, pProps0->backColor.a);
    //gridPosition_->setValue(pProps0->dTileOffsetX, pProps0->dTileOffsetY);

    bool fWidthAnimates = false;
    bool fHeightAnimates = false;
    bool fSquareAnimates = false;
    bool fColor1Animates = false;
    bool fColor2Animates = false;
    bool fPositionAnimates = false;

    for(int idx = 1; idx < upliftInfo.keyframeCount; idx++)
    {
        void*  pvDataN = upliftInfo.getKeyframeData     (idx);
        int    ccDataN = upliftInfo.getKeyframeDataSize (idx);
        double dDataN  = upliftInfo.getKeyframeTime     (idx);
        CHECKR_PROPS* pPropsN = (CHECKR_PROPS*)pvData0;

        if(pProps0->dTileWidth != pPropsN->dTileWidth)
        {
            if(! fWidthAnimates)
                width_->setValueAtTime(dData0, pProps0->dTileWidth);
            width_->setValueAtTime(dDataN, pPropsN->dTileWidth);
            fWidthAnimates = true;
        }
        //if(pProps0->dTileHeight != pPropsN->dTileHeight)
        //{
        //    if(! fHeightAnimates)
        //        height_->setValueAtTime(dData0, pProps0->dTileHeight);
        //    height_->setValueAtTime(dDataN, pPropsN->dTileHeight);
        //    fHeightAnimates = true;
        //}
        //if(pProps0->bSymmetricTiles != pPropsN->bSymmetricTiles)
        //{
        //    if(! fSquareAnimates)
        //        square_->setValueAtTime(dData0, pProps0->bSymmetricTiles);
        //    square_->setValueAtTime(dDataN, pPropsN->bSymmetricTiles);
        //    fSquareAnimates = true;
        //}
        //if((pProps0->foreColor.r != pPropsN->foreColor.r) ||
        //    (pProps0->foreColor.g != pPropsN->foreColor.g) ||
        //    (pProps0->foreColor.b != pPropsN->foreColor.b) ||
        //    (pProps0->foreColor.a != pPropsN->foreColor.a))
        //{
        //    if(! fColor1Animates)
        //        color1_->setValueAtTime(dData0, pProps0->foreColor.r, pProps0->foreColor.g, pProps0->foreColor.b, pProps0->foreColor.a);
        //    color1_->setValueAtTime(dDataN, pPropsN->foreColor.r, pPropsN->foreColor.g, pPropsN->foreColor.b, pPropsN->foreColor.a);
        //    fColor1Animates = true;
        //}
        //if((pProps0->backColor.r != pPropsN->backColor.r) ||
        //    (pProps0->backColor.g != pPropsN->backColor.g) ||
        //    (pProps0->backColor.b != pPropsN->backColor.b) ||
        //    (pProps0->backColor.a != pPropsN->backColor.a))
        //{
        //    if(! fColor2Animates)
        //        color2_->setValueAtTime(dData0, pProps0->backColor.r, pProps0->backColor.g, pProps0->backColor.b, pProps0->backColor.a);
        //    color2_->setValueAtTime(dDataN, pPropsN->backColor.r, pPropsN->backColor.g, pPropsN->backColor.b, pPropsN->backColor.a);
        //    fColor2Animates = true;
        //}
        //if((pProps0->dTileOffsetX != pPropsN->dTileOffsetX) ||
        //    (pProps0->dTileOffsetY != pPropsN->dTileOffsetY))
        //{
        //    if(! fPositionAnimates)
        //        gridPosition_->setValueAtTime(dData0, pProps0->dTileOffsetX, pProps0->dTileOffsetY);
        //    gridPosition_->setValueAtTime(dDataN, pPropsN->dTileOffsetX, pPropsN->dTileOffsetY);
        //    fPositionAnimates = true;
        //}

        pProps0 = pPropsN;
        dData0 = dDataN;
    }
}


mDeclarePluginFactory(StardotsExamplePluginFactory, {}, {});

using namespace OFX;

void StardotsExamplePluginFactory::describe(OFX::ImageEffectDescriptor &desc) 
{
  desc.setLabels("OFX Stardots", "OFX Stardots", "OFX Stardots");
  desc.setPluginGrouping("Sony OFX");
  desc.addSupportedContext(eContextGenerator);
  desc.addSupportedBitDepth(eBitDepthUByte);
  desc.addSupportedBitDepth(eBitDepthUShort);
  desc.addSupportedBitDepth(eBitDepthFloat);
  desc.addSupportedBitDepth(eBitDepthUByteBGRA);
  desc.addSupportedBitDepth(eBitDepthUShortBGRA);
  desc.addSupportedBitDepth(eBitDepthFloatBGRA);
  desc.setSingleInstance(false);
  desc.setHostFrameThreading(false);
  desc.setSupportsMultiResolution(true);
  desc.setSupportsTiles(true);
  desc.setTemporalClipAccess(false);
  desc.setRenderTwiceAlways(false);
  desc.setSupportsMultipleClipPARs(false);
  desc.setRenderTwiceAlways(false);
}

void StardotsExamplePluginFactory::describeInContext(OFX::ImageEffectDescriptor &desc, ContextEnum context) 
{
  ClipDescriptor *dstClip = desc.defineClip("Output");
  dstClip->addSupportedComponent(ePixelComponentRGBA);
  dstClip->addSupportedComponent(ePixelComponentAlpha);
  dstClip->setSupportsTiles(true);
  dstClip->setFieldExtraction(eFieldExtractSingle);

  RGBAParamDescriptor* bColorParam = desc.defineRGBAParam("Background");
  bColorParam->setLabels("Background", "Background", "Background");
  bColorParam->setScriptName("Background");
  bColorParam->setHint("Background color");
  bColorParam->setDefault(0.0, 0.0, 0.0, 1.0);
  bColorParam->setAnimates(true); // can animate

  RGBAParamDescriptor* stColorParam = desc.defineRGBAParam("Stars");
  stColorParam->setLabels("Stars", "Stars", "Stars");
  stColorParam->setScriptName("Stars");
  stColorParam->setHint("Stars color");
  stColorParam->setDefault(1.0, 1.0, 1.0, 1.0);
  stColorParam->setAnimates(true); // can animate

  IntParamDescriptor* starsCount = desc.defineIntParam("StarsCnt");
  starsCount->setLabels("Stars count", "Stars count", "Stars count");
  starsCount->setScriptName("StarsCnt");
  starsCount->setHint("Stars count in thousands");
  starsCount->setRange(1, 100);
  starsCount->setDefault(20);
  starsCount->setAnimates(true); // can animate

  //Double3DParamDescriptor* camPos = desc.defineDouble3DParam("CamXYZ");
  //camPos->setLabels("Camera pos", "Camera pos", "Camera pos");
  //camPos->setScriptName("CamXYZ");
  //camPos->setHint("Camera 3D position");
  //camPos->setRange(0, 1.0, 0, 5.0, 0, 1.0);
  //camPos->setDefault(0.0,0.0,0.0);
  //camPos->setAnimates(true); // can animate

  DoubleParamDescriptor* camPosX = desc.defineDoubleParam("CamX");
  camPosX->setLabels("Cam X", "Cam X", "Cam x");
  camPosX->setScriptName("CamX");
  camPosX->setHint("Camera X pos.");
  camPosX->setDefault(0.1);
  camPosX->setRange(0, 1.0);
  camPosX->setIncrement(0.1);
  camPosX->setDisplayRange(0, 1);
  camPosX->setAnimates(true); // can animate
  camPosX->setDoubleType(eDoubleTypeScale);

  DoubleParamDescriptor* camPosY = desc.defineDoubleParam("CamY");
  camPosY->setLabels("Cam Y", "Cam Y", "Cam Y");
  camPosY->setScriptName("CamY");
  camPosY->setHint("Camera Y pos.");
  camPosY->setDefault(0.1);
  camPosY->setRange(0, 1.0);
  camPosY->setIncrement(0.1);
  camPosY->setDisplayRange(0, 1);
  camPosY->setAnimates(true); // can animate
  camPosY->setDoubleType(eDoubleTypeScale);

  DoubleParamDescriptor* camPosZ = desc.defineDoubleParam("CamZ");
  camPosZ->setLabels("Cam Z", "Cam Z", "Cam Z");
  camPosZ->setScriptName("CamZ");
  camPosZ->setHint("Camera Z pos.");
  camPosZ->setDefault(0.1);
  camPosZ->setRange(0, 1.0);
  camPosZ->setIncrement(0.1);
  camPosZ->setDisplayRange(0, 1);
  camPosZ->setAnimates(true); // can animate
  camPosZ->setDoubleType(eDoubleTypeScale);


  DoubleParamDescriptor* camDist = desc.defineDoubleParam("CamD");
  camDist->setLabels("Cam zoom", "Cam zoom", "Cam zoom");
  camDist->setScriptName("CamD");
  camDist->setHint("Camera ZOOM.");
  camDist->setDefault(0.1);
  camDist->setRange(0, 1.0);
  camDist->setIncrement(0.1);
  camDist->setDisplayRange(0, 1);
  camDist->setAnimates(true); // can animate
  camDist->setDoubleType(eDoubleTypeScale);


  DoubleParamDescriptor *widthParam = desc.defineDoubleParam("Width");
  widthParam->setLabels("Width", "Width", "Width");
  widthParam->setScriptName("width");
  widthParam->setHint("Width of the nearest star.");
  widthParam->setDefault(0.1);
  widthParam->setRange(0, 1);
  widthParam->setIncrement(0.1);
  widthParam->setDisplayRange(0, 1);
  widthParam->setAnimates(true); // can animate
  widthParam->setDoubleType(eDoubleTypeScale);

  DoubleParamDescriptor* angXParam = desc.defineDoubleParam("AngleX");
  angXParam->setLabels("X Angle", "X Angle", "X Angle");
  angXParam->setScriptName("AngleX");
  angXParam->setHint("angle of X axis.");
  angXParam->setDefault(0.0);
  angXParam->setRange(0, 1);
  angXParam->setIncrement(0.1);
  angXParam->setDisplayRange(0, 1);
  angXParam->setAnimates(true); // can animate
  angXParam->setDoubleType(eDoubleTypeScale);

  DoubleParamDescriptor* angYParam = desc.defineDoubleParam("AngleY");
  angYParam->setLabels("Y Angle", "Y Angle", "Y Angle");
  angYParam->setScriptName("AngleY");
  angYParam->setHint("angle of Y axis.");
  angYParam->setDefault(0.0);
  angYParam->setRange(0, 1);
  angYParam->setIncrement(0.1);
  angYParam->setDisplayRange(0, 1);
  angYParam->setAnimates(true); // can animate
  angYParam->setDoubleType(eDoubleTypeScale);

  DoubleParamDescriptor* angZParam = desc.defineDoubleParam("AngleZ");
  angZParam->setLabels("Z Angle", "Z Angle", "Z Angle");
  angZParam->setScriptName("AngleZ");
  angZParam->setHint("angle of Z axis.");
  angZParam->setDefault(0.0);
  angZParam->setRange(0, 1);
  angZParam->setIncrement(0.1);
  angZParam->setDisplayRange(0, 1);
  angZParam->setAnimates(true); // can animate
  angZParam->setDoubleType(eDoubleTypeScale);


  PageParamDescriptor *page = desc.definePageParam("Controls");
  page->addChild(*bColorParam);
  page->addChild(*stColorParam);
  page->addChild(*starsCount);
//  page->addChild(*camPos);
  page->addChild(*camPosX);
  page->addChild(*camPosY);
  page->addChild(*camPosZ);
  page->addChild(*camDist);
  
  page->addChild(*widthParam);
  page->addChild(*angXParam);
  page->addChild(*angYParam);
  page->addChild(*angZParam);

  desc.addVegasUpgradePath("{DB10DAB1-1247-4194-B666-E0761151BE19}");
}

ImageEffect* StardotsExamplePluginFactory::createInstance(OfxImageEffectHandle handle, ContextEnum context)
{
  return new StardotsPlugin(handle);
}

namespace OFX
{
  namespace Plugin
  {
    void getPluginIDs(OFX::PluginFactoryArray &ids)
    {
      static StardotsExamplePluginFactory p("com.VMRexor:StardotsPlugin", 1, 0);
      ids.push_back(&p);
    }
  };
};
