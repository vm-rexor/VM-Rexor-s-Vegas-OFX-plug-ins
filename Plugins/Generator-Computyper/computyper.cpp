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

#define MAX_TXT_LEN 4097


////////////////////////////////////////////////////////////////////////////////
// base class 

/** @brief  Base class used to blend two images together */
class CompuTypeGeneratorBase : public OFX::ImageProcessor {
protected :

  int        _tilewidth;
  int        _tileheight;
  bool       _square;
  RGBAValue  _color1;
  RGBAValue  _color2;
  Int2DValue _gridOffset;
  std::string  _displText;
  std::string  _fontName;
  //double       _progress;

public :
  /** @brief no arg ctor */
    CompuTypeGeneratorBase(OFX::ImageEffect& instance)
        : OFX::ImageProcessor(instance)
        , _tilewidth(8)
        , _tileheight(8)
        , _square(false)
        , _color1(0.0, 0.0, 0.0, 1.0)
        , _color2(1.0, 1.0, 1.0, 1.0)
        , _gridOffset(0, 0)
       // , _progress(0.0)
  {        
  }

  /** @brief set the dimensions of the tile */
  void setTileDimensions(int w, int h) {_tilewidth = w; _tileheight = h;}

  /** @brief set the dimensions of the tile */
  void setTileSquare(bool square) {_square = square;}

  /** @brief set the dimensions of the tile */
  void setColor1(double r, double g, double b, double a) {_color1.r = r; _color1.g = g; _color1.b = b; _color1.a = a;}

  /** @brief set the dimensions of the tile */
  void setColor2(double r, double g, double b, double a) {_color2.r = r; _color2.g = g; _color2.b = b; _color2.a = a;}

  /** @brief set the dimensions of the tile */
  void setGridOffset(int x, int y) {_gridOffset.x = x; _gridOffset.y = y;}

  /** @brief set text */
  void setText(std::string text) { _displText = text; }

  /** @brief set font name */
  void setFontName(std::string text) { _fontName = text; }

  /** @brief set progress level */
  //void setProgress(double progress) { _progress = progress; }

};

/** @brief templated class to blend between two images */
template <class PIX, int nComponents, int max>
class CompuTypeGenerator : public CompuTypeGeneratorBase {
public :
  // ctor
  CompuTypeGenerator(OFX::ImageEffect &instance) 
    : CompuTypeGeneratorBase(instance)
  {}

