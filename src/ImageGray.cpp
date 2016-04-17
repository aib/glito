// glito/ImageGray.cpp  v1.1  2004.09.05
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

#ifdef CROSS
# include <memory>
#endif

#include <iostream>
#include <cassert>

#include <FL/Fl_Window.H>
#include <FL/Fl.H>
// wait

#include "ImageGray.hpp"
#include "Image.hpp"

#ifdef ENABLE_NLS
# include <libintl.h>
# define _(String) gettext (String)
#else
# define _(String) (String)
#endif

Progress::Progress( const char* title, const int max ) {
    w = new Fl_Window( width, height, title );
    p = new Fl_Progress( border, border, width - 2*border, height - 2*border );
    p->minimum(0);
    p->maximum(max);
    w->end();
    w->show();
}

Progress::~Progress() {
    // we close the window
    delete w;
}

void
Progress::setValue( const float f ) {
    p->value(f);
    Fl::wait(0);
}

const std::string
Transparency::xmlSimple = "one color";
const std::string
Transparency::xmlAlpha = "alpha";
const std::string
Transparency::xmlNone = "none";

std::string
Transparency::stringSimple() const { return _("one color"); }
std::string
Transparency::stringAlpha() const { return _("alpha"); }
std::string
Transparency::stringNone() const { return _("none"); }

Transparency::Transparency() {
    setNoTransparency(); // set alphaTransparency, simpleTransparency
}

void
Background::setBlack( const bool b ) {
    blackBackground = b;
    if ( blackBackground ) {
	full = 255;
	empty = 0;
    } else {
	full = 0;
	empty = 255;
    }
}

std::string
Transparency::transparencyToXML() const {
    if ( simpleTransparency ) {
	return xmlSimple;
    } else if ( alphaTransparency ) {
	 return xmlAlpha;
    } else {
	return xmlNone;
    }
}

void
Transparency::setTransparencyFromXML( const std::string& s ) {
    if ( s == xmlSimple ) {
	setSimpleTransparency();
    } else if ( s == xmlAlpha ) {
	setAlphaTransparency();
    } else {
	setNoTransparency();
    }
}

const std::string
ImageGray::software = std::string("Glito (c) 2002-2003 Emmanuel Debanne http://www.debanne.net/glito. ") + _("Glito is free software.");

Background
ImageGray::background = Background();

Transparency
ImageGray::transparency = Transparency();

const std::string
ImageGray::formatToString( imageFormat f ) {
    switch ( f ) {
    case PGM: return "PGM";
    case BMPB: return "BMP bitmap";
    case BMPG: return "BMP gray-level";
    case PNG: return "PNG gray-level";
    case MNG: return "MNG gray-level";
    default: return "unknown";
    }
}

ImageGray::ImageGray() : allocated(false), width(0),
			 height(0), sizePixels(0), colored(false) {
}

ImageGray::ImageGray( int w, int h, bool c )
    : width(w), height(h), allocated(false), sizePixels(w*h), colored(c) {
    tab = (unsigned char*)calloc( sizePixels, sizeof(unsigned char) );
    if ( colored ) {
        colorTab = (unsigned char*)calloc( 3*sizePixels, sizeof(unsigned char) );
    }
    if ( tab == NULL || (colored && colorTab == NULL) ) {
	std::cerr << "Calloc failed in ImageGray::ImageGray(int,int,bool)!\n";
	abort();
    } else {
	allocated = true;
    }
}

ImageGray::ImageGray( const ImageGray& other ) {
    copy(other);
}

ImageGray::~ImageGray() {
    if ( allocated ) {
	free(tab);
	if ( colored ) {
	    free(colorTab);
	}
    }
}

void
ImageGray::copy( const ImageGray& other ) {
    width = other.width;
    height = other.height;
    sizePixels = other.sizePixels;
    colored = other.colored;
    if ( other.allocated ) {
	tab = (unsigned char*)calloc( sizePixels, sizeof(unsigned char) );
	if ( colored ) {
	    colorTab = (unsigned char*)calloc( 3*sizePixels, sizeof(unsigned char) );
	}
	if ( tab == NULL || (colored && colorTab == NULL) ) {
	    std::cerr << "Calloc failed in ImageGray::copy(const ImageGray&)!\n";
	    abort();
	}
	memcpy( tab, other.tab, sizePixels );
	if ( colored ) {
	    memcpy( colorTab, other.colorTab, 3*sizePixels );
	}
	allocated = true;
    } else {
	std::cerr << "Image not built in ImageGray::copy(const ImageGray&)!\n";
	abort();
	allocated = false;
    }
}

