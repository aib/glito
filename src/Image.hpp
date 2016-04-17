// glito/Image.hpp  v1.1  2004.09.05
/* Copyright (C) 1996, 2002-2004 Emmanuel Debanne
  
   This file is part of Glito.
   Glito is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2 of the License, or (at your
option) any later version.
   Glito is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.
   You should have received a copy of the GNU General Public License
along with Glito (named COPYING); if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
USA.
*/

#ifndef IMAGE_HPP
#define IMAGE_HPP

#include "ImageGray.hpp"

class ElementColorMap {
public:
    ElementColorMap() : c(0), r(0), g(0), b(0) {
    }
    ElementColorMap(float c, float r, float g, float b) : c(c), r(r), g(g), b(b) {
    }
    float c;
    float r;
    float g;
    float b;
};

/**
 * 8 bits image with utilities to hit pixels.
 */
class Image : public ImageGray {
public:
    /// empty constructor: set all the variables to 0
    Image();

    Image( int w, int h, bool color, int wd = -1, int wh = -1, int start = -1 );

    ~Image();

    Image( const Image& other ) { copy(other); }

    Image& operator=( const Image& other );

    /// put (i,j) in tab. range chcking
    virtual void mem_plot( int i, int j );
  
    virtual void mem_coul( int i, int j, float c );

    /// return number of hit. used for julia orbits
    virtual int getHit( int i, int j ) const;

    /// draw the image in memory (tab)
    virtual void mem_draw() const;

    /// test if tab contains only 0. for debug purpose
    virtual bool isEmpty() const;

    /// fill tab with 0 or 255
    virtual void mem_clear();

    /** change the offset #start#
        Used to change the part of the image to crop.
     */
    void shiftStart( const int shiftX, const int shiftY );

    static void readColorMap( const std::string& colorText );

    static void readDefinedMap( const int map );

protected:
    void copy( const Image& other );

    float* rgbTab;

    virtual void plotColor( int n, float r, float g, float b );

private:
    static std::vector<ElementColorMap> colorMap;

    /** offset to apply when cropping the image
	the first pixel of the image (*(tab-start)) is drawn at point (0, 0)
    */
    int start;

    // { size of the part of the image that we can draw to the screen
    int wDraw;
    int hDraw;
    // }

    /** true if we need to crop the image to draw it
	true if wDraw < width && hDraw < height;
    */
    bool crop;

};

class PseudoDensity {
public:
    /// constructor. set logProbaHitMax to log(10000)
    PseudoDensity();

    void setLogProbaHitMax( float logProba );
    float getLogProbaHitMax() const { return logProbaHitMax; }

    // { depends on RAND_MAX
    /// @param proba between 0 and 1
    void setProba( float proba );
    float getProba() const;
    // }

    /// return true if a pixel of color #c# should be incremented
    bool plot( unsigned char c ) const {
	return rand() * plotProbability[c] < RAND_MAX;
    }

private:
    /** to be incremented from 254 to 255, a pixel must be hit
	exp(logProbaHitMax) times
	logProbaHitMax = proba * log(RAND_MAX)
    */
    float logProbaHitMax;

    float plotProbability[256];

};

class ImagePseudoDensity : public Image {
public:
    static PseudoDensity pseudoDensity;
    
    ImagePseudoDensity( int w, int h, bool c, int wd = -1, int hd = -1, int s = -1 );

    /// copy constructor
    ImagePseudoDensity( const ImagePseudoDensity& other ) { copy(other); }

    /// constructor by affectation. frees the previous hitTab.
    ImagePseudoDensity& operator=( const ImagePseudoDensity& other );

    void mem_plot( int i, int j );

    virtual void plotColor( int n, float r, float g, float b );

    void mem_clear();

    void mem_draw() const;

private:
    void resetLimitGray() { limitGray =  background.isBlack() ? 127 : 255-127; }
    
    /// color of a pixel when it is reached for the first time
    int limitGray;

    void copy( const ImagePseudoDensity& other );

};

/** the color of a pixel is set according to the number of times it was
    reached during the computation (number of hit).
    The formula applied is:
    gray = 255 * log( 1 + hit) / log( 1 + maxHit )
*/
class ImageDensity : public Image {
public:
    /// create an image of size w*h
    ImageDensity( int w, int h, bool color, int wd = -1, int wh = -1, int s = -1);

    /// copy constructor. hitTab is copied.
    ImageDensity( const ImageDensity& other ) { copy(other); }

    /// free hitTab automatically
    ~ImageDensity();
    
    /// constructor by affectation. frees the previous hitTab.
    ImageDensity& operator=( const ImageDensity& other );

    /// put (i,j) in hitTab. range chcking
    void mem_plot( int i, int j );

    virtual void plotColor( int n, float r, float g, float b );

    /// return number of hit. used for julia orbits
    int getHit( int i, int j ) const;

    /// test if hitTab contains only 0. for debug purpose
    bool isEmpty() const;

    /// fill hitTab and tab with 0
    void mem_clear();

    /// build tab from hitTab and draw it
    void mem_draw() const;

private:
    /// number of hit for each pixel
    int* hitTab;

    /// called by constructors
    void copy(const ImageDensity& other );

    int maxHit;

    /// max value for maxHit. maxint-1 because pow(1+maxInt,...)
    static const int maxMaxHit;
};

#endif // IMAGE_HPP