  // and do some processing
  void multiThreadProcessImages(OfxRectI procWindow)
  {
    // push pixels


      OfxRectI l_imgbounds = _dstImg->getBounds();

      ULONG uXSize = l_imgbounds.x2 - l_imgbounds.x1;
      ULONG uYSize = l_imgbounds.y2 - l_imgbounds.y1;

      bool  l_boldfont = false;
      bool  l_italicfont = false;
      bool  l_underfont = false;
      char l_fontName[1024];
      float l_rectWidth = 0.5;
      float l_rectHeight = 0.5;
      float l_rectTop = (float)_gridOffset.y / 100.0f;
      float l_rectLeft = (float)_gridOffset.x / 100.0f;

      char l_destTextBuf[4096];

      strcpy(l_fontName, _fontName.c_str());

      strcpy(l_destTextBuf, _displText.c_str());

      // display prepared text
      BITMAPINFO l_binff;
      memset(&l_binff.bmiHeader, 0, sizeof(BITMAPINFOHEADER));
      l_binff.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
      l_binff.bmiHeader.biPlanes = 1;
      l_binff.bmiHeader.biWidth = uXSize;
      l_binff.bmiHeader.biHeight = uYSize + 2;
      l_binff.bmiHeader.biBitCount = 32;

      HDC l_pdc = ::GetDC(0);
      HDC l_tdc = CreateCompatibleDC(l_pdc);



      BYTE* pBase = NULL;
      HBITMAP TmpBmp = CreateDIBSection(l_pdc, &l_binff, DIB_RGB_COLORS, (void**)&pBase, 0, 0);
      HGDIOBJ TmpObj = SelectObject(l_tdc, TmpBmp);

      // get  font size
      int l_fontSize = (int)(_tilewidth);
      // in case zero font size
      if (l_fontSize <= 0)
          l_fontSize = 1;


      HFONT fontNew = CreateFont(
          l_fontSize, // height (negative means use character heights
          0, // width (0 means use default)
          0, // escapement (0 means none)
          0, // orientation (0 means none)
          (l_boldfont) ? FW_NORMAL : FW_BOLD, // "boldness" of font
          l_italicfont, // italic?  true or false
          l_underfont, // underline? true or false
          false, // strikeout?  true or false
          DEFAULT_CHARSET, // desired character set
          OUT_TT_PRECIS, // output precision - use TrueType only
          CLIP_DEFAULT_PRECIS, // clip precision - use default
          ANTIALIASED_QUALITY, // proof quality
          DEFAULT_PITCH | FF_DONTCARE, // pitch and family
          l_fontName // name of font face desired
      );


      HFONT oldfont = (HFONT)SelectObject(l_tdc, fontNew);

      COLORREF oldbackground = SetBkColor(l_tdc, COLORREF(RGB(0, 0, 0)));
      COLORREF oldcolor;
      oldcolor = SetTextColor(l_tdc, COLORREF(RGB(255, 255, 255)));
      int oldbkmode = SetBkMode(l_tdc, TRANSPARENT);



      RECT Regiontymcz;
      Regiontymcz.top = (LONG)(uYSize - _gridOffset.y);
      Regiontymcz.left = (LONG)(_gridOffset.x);
      Regiontymcz.bottom = Regiontymcz.top + (LONG)(uYSize);
      Regiontymcz.right = Regiontymcz.left + (LONG)(uXSize);

      int stat = DrawText(l_tdc, l_destTextBuf, strlen(l_destTextBuf), &Regiontymcz, DT_NOCLIP);

      long l_ssx = l_binff.bmiHeader.biWidth;


      for (int y = procWindow.y1; y < procWindow.y2; y++)
      {
          if (_effect.abort()) break;

          PIX* dstPix = (PIX*)_dstImg->getPixelAddress(procWindow.x1, y);

          for (int x = procWindow.x1; x < procWindow.x2; x++)
          {
              //            BYTE  bAlfa = pBase[((uYSize - y) * l_ssx * 4) + (x * 4)];
              BYTE  bAlfa = pBase[(y * l_ssx * 4) + (x * 4)];

              float ualfa = (float)(bAlfa) / 255.0f;
              if (max == 1) // implies floating point, so don't clamp
              {
                  // bgr space
                  dstPix[0] = (PIX)((_color2.r * ualfa) + (_color1.r * (1.0f - ualfa)));
                  dstPix[1] = (PIX)((_color2.g * ualfa) + (_color1.g * (1.0f - ualfa)));
                  dstPix[2] = (PIX)((_color2.b * ualfa) + (_color1.b * (1.0f - ualfa)));
                  dstPix[3] = (PIX)((_color2.a * ualfa) + (_color1.a * (1.0f - ualfa)));
              }
              else
              {
                  dstPix[0] = (PIX)(((_color2.r * ualfa) + (_color1.r * (1.0f - ualfa))) * max);
                  dstPix[1] = (PIX)(((_color2.g * ualfa) + (_color1.g * (1.0f - ualfa))) * max);
                  dstPix[2] = (PIX)(((_color2.b * ualfa) + (_color1.b * (1.0f - ualfa))) * max);
                  dstPix[3] = (PIX)(((_color2.a * ualfa) + (_color1.a * (1.0f - ualfa))) * max);
              }

              dstPix += nComponents;
          }
      }



      SelectObject(l_tdc, oldfont);
      SelectObject(l_tdc, TmpObj);
      SetBkColor(l_tdc, oldbackground);
      SetBkMode(l_tdc, oldbkmode);
      SetTextColor(l_tdc, oldcolor);

      SelectObject(l_tdc, TmpObj);
      DeleteObject(fontNew);
      DeleteObject(TmpBmp);
      DeleteDC(l_tdc);
      ReleaseDC(0, l_pdc);





  }

};