ImageGray&
ImageGray::operator=( const ImageGray& other ) {
    if ( allocated ) {
	free(tab);
	if ( colored ) {
	    free(colorTab);
	}
	allocated = false;
    }
    copy(other);
    return *this;
}

void
ImageGray::save( std::ofstream& f, const imageFormat format ) const {
    if ( format == PGM ) {
	f << "P5\n"
	  << "#Generated by Glito http://www.debanne.net/glito\n"
	  << width << ' ' << height << '\n' << "255\n";
	save_addition_pgm( f );
    } else if ( format == BMPB || format == BMPG ) {
	// bmp info: http://www.daubnet.com/formats/BMP.html
	//BEGINING HEADER
	f << "BM";
	int headerSize;
	if ( format == BMPB ) {
	    headerSize = 26 + 2*3;
	    // a line is a multiple of 32 bits. we have to complete by bits
	    // equal to 0 if image.w() is not a mutiple.
	    const int horizontalSizeBits = width + (( 32 - (width%32) )%32);
	    const int fileSize = headerSize + height * horizontalSizeBits / 8;
	    f.write( (char*)&fileSize, sizeof fileSize );
	} else {
	    headerSize = 26 + 256*3;
	    // a line is a multiple of 32 bits. we have to complete by bits
	    // equal to 0 if image.w() is not a mutiple.
	    const int horizontalSizeBytes = width + (( 4 - (width%4) )%4);
	    const int fileSize = headerSize + height * horizontalSizeBytes;
	    f.write( (char*)&fileSize, sizeof fileSize );
	}
	f << (char)0 << (char)0 << (char)0 << (char)0; //unused
	f.write( (char*)&headerSize, sizeof headerSize );
	//INFOHEADER
	const int infoHeaderSize = 12;
	f.write( (char*)&infoHeaderSize, sizeof infoHeaderSize );
	const short imageW = (short)width;
	const short imageH = (short)height;
	f.write( (char*)&imageW, sizeof imageW );
	f.write( (char*)&imageH, sizeof imageH );
	f << (char)1 << (char)0; //number of planes = 1
	if ( format == BMPB ) {
	    f << (char)1 << (char)0; // 1=1bit, 8=8bits, 24=24bits
	    //COLOR TABLE
	    f << (char)0 << (char)0 << (char)0 //white
	      << (char)255 << (char)255 << (char)255; //black
	    save_addition_bmpb( f );
	} else if ( format == BMPG ) {
	    f << (char)8 << (char)0; // 8 bits
	    //COLOR TABLE
	    for ( int i = 0; i < 256; ++i ) {
		f << (char)i << (char)i << (char)i;
	    }
	    save_addition_bmpg( f );
	}
    }
}

void
ImageGray::save_addition_pgm( std::ofstream& f ) const {
    for ( int i = 0; i < sizePixels; ++i ) {
	f << tab[i];
    }
}

void
ImageGray::save_addition_bmpb( std::ofstream& f ) const {
    Progress progress( _("Saving BMP file"), height );
    const int horizontalSizeBits = w() + (( 32 - (w()%32) )%32);
    for ( int y = height-1; y >= 0; --y ) {
	progress.setValue(height-y);
	int x = 0;
	for ( ; x < w(); x+=8 ) {
	    const int i = x + y*w();
	    int c = 0;
	    for ( int k = 0; k < 8; ++k ) {
		if ( x + k < w() ) {
		    c += ( 1<<(7-k) )*( tab[i+k] != 0 );
		}
	    }
	    // bug of mingw: occurs when the two lowest bits of c are not 0
	    f << (unsigned char)c;
	}
	for ( ; x < horizontalSizeBits; x+=8 ) {
	    f << (unsigned char)0;
	}
    }
}

void
ImageGray::save_addition_bmpg( std::ofstream& f ) const {
    Progress progress( _("Saving BMP file"), height );
    const int horizontalSizeBytes = w() + (( 4 - (w()%4) )%4);
    for ( int y = height-1; y >= 0; --y ) {
	progress.setValue(height-y);
	int x = 0;
	for ( ; x < w(); ++x ) {
	    const int i = x + y*w();
	    f << tab[i];
	}
	for ( ; x < horizontalSizeBytes; ++x ) {
	    f << (unsigned char)0;
	}
    }
}

