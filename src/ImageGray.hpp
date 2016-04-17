// glito/ImageGray.hpp  v1.1  2004.09.05
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

#ifndef IMAGEGRAY_HPP
#define IMAGEGRAY_HPP

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <vector>
#include <fstream>

#ifdef HAVE_LIBPNG
# include <png.h>
// for version of libpng < 1.0.6
# ifndef png_jmpbuf
#  define png_jmpbuf(png_ptr) ((png_ptr)->jmpbuf)
# endif
#endif

#ifdef HAVE_LIBMNG
# define MNG_SUPPORT_WRITE
# define MNG_ACCESS_CHUNKS
# include <libmng.h>
#endif

#include <FL/Fl_Progress.H>

class Progress {
public:
    Progress( const char* title, const int max );

    ~Progress();

    // updates the progress bar
    void setValue( const float f );

private:
    Fl_Window* w;

    Fl_Progress* p;

    static const int border = 10;

    static const int height = 50;

    static const int width = 250;

};

/** blackBackground = Black to White or White to Black palette.
 */
class Background {
public:
    /// constructor
    Background() { setBlack(true); }

    /// change empty and full too
    void setBlack( const bool b );
    bool isBlack() const { return blackBackground; }

    unsigned char getEmpty() const { return empty; }
    unsigned char getFull() const { return full; }

private:
    /// when hit the maximum times
    unsigned char full;
    /// not hit yet
    unsigned char empty;
    
    /// true if the background of an image is black
    bool blackBackground;

};

/** alpha, simple = transparency.
 */
class Transparency {
public:
    /// constructor. none transparency
    Transparency();

    // { set  transparency
    void setNoTransparency() { simpleTransparency = false; alphaTransparency = false; }
    void setSimpleTransparency() { simpleTransparency = true; alphaTransparency = false; }
    void setAlphaTransparency() { simpleTransparency = false; alphaTransparency = true; }
    // }

    // { access transparency
    bool useSimpleTransparency() const { return simpleTransparency; }
    bool useAlphaTransparency() const { return alphaTransparency; }
    // }

    // { to save transparency parameter to an XML file and to recover it
    std::string transparencyToXML() const;
    void setTransparencyFromXML( const std::string& s );
    //}

    // { to save in XML and for the menu in the window of the parameters
    /// "one color"
    static const std::string xmlSimple;
    /// "alpha"
    static const std::string xmlAlpha;
    /// "none"
    static const std::string xmlNone;
    // }

    // {
    /// "one color" translated
    std::string stringSimple() const;
    /// "alpha" translated
    std::string stringAlpha() const;
    /// "none" translated
    std::string stringNone() const;
    // }

private:
    /// #empty# color is the transparent color
    bool simpleTransparency;
    /// the gray level determines the opacity of the pixel
    bool alphaTransparency;

};

class Image;

/**
 * 8 bits image with utilities to save it to different formats
 */
class ImageGray {
public:
    /// for description in PNG and MNG files
    static const std::string software;

    static Background background;

    static Transparency transparency;

    enum imageFormat {
	PGM,
	BMPB, // BMP bitmap
	BMPG, // BMP gray
	PNG,  // gray 8bits
	MNG   // gray 8bits or gray_alpha
    };

    /// used by Main.cpp
    static const std::string formatToString( imageFormat f );
 
    /// empty constructor: set all the variables to 0
    ImageGray();

    /// create an image of size w*h
    ImageGray( int w, int h, bool color = false );

    /// copy constructor. Tab is copied.
    ImageGray( const ImageGray& other );

    /// free tab automatically
    virtual ~ImageGray();
    
    /// constructor by affectation. frees the previous tab.
    ImageGray& operator=( const ImageGray& other );

    // { access to private data
    int w() const { return width; }
    int h() const { return height; }
    // }

    /// save the image to a file stream #f# under format #format#
    void save( std::ofstream& f, imageFormat format ) const;

#ifdef HAVE_LIBPNG
    /// write a PNG image. returns 1 if succeed, 0 if failed
    int savePNG( FILE* f, const std::string& description ) const;

    /** read the description contained in a PNG file
     * @throw 1 if the file is not a PNG file
     * @throw 2 if no description is found
     */
    static const std::string getDescriptionFromPNG( const std::string& file );
#endif // HAVE_LIBPNG

#ifdef HAVE_LIBMNG
    /// write a MNG animation. returns 1 if succeed, 0 if failed
    static int saveMNG( const std::vector<Image*>& images, FILE* f,
			const int framesPerSecond, const std::string& description );
#endif // HAVE_LIBMNG

    bool isColored() const { return colored; }
 
protected:
    /// called by constructors
    void copy(const ImageGray& other );

#ifdef HAVE_LIBMNG
    int addChunks( mng_handle myhandle ) const;
#endif

    // { save tab to f. the header of the file has already been saved. 
    void save_addition_pgm( std::ofstream& f ) const;
    void save_addition_bmpb( std::ofstream& f ) const;
    void save_addition_bmpg( std::ofstream& f ) const;
    // }

    /// image in gray levels
    unsigned char* tab;

    /// image in color levels
    unsigned char* colorTab;

    bool colored;

    /// if memory is allocated to tab
    bool allocated;

    int width;

    int height;

    /// width*height
    int sizePixels;
};

#endif // IMAGEGRAY_HPP