std::string  formPLCharacters(std::string in)
{
    std::string out;
    size_t j = 0;
    for (size_t i = 0; i < in.size(); ++i)
    {
        if (i < in.size() - 1)
        {
            if (in.at(i) == (char)0xc5)
            {
                ++i;
                if (in.at(i) == (char)0x82)
                {
                    out += "³";
                }
                if (in[i] == (char)0x84)
                {
                    out += "ñ";
                }
                if (in[i] == (char)0x9b)
                {
                    out += "œ";
                }
                if (in[i] == (char)0xbc)
                {
                    out += "¿";
                }
                if (in[i] == (char)0xba)
                {
                    out += "Ÿ";
                }
                if (in[i] == (char)0x81)
                {
                    out += "£";
                }
                if (in[i] == (char)0x83)
                {
                    out += "Ñ";
                }
                if (in[i] == (char)0x9a)
                {
                    out += "Œ";
                }
                if (in[i] == (char)0xbb)
                {
                    out += "¯";
                }
                if (in[i] == (char)0xb9)
                {
                    out += "";
                }

            }
            else if (in[i] == (char)0xc3)
            {
                ++i;
                if (in[i] == (char)0xb3)
                {
                    out += "ó";
                }
                if (in[i] == (char)0x93)
                {
                    out += "Ó";
                }
            }
            else if (in.at(i) == (char)0xc4)
            {
                ++i;
                if (in.at(i) == (char)0x85)
                {
                    out += "¹";
                }
                if (in[i] == (char)0x87)
                {
                    out += "æ";
                }
                if (in[i] == (char)0x99)
                {
                    out += "ê";
                }
                if (in[i] == (char)0x84)
                {
                    out += "¥";
                }
                if (in[i] == (char)0x86)
                {
                    out += "Æ";
                }
                if (in[i] == (char)0x98)
                {
                    out += "Ê";
                }
            }
            else
                out += in[i];
        }
        else
        {
            out += in[i];
        }

    }
    return (out);
}


/** @brief templated class to blend between two images */
template <class PIX, int nComponents, int max>
class CompuTypeGeneratorBGRA : public CompuTypeGeneratorBase {
public :
  // ctor
  CompuTypeGeneratorBGRA(OFX::ImageEffect &instance) 
    : CompuTypeGeneratorBase(instance)
  {}