#ifdef HAVE_LIBPNG
int
ImageGray::savePNG( FILE* fp, const std::string& description ) const {
    Progress progress( _("Saving PNG file"), height );
    // origin: example.c of libpng
    png_structp png_ptr;
    png_infop info_ptr;
    
    png_ptr = png_create_write_struct( PNG_LIBPNG_VER_STRING, NULL/*error callback*/,
				       NULL/*error callback*/, NULL/*warning callback*/ );
    if ( png_ptr == NULL ) {
	fclose(fp);
	return 0;
    }
    
    // Allocate/initialize the image information data
    info_ptr = png_create_info_struct(png_ptr);
    if ( info_ptr == NULL ) {
	fclose(fp);
	png_destroy_write_struct( &png_ptr, (png_infopp)NULL );
	return 0;
    }
    
    // Set error handling.  REQUIRED if you aren't supplying your own
    // error handling functions in the png_create_write_struct() call.
    if (setjmp(png_jmpbuf(png_ptr))) {
	// If we get here, we had a problem reading the file
	fclose(fp);
	png_destroy_write_struct( &png_ptr, &info_ptr );
	return 0;
    }
    // I/O initialization
    png_init_io( png_ptr, fp );

    const int bit_depth = 8;
    int color_type;
    if ( transparency.useAlphaTransparency() ) {
	color_type = colored ? PNG_COLOR_TYPE_RGB_ALPHA : PNG_COLOR_TYPE_GRAY_ALPHA;
    } else { // simple or no transparency
	color_type = colored ? PNG_COLOR_TYPE_RGB : PNG_COLOR_TYPE_GRAY;
    }
    png_set_IHDR( png_ptr, info_ptr, width, height, bit_depth, color_type,
		  PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    
    // comments into the image (Title, Author, ...)
    const int text_ptr_size = 2; 
    png_text text_ptr[text_ptr_size];
    text_ptr[0].key = "Software";
    char soft[software.size()+1];
    software.copy( soft, software.size() );
    soft[software.size()] = '\0';
    text_ptr[0].text = soft;
    text_ptr[0].compression = PNG_TEXT_COMPRESSION_NONE;
    text_ptr[1].key = "Description";
    char desc[description.size()+1];
    description.copy( desc, description.size() );
    desc[description.size()]='\0';
    text_ptr[1].text = desc;
    // As it is difficult to find a software which can retrieve a compressed
    // text in a PNG file, compression is not used for the moment.
    // text_ptr[1].compression = PNG_TEXT_COMPRESSION_zTXt;
    text_ptr[1].compression = PNG_TEXT_COMPRESSION_NONE;
    png_set_text( png_ptr, info_ptr, text_ptr, text_ptr_size );
    
    if ( transparency.useSimpleTransparency() ) {
	// transparent color:
	png_color_16 transparent;
	transparent.gray = background.getEmpty();
	if ( colored ) {
	    transparent.red = background.getEmpty();
	    transparent.green =  background.getEmpty();
	    transparent.blue = background.getEmpty();
	}
	png_set_tRNS( png_ptr, info_ptr, NULL/*trans*/, 0/*trans size*/, &transparent );
    }

    // Write the file header information.
    png_write_info( png_ptr, info_ptr );
    
    // we transfer #tab# to an other format
    const int bytes_per_pixel = (transparency.useAlphaTransparency() ? 1 : 0) + (colored ? 3 : 1);
//    png_byte image[height][width*bytes_per_pixel];
    png_byte* image = new png_byte[height*width*bytes_per_pixel];
    for ( png_uint_32 y = 0; y < height; ++y ) {
	progress.setValue(y/2);
	for ( png_uint_32 x = 0; x < width; ++x ) {
	    const int posImage = bytes_per_pixel * (x + y*width);
	    const int posTab = (colored ? 3 : 1) * (x + y*width);
	    if ( transparency.useAlphaTransparency() ) {
		if ( colored ) {
		    image[posImage    ] = colorTab[posTab];
		    image[posImage + 1] = colorTab[posTab + 1];
		    image[posImage + 2] = colorTab[posTab + 2];
		    if ( background.isBlack() ) {
		        image[posImage + 3] = tab[posTab/3];
		    } else {
		        image[posImage + 3] = 255 - tab[posTab/3];
		    }
		} else {
		    image[posImage] = background.getFull();
		    image[posImage + 1] = 255 - tab[posTab];
		}
	    } else {
		if ( colored ) {
		    image[posImage] = colorTab[posTab];
		    image[posImage + 1] = colorTab[posTab + 1];
 		    image[posImage + 2] = colorTab[posTab + 2];
		} else {
		    image[posImage] = tab[posTab];
		}
	    }
	}
    }
    png_bytep row_pointer;
    for ( png_uint_32 k = 0; k < height; ++k ) {
	progress.setValue(height/2+k/2);
	row_pointer = (png_bytep)image + k*width*bytes_per_pixel;
	// we write the pixels
	png_write_row( png_ptr, row_pointer );
    }

    delete[] image;
    
    // finish writing the rest of the file
    png_write_end(png_ptr, info_ptr);
    
    // clean up after the write, and free any memory allocated
    png_destroy_write_struct( &png_ptr, &info_ptr );
    
    fclose(fp);
    return 1;
}

const std::string
ImageGray::getDescriptionFromPNG( const std::string& file ) {
    FILE* fp = fopen( file.c_str(), "rb" );
    if ( fp == NULL ) {
        throw 2;
    }

    // first check that it is a PNG file
    const int PNG_BYTES_TO_CHECK = 4;
    png_byte buf[PNG_BYTES_TO_CHECK];

    // Read in some of the signature bytes
    if ( fread( buf, 1, PNG_BYTES_TO_CHECK, fp ) != PNG_BYTES_TO_CHECK ) {
        fclose(fp);
        throw 1;
    }

    // Compare the first PNG_BYTES_TO_CHECK bytes of the signature.
    if ( png_sig_cmp( buf, (png_size_t)0, PNG_BYTES_TO_CHECK ) ) {
        fclose(fp);
        throw 1;
    }

    png_structp png_ptr;
    png_infop info_ptr;

    // Create and initialize the png_struct with the default error handler functions.
    png_ptr = png_create_read_struct( PNG_LIBPNG_VER_STRING, NULL, NULL, NULL );
    
    if ( png_ptr == NULL ) {
        fclose(fp);
	throw 2;
    }

    // Allocate/initialize the memory for image information.
    info_ptr = png_create_info_struct(png_ptr);
    if ( info_ptr == NULL ) {
        fclose(fp);
	png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
	throw 2;
    }

    /* Set error handling if you are using the setjmp/longjmp method (this is
     * the normal method of doing things with libpng). 
     */
    if (setjmp(png_jmpbuf(png_ptr))) {
      // Free all of the memory associated with the png_ptr and info_ptr
        png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
	fclose(fp);
	// If we get here, we had a problem reading the file
	throw 2;
     }

    png_init_io(png_ptr, fp);

    // If we have already read some of the signature
    png_set_sig_bytes( png_ptr, PNG_BYTES_TO_CHECK );

    png_read_png( png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, png_voidp_NULL );
    
    png_textp text;
    int num_text;
    png_get_text( png_ptr, info_ptr, &text, &num_text );
    std::string desc;
    for ( int t = 0; t < num_text; ++t ) {
	if ( std::string(text[t].key) == "Description" ) {
    	    desc = text[t].text;
	}
    }

    // clean up after the read, and free any memory allocated
    png_destroy_read_struct( &png_ptr, &info_ptr, png_infopp_NULL );

    // close the file
    fclose(fp);

    return desc;
}
#endif

#ifdef HAVE_LIBMNG
typedef struct user_struct {
    FILE *hFile;
} userdata;

typedef userdata* userdatap; 

// callbacks declaration
mng_bool mywritedata( mng_handle myhandle, mng_ptr pBuf,
		      mng_uint32 iSize, mng_uint32* iWritten ) {
    userdatap pMydata = (userdatap)mng_get_userdata( myhandle );
    // iWritten will indicate errors
    *iWritten = fwrite( pBuf, 1, iSize, pMydata->hFile );
    return MNG_TRUE;
} 
mng_ptr myalloc( mng_size_t iSize ) {
    return (mng_ptr)calloc( 1, iSize );
}
void myfree( mng_ptr pPtr, mng_size_t iSize ) {
    free (pPtr);
    return;
}
mng_bool myopenstream( mng_handle myhandle ) {
    return MNG_TRUE;  // already opened in main function
}
mng_bool myclosestream( mng_handle myhandle ) {
    return MNG_TRUE;  // gets closed in main function
}

int
ImageGray::addChunks( mng_handle myhandle ) const {
    mng_retcode ret;

    if ( transparency.useSimpleTransparency() || transparency.useAlphaTransparency() ) {
	// to allow a redraw before writting transparent pixel on already hit pixels!
	ret = mng_putchunk_fram( myhandle, false, 3/*generate background layer*/,
				 0, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL );
	if ( ret != MNG_NOERROR ) { return ret; }
    }

    ret = mng_putchunk_ihdr( myhandle, width, height, MNG_BITDEPTH_8,
			     transparency.useAlphaTransparency() ?
			     ( colored ? MNG_COLORTYPE_RGBA : MNG_COLORTYPE_GRAYA)
			     : (colored ? MNG_COLORTYPE_RGB : MNG_COLORTYPE_GRAY),
			     MNG_COMPRESSION_DEFLATE, MNG_FILTER_ADAPTIVE, MNG_INTERLACE_NONE );
    if ( ret != MNG_NOERROR ) { return ret; }

    if ( transparency.useSimpleTransparency() ) {
	mng_uint8arr aAlphas;
	mng_uint8arr aRawdata;
	ret = mng_putchunk_trns( myhandle, false/*empty*/, false/*global*/,
				 colored ? MNG_COLORTYPE_RGB : MNG_COLORTYPE_GRAY,
				 0/*iCount*/, aAlphas/*aAlphas*/,
				 background.getEmpty()/*gray*/,
				 background.getEmpty()/*red*/, background.getEmpty()/*green*/, background.getEmpty()/*blue*/,
				 0, aRawdata/*aRawdata*/ );
	if ( ret != MNG_NOERROR ) { return ret; }
    }

//    mng_putchunk_bkgd( myhandle, false, MNG_COLORTYPE_GRAY, 0, 0/*gray*/, 0, 0, 0 );

    const int bytes_per_pixel = ( colored ? 3 : 1) + ( transparency.useAlphaTransparency() ? 1 : 0 );
    // we add a filter byte to the begining of each row:
    const int extendedSize = (1+width*bytes_per_pixel)*height;
    png_byte* extended = new png_byte[extendedSize];
    for ( int y = 0; y < height; ++y ) {
	extended[ y*(1+width*bytes_per_pixel) ] = 0; // the filter
	if ( transparency.useAlphaTransparency() ) {
	    const int xstart = y*(1+width*bytes_per_pixel) + 1;
	    const int xdelta = colored ? 4 : 2;
	    for ( int x = xstart, xtab = 0;
		  x < xstart + width*bytes_per_pixel;
		  x+=xdelta, ++xtab ) {
		if ( colored ) {
		    extended[x  ] = colorTab[  (xtab+y*width)*3]; // red
		    extended[x+1] = colorTab[1+(xtab+y*width)*3]; // green
		    extended[x+2] = colorTab[2+(xtab+y*width)*3]; // blue
		    if ( background.isBlack() ) {
		        extended[x+3] = tab[xtab+y*width]; // alpha
		    } else {
		        extended[x+3] = 255 - tab[xtab+y*width]; // alpha
		    }
		} else {
		    extended[x] = background.getFull(); //gray
		    if ( background.isBlack() ) {
		        extended[x+1] = tab[xtab+y*width]; // alpha
		    } else {
		        extended[x+1] = 255 - tab[xtab+y*width]; // alpha
		    }
		}
	    }
	} else {
	    if ( colored ) {
	        memcpy( extended + y*(1+width*bytes_per_pixel) + 1, colorTab + y*width*bytes_per_pixel, width*bytes_per_pixel );
	    } else {
	        memcpy( extended + y*(1+width) + 1, tab + y*width, width );
	    }
	}
    }
    // we compress "extended" to "compressed":
    int compressedSize = extendedSize*101/100 + 12; // large enough for calculations
    png_byte* compressed = new png_byte[compressedSize];
    if ( Z_OK != compress2( (Bytef*)compressed, (uLongf*)&compressedSize,
			    (const Bytef*)extended, (long unsigned int)extendedSize,
			    Z_DEFAULT_COMPRESSION ) ) { // put 9 for best compression
	return 0;
    }
    
    delete[] extended;
    
    ret = mng_putchunk_idat( myhandle, compressedSize, (mng_ptr*)compressed );
    if ( ret != MNG_NOERROR ) { return ret; }

    delete[] compressed; 

    mng_putchunk_iend( myhandle );
    return MNG_NOERROR;
}

int
ImageGray::saveMNG( const std::vector<Image*>& images, FILE* fp,
		const int framesPerSecond, const std::string& description ) {
    Progress progress( _("Saving MNG file"), images.size()-1 );
    // get a data buffer
    userdatap pMydata = (userdatap)calloc( 1, sizeof(userdata) );
    if ( pMydata == NULL ) {
	return 0;
    }
    pMydata->hFile = fp;

    mng_handle myhandle = mng_initialize( (mng_ptr)pMydata, myalloc, myfree, MNG_NULL );
    if ( myhandle == MNG_NULL ) {
	return 0;
    }

    mng_retcode ret;

    // setup callbacks
    ret = mng_setcb_writedata( myhandle, mywritedata );
    if ( ret != MNG_NOERROR ) {	return 0; }
    ret = mng_setcb_openstream( myhandle, myopenstream );
    if ( ret != MNG_NOERROR ) { return 0; }
    ret = mng_setcb_closestream( myhandle, myclosestream );
    if ( ret != MNG_NOERROR ) { return 0; }

    ret = mng_create( myhandle );
    if ( ret != MNG_NOERROR ) { return 0; }

    assert( !images.empty() );

    /////////////// add chunks
    ret = mng_putchunk_mhdr( myhandle, images[0]->width, images[0]->height,
			     framesPerSecond, images.size()+1, images.size(), images.size(),
			     1 + 2 + // MNG-LC because of FRAM chunk
			     8 + // transparency
			     64 + // bit 7, 8 and 9 are meaning-full
			     128 ); // background transparency
    if ( ret != MNG_NOERROR ) { return 0; }

    // loop, last frame when finished, delay before restart, infinity
    ret = mng_putchunk_term( myhandle, 3, 0, 0, 0x7fffffff );
    if ( ret != MNG_NOERROR ) { return 0; }

// mng_putchunk_back( myhandle, 256*background.getEmpty(), 
//   		   256*background.getEmpty(), 256*background.getEmpty(), // true color background
// 		   0/*not mandatory*/, 0/*imageid omitted*/, 0 /*tile omitted*/ );
// }

    // text chunk
    char soft[software.size()];
    software.copy( soft, software.size() );
    mng_putchunk_text( myhandle, 8, "Software", software.size(), soft );
    char desc[description.size()];
    description.copy( desc, description.size() );
    mng_putchunk_ztxt( myhandle, 11, "Description",
		       MNG_COMPRESSION_DEFLATE, description.size(), desc );

    for ( int i = 0; i < images.size(); ++i ) {
	progress.setValue(i);
	ret = images[i]->addChunks( myhandle );
	if ( ret != MNG_NOERROR ) { return 0; }
    }

    mng_putchunk_mend( myhandle );

    // call mywritedata
    ret = mng_write( myhandle );
    if ( ret != MNG_NOERROR ) { return 0; }

    mng_cleanup( &myhandle );
    fclose( pMydata->hFile );
    free( pMydata );

    return 1;
}
#endif

/*
class ImageTest : public ImageGray {
public:
    ImageTest( int w, int h, bool c ) : ImageGray(w,h,c) {
        for ( int i = 0; i < sizeBytes; ++i) {
	    tab[i] = 0;
	}
    }
    void plot( int pos, unsigned char val ) {
        tab[pos] = val;
    }
};

int main() {
    std::cerr << "Test of ImageGray\n";
#ifdef HAVE_LIBPNG
    {
        ImageTest image(100, 80, false);
	image.plot(100,255);
	image.plot(102,255);
	const std::string desc = "<test>foo</test>";
	const std::string file = "test.png";
	// test savePNG
	FILE* pf = fopen( file.c_str(), "wb" );
	assert( pf != NULL );
	assert( 1 == image.savePNG( pf, desc ) );
	// test getDescriptionFromPNG
	try {
	    assert( desc == ImageGray::getDescriptionFromPNG(file) );
	} catch ( int e ) {
	    std::cerr << "thrown: " << e << std::endl;
	    assert( false );
	}
    }
    {
        // test color image
      ImageTest image(100, 80, true);
      image.plot(3*100, 255);
      image.plot(3*100 + 1, 255);
      image.plot(3*100 + 2, 255);
      image.plot(3*100 + 3*2, 255);
      const std::string desc = "<test>foo</test>";
      const std::string file = "testColor.png";
      // test savePNG
      FILE* pf = fopen( file.c_str(), "wb" );
      assert( pf != NULL );
      assert( 1 == image.savePNG( pf, desc ) );
    }
#endif
    std::cerr << "End Test of ImageGray\n";
}
*/