  // and do some processing
  void multiThreadProcessImages(OfxRectI procWindow)
  {
    // push pixels

    OfxRectI l_imgbounds = _dstImg->getBounds();

    ULONG uXSize = l_imgbounds.x2 - l_imgbounds.x1;
    ULONG uYSize = l_imgbounds.y2 - l_imgbounds.y1;

    bool  l_boldfont = false;
    bool  l_italicfont = false;
    bool  l_underfont = false;
    char l_fontName[1024];
    float l_rectWidth = 0.5;
    float l_rectHeight = 0.5;
    float l_rectTop = (float)_gridOffset.y /100.0f;
    float l_rectLeft = (float)_gridOffset.x /100.0f;

    char l_destTextBuf[4096];

    strcpy(l_fontName, _fontName.c_str());

    strcpy(l_destTextBuf, _displText.c_str());

    // display prepared text
    BITMAPINFO l_binff;
    memset(&l_binff.bmiHeader, 0, sizeof(BITMAPINFOHEADER));
    l_binff.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    l_binff.bmiHeader.biPlanes = 1;
    l_binff.bmiHeader.biWidth = uXSize;
    l_binff.bmiHeader.biHeight = uYSize + 2;
    l_binff.bmiHeader.biBitCount = 32;

    HDC l_pdc = ::GetDC(0);
    HDC l_tdc = CreateCompatibleDC(l_pdc);



    BYTE* pBase = NULL;
    HBITMAP TmpBmp = CreateDIBSection(l_pdc, &l_binff, DIB_RGB_COLORS, (void**)&pBase, 0, 0);
    HGDIOBJ TmpObj = SelectObject(l_tdc, TmpBmp);

    // get  font size
    int l_fontSize = (int)(_tilewidth);
    // in case zero font size
    if (l_fontSize <= 0)
        l_fontSize = 1;


    HFONT fontNew = CreateFont(
        l_fontSize, // height (negative means use character heights
        0, // width (0 means use default)
        0, // escapement (0 means none)
        0, // orientation (0 means none)
        (l_boldfont) ? FW_NORMAL : FW_BOLD, // "boldness" of font
        l_italicfont, // italic?  true or false
        l_underfont, // underline? true or false
        false, // strikeout?  true or false
        DEFAULT_CHARSET, // desired character set
        OUT_TT_PRECIS, // output precision - use TrueType only
        CLIP_DEFAULT_PRECIS, // clip precision - use default
        ANTIALIASED_QUALITY, // proof quality
        DEFAULT_PITCH | FF_DONTCARE, // pitch and family
        l_fontName // name of font face desired
    );


    HFONT oldfont = (HFONT)SelectObject(l_tdc, fontNew);

    COLORREF oldbackground = SetBkColor(l_tdc, COLORREF(RGB(0, 0, 0)));
    COLORREF oldcolor;
    oldcolor = SetTextColor(l_tdc, COLORREF(RGB(255, 255, 255)));
    int oldbkmode = SetBkMode(l_tdc, TRANSPARENT);



    RECT Regiontymcz;
    Regiontymcz.top = (LONG)( uYSize - _gridOffset.y );
    Regiontymcz.left = (LONG)(_gridOffset.x);
    Regiontymcz.bottom = Regiontymcz.top + (LONG)(uYSize);
    Regiontymcz.right = Regiontymcz.left + (LONG)(uXSize);

    int stat = DrawText(l_tdc, l_destTextBuf, strlen(l_destTextBuf), &Regiontymcz, DT_NOCLIP);

    long l_ssx = l_binff.bmiHeader.biWidth;


    for (int y = procWindow.y1; y < procWindow.y2; y++)
    {
        if (_effect.abort()) break;

        PIX* dstPix = (PIX*)_dstImg->getPixelAddress(procWindow.x1, y);

        for (int x = procWindow.x1; x < procWindow.x2; x++)
        {
//            BYTE  bAlfa = pBase[((uYSize - y) * l_ssx * 4) + (x * 4)];
            BYTE  bAlfa = pBase[( y * l_ssx * 4) + (x * 4)];

            float ualfa = (float)(bAlfa) / 255.0f;
            if (max == 1) // implies floating point, so don't clamp
            {
                // bgr space
                dstPix[0] = (PIX)((_color2.b * ualfa) + (_color1.b * (1.0f - ualfa)));
                dstPix[1] = (PIX)((_color2.g * ualfa) + (_color1.g * (1.0f - ualfa)));
                dstPix[2] = (PIX)((_color2.r * ualfa) + (_color1.r * (1.0f - ualfa)));
                dstPix[3] = (PIX)((_color2.a * ualfa) + (_color1.a * (1.0f - ualfa)));
            }
            else
            {
                dstPix[0] = (PIX)(((_color2.b * ualfa) + (_color1.b * (1.0f - ualfa))) * max);
                dstPix[1] = (PIX)(((_color2.g * ualfa) + (_color1.g * (1.0f - ualfa))) * max);
                dstPix[2] = (PIX)(((_color2.r * ualfa) + (_color1.r * (1.0f - ualfa))) * max);
                dstPix[3] = (PIX)(((_color2.a * ualfa) + (_color1.a * (1.0f - ualfa))) * max);
            }

            dstPix += nComponents;
        }
    }



    SelectObject(l_tdc, oldfont);
    SelectObject(l_tdc, TmpObj);
    SetBkColor(l_tdc, oldbackground);
    SetBkMode(l_tdc, oldbkmode);
    SetTextColor(l_tdc, oldcolor);

    SelectObject(l_tdc, TmpObj);
    DeleteObject(fontNew);
    DeleteObject(TmpBmp);
    DeleteDC(l_tdc);
    ReleaseDC(0, l_pdc);





  }

};

////////////////////////////////////////////////////////////////////////////////
/** @brief The plugin that does our work */
class CompuTypePlugin : public OFX::ImageEffect 
{
protected :
  // do not need to delete these, the ImageEffect is managing them for us
  OFX::Clip *dstClip_;

  OFX::DoubleParam   *width_;
  OFX::DoubleParam   *height_;
  OFX::BooleanParam  *square_;
  OFX::RGBAParam     *color1_;
  OFX::RGBAParam     *color2_;
  OFX::Double2DParam *gridPosition_;
  OFX::StringParam   *dispText_;
  OFX::StringParam   *cursor_;
  OFX::DoubleParam   * progress_;
  OFX::DoubleParam   * blinkfreq_;
  OFX::StringParam   * fontName_;

  

public :
  /** @brief ctor */
  CompuTypePlugin(OfxImageEffectHandle handle)
    : ImageEffect(handle)
    , dstClip_(0)
    , width_(0)
    , height_(0)
    , square_(0)
    , color1_(0)
    , color2_(0)
    , gridPosition_(0)
    , dispText_(0)
    , cursor_(0)
    , progress_(0)
      , blinkfreq_(0)
      , fontName_(0)
  {
    dstClip_  = fetchClip("Output");
    width_                = fetchDoubleParam("Width");
    height_               = fetchDoubleParam("Height");
    color1_               = fetchRGBAParam("Color1");
    color2_               = fetchRGBAParam("Color2");
    gridPosition_         = fetchDouble2DParam("GridPosition");
    dispText_             = fetchStringParam("DispText");
    cursor_               = fetchStringParam("Cursor");
    progress_             = fetchDoubleParam("Progress");
    blinkfreq_            = fetchDoubleParam("BlinkFreq");

    fontName_             = fetchStringParam("FontName");

  }

  /* Override the render */
  virtual void render(const OFX::RenderArguments &args);

  /* override changedParam */
  virtual void changedParam(const OFX::InstanceChangedArgs &args, const std::string &paramName);

  /* Override the clip preferences, we need to say we are setting the frame varying flag */
  virtual void getClipPreferences(OFX::ClipPreferencesSetter &clipPreferences);

  /* set up and run a processor */
  void setupAndProcess(CompuTypeGeneratorBase &, const OFX::RenderArguments &args);

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
CompuTypePlugin::setupAndProcess(CompuTypeGeneratorBase &processor, const OFX::RenderArguments &args)
{
  // get a dst image
  std::auto_ptr<OFX::Image>  dst(dstClip_->fetchImage(args.time));
  OFX::BitDepthEnum         dstBitDepth    = dst->getPixelDepth();
  OFX::PixelComponentEnum   dstComponents  = dst->getPixelComponents();
  OfxRectI                  dstBounds      = dst->getBounds();

  // set the images
  processor.setDstImg(dst.get());

  // set the render window
  processor.setRenderWindow(args.renderWindow);

  // set parameter values
  double width = width_->getValueAtTime(args.time);
  double height = height_->getValueAtTime(args.time);
  int pixelwidth = (int)(width * (dstBounds.x2 - dstBounds.x1));
  int pixelheight = (int)(height * (dstBounds.y2 - dstBounds.y1));
  if(pixelwidth == 0) pixelwidth = 1;
  if(pixelheight == 0) pixelheight = 1;

  processor.setTileDimensions(pixelwidth, pixelheight);
  RGBAValue color;
  color1_->getValueAtTime(args.time, color.r, color.g, color.b, color.a);
  processor.setColor1(color.r, color.g, color.b, color.a);
  color2_->getValueAtTime(args.time, color.r, color.g, color.b, color.a);
  processor.setColor2(color.r, color.g, color.b, color.a);
  double ox = 0.0;
  double oy = 0.0;
  gridPosition_->getValueAtTime(args.time, ox, oy);
  processor.setGridOffset((int)(ox * (dstBounds.x2 - dstBounds.x1)), (int)(oy * (dstBounds.y2 - dstBounds.y1)));
  std::string l_text;
  dispText_->getValueAtTime(args.time, l_text);
  std::string l_cursor;
  cursor_->getValueAtTime(args.time, l_cursor);

  l_text = formPLCharacters(l_text);  // correct polish language characters
 
  double l_progress = progress_->getValueAtTime(args.time);
  size_t destLen = (size_t)(fabs((double)(l_text.size()) * l_progress));
  
  double l_blinkfreq =  blinkfreq_->getValueAtTime(args.time);
  // time is in ms in local time event (from 0)
  int blinkingperiod = (int)(l_blinkfreq * 1000.0);

  if (blinkingperiod == 0)
  {
      processor.setText(l_text.substr(0, destLen) + l_cursor);
  }
  else
  {
      int timeinPeriod = (int)args.time % blinkingperiod;
      int halfperiod = blinkingperiod / 2;
      if (timeinPeriod > halfperiod)
      {
          processor.setText(l_text.substr(0, destLen));
      }
      else
      {
          processor.setText(l_text.substr(0, destLen) + l_cursor);
      }
  }
  std::string l_fntame;
  fontName_->getValueAtTime(args.time, l_fntame);
  if (l_fntame.size() == 0)
  {
      l_fntame = "Arial";
  }
  processor.setFontName(l_fntame);
//  processor.setProgress(l_progress);
  // Call the base class process member, this will call the derived templated process code
  processor.process();
}

/* Override the clip preferences, we need to say we are setting the frame varying flag */
void 
CompuTypePlugin::getClipPreferences(OFX::ClipPreferencesSetter &clipPreferences)
{
  clipPreferences.setOutputFrameVarying(true);
}

/** @brief The get RoD action.  We flag an infinite rod */
bool 
CompuTypePlugin::getRegionOfDefinition(const OFX::RegionOfDefinitionArguments &args, OfxRectD &rod)
{
  // we can generate noise anywhere on the image plan, so set our RoD to be infinite
  rod.x1 = rod.y1 = -std::numeric_limits<double>::infinity(); // kOfxFlagInfiniteMin;
  rod.x2 = rod.y2 = std::numeric_limits<double>::infinity(); // kOfxFlagInfiniteMax;
  return true;
}

// the overridden render function
void
CompuTypePlugin::render(const OFX::RenderArguments &args)
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
              CompuTypeGeneratorBGRA<unsigned char, 4, 255> fred(*this);
              setupAndProcess(fred, args);
            }
            break;

        case OFX::eBitDepthUShort : 
          {
            CompuTypeGeneratorBGRA<unsigned short, 4, 65535> fred(*this);
            setupAndProcess(fred, args);
          }                          
          break;

        case OFX::eBitDepthFloat : 
          {
            CompuTypeGeneratorBGRA<float, 4, 1> fred(*this);
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
              CompuTypeGenerator<unsigned char, 4, 255> fred(*this);
              setupAndProcess(fred, args);
            }
            break;

        case OFX::eBitDepthUShort : 
          {
            CompuTypeGenerator<unsigned short, 4, 65535> fred(*this);
            setupAndProcess(fred, args);
          }                          
          break;

        case OFX::eBitDepthFloat : 
          {
            CompuTypeGenerator<float, 4, 1> fred(*this);
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
        CompuTypeGenerator<unsigned char, 1, 255> fred(*this);
        setupAndProcess(fred, args);
      }
      break;

    case OFX::eBitDepthUShort : 
      {
        CompuTypeGenerator<unsigned short, 1, 65536> fred(*this);
        setupAndProcess(fred, args);
      }                          
      break;

    case OFX::eBitDepthFloat : 
      {
        CompuTypeGenerator<float, 1, 1> fred(*this);
        setupAndProcess(fred, args);
      }                          
      break;
    }
  } 
}

// we have changed a param
void
CompuTypePlugin::changedParam(const OFX::InstanceChangedArgs &args, const std::string &paramName)
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
void CompuTypePlugin::upliftVegasKeyframes(const OFX::SonyVegasUpliftArguments &upliftInfo)
{
    void*  pvData0 = upliftInfo.getKeyframeData     (0);
    int    ccData0 = upliftInfo.getKeyframeDataSize (0);
    double dData0  = upliftInfo.getKeyframeTime     (0);

    if(ccData0 != sizeof(CHECKR_PROPS))
        return;

    CHECKR_PROPS* pProps0 = (CHECKR_PROPS*)pvData0;

    width_->setValue(pProps0->dTileWidth);
    height_->setValue(pProps0->dTileHeight);
    color1_->setValue(pProps0->foreColor.r, pProps0->foreColor.g, pProps0->foreColor.b, pProps0->foreColor.a);
    color2_->setValue(pProps0->backColor.r, pProps0->backColor.g, pProps0->backColor.b, pProps0->backColor.a);
    gridPosition_->setValue(pProps0->dTileOffsetX, pProps0->dTileOffsetY);

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
        if(pProps0->dTileHeight != pPropsN->dTileHeight)
        {
            if(! fHeightAnimates)
                height_->setValueAtTime(dData0, pProps0->dTileHeight);
            height_->setValueAtTime(dDataN, pPropsN->dTileHeight);
            fHeightAnimates = true;
        }
        if(pProps0->bSymmetricTiles != pPropsN->bSymmetricTiles)
        {
            if(! fSquareAnimates)
                square_->setValueAtTime(dData0, pProps0->bSymmetricTiles);
            square_->setValueAtTime(dDataN, pPropsN->bSymmetricTiles);
            fSquareAnimates = true;
        }
        if((pProps0->foreColor.r != pPropsN->foreColor.r) ||
            (pProps0->foreColor.g != pPropsN->foreColor.g) ||
            (pProps0->foreColor.b != pPropsN->foreColor.b) ||
            (pProps0->foreColor.a != pPropsN->foreColor.a))
        {
            if(! fColor1Animates)
                color1_->setValueAtTime(dData0, pProps0->foreColor.r, pProps0->foreColor.g, pProps0->foreColor.b, pProps0->foreColor.a);
            color1_->setValueAtTime(dDataN, pPropsN->foreColor.r, pPropsN->foreColor.g, pPropsN->foreColor.b, pPropsN->foreColor.a);
            fColor1Animates = true;
        }
        if((pProps0->backColor.r != pPropsN->backColor.r) ||
            (pProps0->backColor.g != pPropsN->backColor.g) ||
            (pProps0->backColor.b != pPropsN->backColor.b) ||
            (pProps0->backColor.a != pPropsN->backColor.a))
        {
            if(! fColor2Animates)
                color2_->setValueAtTime(dData0, pProps0->backColor.r, pProps0->backColor.g, pProps0->backColor.b, pProps0->backColor.a);
            color2_->setValueAtTime(dDataN, pPropsN->backColor.r, pPropsN->backColor.g, pPropsN->backColor.b, pPropsN->backColor.a);
            fColor2Animates = true;
        }
        if((pProps0->dTileOffsetX != pPropsN->dTileOffsetX) ||
            (pProps0->dTileOffsetY != pPropsN->dTileOffsetY))
        {
            if(! fPositionAnimates)
                gridPosition_->setValueAtTime(dData0, pProps0->dTileOffsetX, pProps0->dTileOffsetY);
            gridPosition_->setValueAtTime(dDataN, pPropsN->dTileOffsetX, pPropsN->dTileOffsetY);
            fPositionAnimates = true;
        }

        pProps0 = pPropsN;
        dData0 = dDataN;
    }
}


mDeclarePluginFactory(CompuTypeExamplePluginFactory, {}, {});

using namespace OFX;

void CompuTypeExamplePluginFactory::describe(OFX::ImageEffectDescriptor &desc) 
{
  desc.setLabels("OFX CompuType", "OFX CompuType", "OFX CompuType");
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

void CompuTypeExamplePluginFactory::describeInContext(OFX::ImageEffectDescriptor &desc, ContextEnum context) 
{
  ClipDescriptor *dstClip = desc.defineClip("Output");
  dstClip->addSupportedComponent(ePixelComponentRGBA);
  dstClip->addSupportedComponent(ePixelComponentAlpha);
  dstClip->setSupportsTiles(true);
  dstClip->setFieldExtraction(eFieldExtractSingle);

  DoubleParamDescriptor *widthParam = desc.defineDoubleParam("Width");
  widthParam->setLabels("Size", "Size", "Size");
  widthParam->setScriptName("width");
  widthParam->setHint("Font size.");
  widthParam->setDefault(0.1);
  widthParam->setRange(0, 1);
  widthParam->setIncrement(0.1);
  widthParam->setDisplayRange(0, 1);
  widthParam->setAnimates(true); // can animate
  widthParam->setDoubleType(eDoubleTypeScale);

  DoubleParamDescriptor *heightParam = desc.defineDoubleParam("Height");
  heightParam->setLabels("Height", "Height", "Height");
  heightParam->setScriptName("height");
  heightParam->setHint("Height of the tile.");
  heightParam->setDefault(0.1);
  heightParam->setRange(0, 1);
  heightParam->setIncrement(0.1);
  heightParam->setDisplayRange(0, 1);
  heightParam->setEnabled(false);
  heightParam->setAnimates(true); // can animate
  heightParam->setDoubleType(eDoubleTypeScale);

  RGBAParamDescriptor* color1Param = desc.defineRGBAParam("Color1");
  color1Param->setLabels("Color 1", "Color 1", "Color 1");
  color1Param->setScriptName("color1");
  color1Param->setHint("CompuType color 1.");
  color1Param->setDefault(0.0, 0.0, 0.0, 1.0);
  color1Param->setAnimates(true); // can animate

  RGBAParamDescriptor* color2Param = desc.defineRGBAParam("Color2");
  color2Param->setLabels("Color 2", "Color 2", "Color 2");
  color2Param->setScriptName("color2");
  color2Param->setHint("CompuType color 2.");
  color2Param->setDefault(1.0, 1.0, 1.0, 1.0);
  color2Param->setAnimates(true); // can animate

  Double2DParamDescriptor* gridPositionParam = desc.defineDouble2DParam("GridPosition");
  gridPositionParam->setLabels("Text Position", "Text Position", "Text Position");
  gridPositionParam->setScriptName("gridPosition");
  gridPositionParam->setHint("XY text position.");
  gridPositionParam->setDefault(0.0, 1.0);
  gridPositionParam->setRange(0.0, 0.0, 1.0, 1.0);
  gridPositionParam->setAnimates(true); // can animate

  StringParamDescriptor* fontName = desc.defineStringParam("FontName");
  fontName->setLabels("Font Name", "Font Name", "Font Name");
  fontName->setScriptName("DispText");
  fontName->setHint("Font name.");
  fontName->setDefault("Arial");
  fontName->setStringType(eStringTypeSingleLine);
  fontName->setAnimates(true); // can animate


  StringParamDescriptor* textParam = desc.defineStringParam("DispText");
  textParam->setLabels("Text", "Text", "Text");
  textParam->setScriptName("DispText");
  textParam->setHint("Display text.");
  textParam->setDefault("Example");
  textParam->setStringType(eStringTypeMultiLine);
  textParam->setAnimates(true); // can animate

  StringParamDescriptor* cursorParam = desc.defineStringParam("Cursor");
  cursorParam->setLabels("Cursor", "Cursor", "Cursor");
  cursorParam->setScriptName("cursor");
  cursorParam->setHint("Cursor string.");
  cursorParam->setDefault("_");
  cursorParam->setAnimates(true); // can animate

  DoubleParamDescriptor* blinkfrqParam = desc.defineDoubleParam("BlinkFreq");
  blinkfrqParam->setLabels("Blinking freq.", "Blinking freq.", "Blinking freq.");
  blinkfrqParam->setScriptName("BlinkFreq");
  blinkfrqParam->setHint("Cursor blinking frequency.");
  blinkfrqParam->setDefault(1.0);
  blinkfrqParam->setRange(0, 1);
  blinkfrqParam->setIncrement(0.1);
  blinkfrqParam->setDisplayRange(0, 1);
  blinkfrqParam->setAnimates(true); // can animate
  blinkfrqParam->setDoubleType(eDoubleTypeScale);



  DoubleParamDescriptor* progressParam = desc.defineDoubleParam("Progress");
  progressParam->setLabels("Progress", "Progress", "Progress");
  progressParam->setScriptName("progress");
  progressParam->setHint("Typing progress.");
  progressParam->setDefault(1.0);
  progressParam->setRange(0, 1);
  progressParam->setIncrement(0.1);
  progressParam->setDisplayRange(0, 1);
  progressParam->setAnimates(true); // can animate
  progressParam->setDoubleType(eDoubleTypeScale);

  PageParamDescriptor *page = desc.definePageParam("Controls");
  page->addChild(*widthParam);
  page->addChild(*heightParam);
  page->addChild(*fontName);
  page->addChild(*color1Param);
  page->addChild(*color2Param);
  page->addChild(*gridPositionParam);
  page->addChild(*textParam);
  page->addChild(*cursorParam);
  page->addChild(*blinkfrqParam);
  page->addChild(*progressParam);

  desc.addVegasUpgradePath("{DB33DAB1-5247-4324-B776-E0CCAD45BE14}");
}

ImageEffect* CompuTypeExamplePluginFactory::createInstance(OfxImageEffectHandle handle, ContextEnum context)
{
  return new CompuTypePlugin(handle);
}

namespace OFX
{
  namespace Plugin
  {
    void getPluginIDs(OFX::PluginFactoryArray &ids)
    {
      static CompuTypeExamplePluginFactory p("com.VMRexor:CompuTyper", 1, 0);
      ids.push_back(&p);
    }
  };
};
