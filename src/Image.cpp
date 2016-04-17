// glito/Image.cpp  v1.1  2004.09.05
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
#include <algorithm>
// lower_bound
#include <limits>
#include <cassert>

#include <FL/fl_draw.H>

#include "Image.hpp"
#include "IndentedString.hpp"

Image::Image() : start(0), wDraw(0), hDraw(0), crop(false), rgbTab(NULL) {
}

Image::Image( int w, int h, bool color, int wd, int hd, int s )
    : ImageGray(w, h, color), start(s), wDraw(wd), hDraw(hd) {
    if ( colored ) {
        rgbTab = (float*)calloc( 3*sizePixels, sizeof(float) );
	if ( rgbTab == NULL ) {
	    std::cerr << "Calloc failed in Image::Image(int,int,bool)!\n";
	    abort();
	}
	if ( colorMap.empty() ) {
 	    readDefinedMap(-1);
	}
    } else {
        rgbTab = NULL;
    }
    // tab is full of 0 and we want it full of 255:
    mem_clear();
    if ( wDraw == -1 && hDraw == -1 ) {
	 wDraw = width;
	 hDraw = height;
    }
    crop = wDraw < width && hDraw < height;
    if ( crop && start == -1 ) { // animation
	start = (wDraw - width)/2 + (hDraw - height)/2 * width;
    }
}

Image::~Image() {
    if ( colored && rgbTab != NULL ) {
	free(rgbTab);
    }
}

Image&
Image::operator=( const Image& other ) {
    if ( allocated ) {
	free(tab);
	allocated = false;
    }
    if ( colored && rgbTab != NULL ) {
        free(rgbTab);
    }
    copy(other);
    return *this;
}

void
Image::copy( const Image& other ) {
    ImageGray::copy( other );
    wDraw = other.wDraw;
    hDraw = other.hDraw;
    start = other.start;
    crop = other.crop;
    if ( other.rgbTab != NULL ) {
        rgbTab = (float*)calloc( 3*sizePixels, sizeof(float) );
	if ( rgbTab == NULL ) {
	    std::cerr << "Calloc failed in Image::copy(const Image&)!\n";
	    abort();
	}
	memcpy( rgbTab, other.rgbTab, 3*sizePixels );
    }
}

int
Image::getHit( int i, int j ) const {
    if ( 0 <= i && i < width && 0 <= j && j < height ) {
	return abs( tab[(colored ? 3 : 1)*( i+j*width )] - background.getEmpty() );
    } else {
	return 0;
    }
}

void
Image::mem_clear() {
    const unsigned char emptyBg = background.getEmpty();
    for ( int i = 0; i < sizePixels; ++i ) {
        tab[i] = emptyBg;
    }
    if ( colored ) {
        for ( int i = 0; i < 3*sizePixels; ++i ) {
	    rgbTab[i] = 0;
	    colorTab[i] = emptyBg;
	}
    }
}

void
Image::mem_plot( int i, int j ) {
    const unsigned char full = background.getFull();
    if ( 0 <= i && i < width && 0 <= j && j < height ) {
	unsigned char* element = &tab[i+j*width];
	*element = full;
	if ( colored ) {
	    colorTab[3*(i+j*width)] = full;
	    colorTab[3*(i+j*width)+1] = full;
	    colorTab[3*(i+j*width)+2] = full;
	}
    }
}

void
Image::mem_coul( int i, int j, float c ) {
    if ( colored && 0 <= i && i < width && 0 <= j && j < height ) {
        const int n = i+j*width;
	float rc = 0;
	float gc = 0;
	float bc = 0;
	int k = 0;
	while ( k + 1 < colorMap.size()
		&& !(colorMap[k].c <= c && c <= colorMap[k+1].c) ) {
	    ++k;
	}
	if ( k + 1 < colorMap.size() ) {
            ElementColorMap& col1 = colorMap[k]; 
            ElementColorMap& col2 = colorMap[k+1]; 
	    const float p = (c - col1.c) / (col2.c - col1.c);
	    rc = (1-p)*col1.r + p*col2.r;
	    gc = (1-p)*col1.g + p*col2.g;
	    bc = (1-p)*col1.b + p*col2.b;
	}
	assert( k >= 0 && k <= colorMap.size() - 1 );
	plotColor( n, rc, gc, bc );
    }
}

void
Image::plotColor( int n, float r, float g, float b ) {
    colorTab[3*n] = (unsigned char)r*255;
    colorTab[3*n+1] = (unsigned char)g*255;
    colorTab[3*n+2] = (unsigned char)b*255;
}

bool
Image::isEmpty() const {
    for ( int i = 0; i < sizePixels; ++i ) {
	if ( tab[i] != background.getEmpty() ) {
	    return false;
	}
    }
    return true;
}

void
Image::mem_draw() const {
    if ( crop ) {
        if ( colored ) {
	    fl_draw_image( colorTab-3*start, 0, 0, wDraw, hDraw, 3, 3*width );
	} else {
	    fl_draw_image_mono( tab-start, 0, 0, wDraw, hDraw, 1, width );
	}
    } else {
        if ( colored ) {
	    fl_draw_image( colorTab, (wDraw-width)/2, (hDraw-height)/2, width, height );
	} else {
	    fl_draw_image_mono( tab, (wDraw-width)/2, (hDraw-height)/2, width, height );
	}
    }
}

void
Image::shiftStart( const int shiftX, const int shiftY ) {
    const int startX = start%width;
    const int startY = start/width;
    int newStartX = startX + shiftX;
    int newStartY = startY + shiftY;
    if ( newStartX > 0 ) {
        newStartX = 0;
    }
    if ( newStartX < wDraw-width ) {
        newStartX = wDraw-width;
    }
    if ( newStartY > 0 ) {
        newStartY = 0;
    }
    if ( newStartY < hDraw-height ) {
        newStartY = hDraw-height;
    }
    start = newStartX + newStartY*width;
}

std::vector<ElementColorMap>
Image::colorMap;

void
Image::readColorMap( const std::string& colorText ) {
    colorMap.clear();
    const vector< string > colors = IS::extractAll( colorText, "[", "]" );
    // [ccc color rgbt <rrr, ggg, bbb, 0.0>]
    for ( vector< string >::const_iterator i = colors.begin(); i != colors.end(); ++i ) {
        const string c = IS::extractFirst( *i, "", " " );
        const string r = IS::extractFirst( *i, "<", "," );
	const vector< string > gb = IS::extractAll( *i, " ", "," );
	// gb = { " color rgbt <rrr"; " ggg"; " bbb" }
	if ( gb.size() >= 3 ) {
	    const string g = gb[1];
	    const string b = gb[2];
	    colorMap.push_back( ElementColorMap( atof(c.c_str()), atof(r.c_str()),
						 atof(g.c_str()), atof(b.c_str()) ) );
	}
    }
}

void
Image::readDefinedMap( const int map ) {
    colorMap.clear();
    if ( map == 0 ) { // autumn
	colorMap.push_back( ElementColorMap(0.00, 0.000, 0.595, 1.000) );
	colorMap.push_back( ElementColorMap(0.38, 0.567, 1.000, 0.000) );
	colorMap.push_back( ElementColorMap(0.6,  1.000, 1.000, 0.000) );
	colorMap.push_back( ElementColorMap(1.00, 0.915, 0.000, 0.058) );
    } else if ( map == 1 ) { // cmy
        colorMap.push_back( ElementColorMap(0.0,      0.0, 1.0, 1.0) );
	colorMap.push_back( ElementColorMap(0.333333, 1.0, 0.0, 1.0) );
	colorMap.push_back( ElementColorMap(0.666667, 1.0, 1.0, 0.0) );
	colorMap.push_back( ElementColorMap(1.0,      0.0, 1.0, 1.0) );
    } else if ( map == 2 ) { // rgb
	colorMap.push_back( ElementColorMap(0.0,      1.0, 0.0, 0.0) );
	colorMap.push_back( ElementColorMap(0.333333, 0.0, 1.0, 0.0) );
	colorMap.push_back( ElementColorMap(0.666667, 0.0, 0.0, 1.0) );
	colorMap.push_back( ElementColorMap(1.0,      1.0, 0.0, 0.0) );
    } else if ( map == 3 ) { // light cmy
	colorMap.push_back( ElementColorMap(0.0,      0.5, 1.0, 1.0) );
	colorMap.push_back( ElementColorMap(0.333333, 1.0, 0.5, 1.0) );
	colorMap.push_back( ElementColorMap(0.666667, 1.0, 1.0, 0.5) );
	colorMap.push_back( ElementColorMap(1.0,      0.5, 1.0, 1.0) );
    } else if ( map == 4 ) { // dark
	colorMap.push_back( ElementColorMap(0.0,      0.00, 0.32, 0.00) );
	colorMap.push_back( ElementColorMap(0.333333, 0.58, 0.58, 0.00) );
	colorMap.push_back( ElementColorMap(0.666667, 0.43, 0.00, 0.00) );
	colorMap.push_back( ElementColorMap(1.0,      0.00, 0.00, 0.47) );
    } else if ( map == 5 ) { // rich
	colorMap.push_back( ElementColorMap(0.000000, 0.459018, 0.815603, 0.242469) );
	colorMap.push_back( ElementColorMap(0.035714, 0.570534, 0.529856, 0.253161) );
	colorMap.push_back( ElementColorMap(0.071429, 0.682050, 0.244109, 0.263853) );
	colorMap.push_back( ElementColorMap(0.107143, 0.736788, 0.396304, 0.209489) );
	colorMap.push_back( ElementColorMap(0.142857, 0.791525, 0.548498, 0.155124) );
	colorMap.push_back( ElementColorMap(0.178571, 0.754719, 0.689115, 0.577562) );
	colorMap.push_back( ElementColorMap(0.214286, 0.717913, 0.829731, 1.000000) );
	colorMap.push_back( ElementColorMap(0.250000, 0.823495, 0.871860, 0.561662) );
	colorMap.push_back( ElementColorMap(0.285714, 0.929078, 0.913989, 0.123323) );
	colorMap.push_back( ElementColorMap(0.321429, 0.577779, 0.889619, 0.171096) );
	colorMap.push_back( ElementColorMap(0.357143, 0.226480, 0.865248, 0.218870) );
	colorMap.push_back( ElementColorMap(0.392857, 0.329124, 0.679513, 0.485321) );
	colorMap.push_back( ElementColorMap(0.428571, 0.431768, 0.493777, 0.751773) );
	colorMap.push_back( ElementColorMap(0.464286, 0.680423, 0.706713, 0.739927) );
	colorMap.push_back( ElementColorMap(0.500000, 0.929078, 0.919648, 0.728080) );
	colorMap.push_back( ElementColorMap(0.535714, 0.824186, 0.571752, 0.507281) );
	colorMap.push_back( ElementColorMap(0.571429, 0.719293, 0.223855, 0.286481) );
	colorMap.push_back( ElementColorMap(0.607143, 0.684035, 0.537460, 0.440468) );
	colorMap.push_back( ElementColorMap(0.642857, 0.648777, 0.851064, 0.594454) );
	colorMap.push_back( ElementColorMap(0.678571, 0.631586, 0.907801, 0.499426) );
	colorMap.push_back( ElementColorMap(0.714286, 0.614394, 0.964539, 0.404399) );
	colorMap.push_back( ElementColorMap(0.750000, 0.469942, 0.572466, 0.535533) );
	colorMap.push_back( ElementColorMap(0.785714, 0.325490, 0.180392, 0.666667) );
	colorMap.push_back( ElementColorMap(0.821429, 0.645015, 0.382967, 0.333333) );
	colorMap.push_back( ElementColorMap(0.857143, 0.964539, 0.585541, 0.000000) );
	colorMap.push_back( ElementColorMap(0.892857, 0.953901, 0.740186, 0.117787) );
	colorMap.push_back( ElementColorMap(0.928571, 0.943262, 0.894831, 0.235574) );
	colorMap.push_back( ElementColorMap(0.964286, 0.701140, 0.855217, 0.239021) );
	colorMap.push_back( ElementColorMap(1.000000, 0.459018, 0.815603, 0.242469) );
    } else { //fast
        colorMap.push_back( ElementColorMap(0.0, 1.0, 1.0, 0.0) );
        colorMap.push_back( ElementColorMap(1.0, 0.0, 0.5, 1.0) );
    }
}

///////////////////////////////////////////////////////////////////

PseudoDensity::PseudoDensity() {
    setLogProbaHitMax( log((float)10000) );
}

void
PseudoDensity::setLogProbaHitMax( float logProba ) {
    logProbaHitMax = logProba;
    for ( int i = 0; i < 256; ++i ) {
	plotProbability[i] = exp( logProbaHitMax * i/255 );
    }
}

void
PseudoDensity::setProba( float proba ) {
    setLogProbaHitMax( proba * log((float)RAND_MAX) );
}

float
PseudoDensity::getProba() const {
    return logProbaHitMax / log((float)RAND_MAX);
}

PseudoDensity
ImagePseudoDensity::pseudoDensity = PseudoDensity();

ImagePseudoDensity::ImagePseudoDensity( int w, int h, bool c, int wd, int hd, int s )
    : Image( w, h, c, wd, hd, s ) {
    resetLimitGray();
}

ImagePseudoDensity&
ImagePseudoDensity::operator=( const ImagePseudoDensity& other ) {
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
ImagePseudoDensity::copy( const ImagePseudoDensity& other ) {
    Image::copy( other );
    limitGray = other.limitGray;
}

void
ImagePseudoDensity::mem_plot( int i, int j ) {
    if ( 0 <= i && i < width && 0 <= j && j < height ) {
	unsigned char* element = &tab[i+j*width];
	if ( background.isBlack() ) {
	    if ( *element == 0 ) {
		*element = limitGray;
	    } else if ( pseudoDensity.plot( *element - limitGray ) ) {
		if ( *element < 255-limitGray ) {
		    ++(*element);
		} else if ( limitGray > 1 ) {
		    --limitGray;
		    for ( int n = 0; n < sizePixels; ++n ) {
			if ( tab[n] > 1 ) --tab[n];
		    }
		}
	    }
	} else {
	    if ( *element == 255 ) {
		*element = limitGray;
	    } else if ( pseudoDensity.plot( limitGray - *element ) ) {
		if ( *element > 255-limitGray ) {
		    --(*element);
		} else if ( limitGray < 254 ) {
		    ++limitGray;
		    for ( int n = 0; n < sizePixels; ++n ) {
			if ( tab[n] < 254 ) ++tab[n];
		    }
		}
	    }
	}
    }
}

void
ImagePseudoDensity::plotColor( int n, float r, float g, float b ) {
    const float hit = 1+abs( tab[n] - limitGray );
    //    std::cerr<<"### "<< rgbTab[3*n] << " "<<rgbTab[3*n+1] << " "<<rgbTab[3*n+2] << "\n";
    //    std::cerr<< hit << " "<<r << " "<<g<<" "<<b << "\n";
    //    assert(0.999<=hit && hit<=255.0001);
    rgbTab[3*n] = (hit-1)*rgbTab[3*n] + r;
    rgbTab[3*n] /= hit;
    rgbTab[3*n+1] = (hit-1)*rgbTab[3*n+1] + g;
    rgbTab[3*n+1] /= hit;
    rgbTab[3*n+2] = (hit-1)*rgbTab[3*n+2] + b;
    rgbTab[3*n+2] /= hit;
    //    std::cerr<<rgbTab[3*n] << " "<<rgbTab[3*n+1] << " "<<rgbTab[3*n+2] << "\n";
}

void
ImagePseudoDensity::mem_clear() {
    Image::mem_clear();
    resetLimitGray();
}

void
ImagePseudoDensity::mem_draw() const {
    if ( colored ) {
        const unsigned char emptyBg = background.getEmpty();
        if ( background.isBlack() ) {
	    for ( int i = 0; i < sizePixels; ++i ) {
	        if ( tab[i] != emptyBg ) {  
		    colorTab[3*i  ] = (unsigned char)( rgbTab[3*i  ] * tab[i] );
		    colorTab[3*i+1] = (unsigned char)( rgbTab[3*i+1] * tab[i] );
		    colorTab[3*i+2] = (unsigned char)( rgbTab[3*i+2] * tab[i] );
		    //		    if ( i%100 == 0 )
		    //		      std::cerr<<(int)colorTab[3*i] << " "<<(int)colorTab[3*i+1] << " "<<(int)colorTab[3*i+2] << "\n";
		}
	    }
	} else {
	    for ( int i = 0; i < sizePixels; ++i ) { 
	        if ( tab[i] != emptyBg ) {  
		    colorTab[3*i  ] = (unsigned char)( 255-(1.0-rgbTab[3*i  ])*(255-tab[i]) );
		    colorTab[3*i+1] = (unsigned char)( 255-(1.0-rgbTab[3*i+1])*(255-tab[i]) );
		    colorTab[3*i+2] = (unsigned char)( 255-(1.0-rgbTab[3*i+2])*(255-tab[i]) );
		}
	    }
	}
    }
    Image::mem_draw();
}

///////////////////////////////////////////////////////////////////

ImageDensity::ImageDensity( int w, int h, bool c, int wd, int hd, int s )
  : Image( w, h, c, wd, hd, s ), maxHit(0) {
    hitTab = (int*)calloc( sizePixels, sizeof(int) );
    if ( hitTab == NULL ) {
        std::cerr << "calloc of "<< sizePixels*4
		<<" B failed in ImageDensity::ImageDensity(int,int,int,int,int,bool)!\n";
	abort();
    } else {
	allocated = true;
    }
}

ImageDensity::~ImageDensity() {
    if ( allocated ) {
	free(hitTab);
    }
}

void
ImageDensity::copy( const ImageDensity& other ) {
    Image::copy( other );
    if ( other.allocated ) {
	hitTab = (int*)calloc( sizePixels, sizeof(int) );
	if ( hitTab == NULL ) {
	    std::cerr << "Calloc failed in ImageDensity::copy(const ImageDensity&)!\n";
	    abort();
	}
	memcpy( hitTab, other.hitTab, sizePixels );
	maxHit = other.maxHit;
	allocated = true;
    } else {
	std::cerr << "Image not built in ImageDensity::copy(const ImageDensity&)!\n";
	abort();
	allocated = false;
    }
}

ImageDensity&
ImageDensity::operator=( const ImageDensity& other ) {
    if ( allocated ) {
	free(tab);
	if ( colored ) {
	  free(colorTab);
	}
	free(hitTab);
	allocated = false;
    }
    copy(other);
    return *this;
}

const int
ImageDensity::maxMaxHit = std::numeric_limits<int>().max() - 1;

void
ImageDensity::plotColor( int n, float r, float g, float b ) {
    const float hit = hitTab[n];
    rgbTab[3*n] = (hit-1)*rgbTab[3*n] + r;
    rgbTab[3*n] /= hit;
    rgbTab[3*n+1] = (hit-1)*rgbTab[3*n+1] + g;
    rgbTab[3*n+1] /= hit;
    rgbTab[3*n+2] = (hit-1)*rgbTab[3*n+2] + b;
    rgbTab[3*n+2] /= hit;
}

void
ImageDensity::mem_plot( int i, int j ) {
    if ( 0 <= i && i < width && 0 <= j && j < height ) {
 	const int hits = ++hitTab[i+j*width];
  	if ( hits > maxHit ) {
	    maxHit = hits;
	    if ( maxHit > maxMaxHit ) { // to avoid int > MAX_INT
	        --hitTab[i+j*width];
		--maxHit;
	    }
  	}
    }
}

int
ImageDensity::getHit( int i, int j ) const {
    if ( 0 <= i && i < width && 0 <= j && j < height ) {
	return hitTab[i+j*width];
    } else {
	return 0;
    }
}

void
ImageDensity::mem_clear() {
    Image::mem_clear();
    for ( int i = 0; i < sizePixels; ++i ) {
	hitTab[i] = 0;
    }
    maxHit = 0;
}

bool
ImageDensity::isEmpty() const {
    for ( int i = 0; i < sizePixels; ++i ) {
	if ( hitTab[i] != 0 ) {
	    return false;
	}
    }
    return true;
}

void
ImageDensity::mem_draw() const {
// const float coef = (float)255 / log1p(maxHit);;
// tab[i] = (unsigned char)( log1p(hitTab[i]) * coef );
//  std::cerr<<maxHit<<"\n";
    std::vector<int> nbHit(256);
    for ( int c = 0; c <= 255; ++c ) {
	nbHit[c] = (int)pow( 1 + maxHit, (float)c / 255 ); /* from 1 to 1+maxHit */
    }
    if ( background.isBlack() ) {
	for ( int i = 0; i < sizePixels; ++i ) {
	    if ( hitTab[i] ) {
		if ( hitTab[i] > nbHit[tab[i]] ) {
		    const int gray = std::lower_bound( nbHit.begin(), nbHit.end(), hitTab[i] ) - nbHit.begin();
		    tab[i] = (unsigned char)gray;
		    if ( colored ) {
			colorTab[3*i  ] = (unsigned char)( rgbTab[3*i  ] * gray );
			colorTab[3*i+1] = (unsigned char)( rgbTab[3*i+1] * gray );
			colorTab[3*i+2] = (unsigned char)( rgbTab[3*i+2] * gray );
		    }
		}
	    }
	}
    } else {
	for ( int i = 0; i < sizePixels; ++i ) {
	    if ( hitTab[i] ) {
		if ( colored || hitTab[i] > nbHit[255-tab[i]] ) {
		    const int gray = nbHit.end() - 1 - std::lower_bound( nbHit.begin(), nbHit.end(), hitTab[i] );
		    tab[i] = (unsigned char)gray;
		    if ( colored ) {
		        colorTab[3*i  ] = (unsigned char)( 255-(1.0-rgbTab[3*i  ])*(255-gray) );
			colorTab[3*i+1] = (unsigned char)( 255-(1.0-rgbTab[3*i+1])*(255-gray) );
			colorTab[3*i+2] = (unsigned char)( 255-(1.0-rgbTab[3*i+2])*(255-gray) );
		    }
		}
	    }
	}
    }
    Image::mem_draw();
}

/*
int main() {
    const std::string desc = "<test>foo</test>";
    std::cerr << "Test of Image\n";
    {
        Image image( 100, 80, false );
	// test isEmpty after construction
	assert( image.isEmpty() );
	image.mem_plot( 0, 1 );
	image.mem_clear();
	// test isEmpty after clear
	assert( image.isEmpty() );
	image.mem_plot( 0, 1 );
	// test mem_plot
	assert( !image.isEmpty() );
	image.mem_plot( 2, 1 );
	image.mem_plot( 4, 1 );
	// test getHit
	assert( image.getHit(2, 1) == 255 );
	assert( image.getHit(2, 2) == 0 );
	const std::string file = "test.png";
	FILE* pf = fopen( file.c_str(), "wb" );
	assert( pf != NULL );
	assert( 1 == image.savePNG( pf, desc ) );
	{
	    Image i2 = image;
	    const std::string file = "test2.png";
	    FILE* pf = fopen( file.c_str(), "wb" );
	    assert( pf != NULL );
	    assert( 1 == i2.savePNG( pf, desc ) );
	}
    }
    {
        Image image( 100, 80, true );
	// test isEmpty after construction
	assert( image.isEmpty() );
	image.mem_plot( 0, 1 );
	image.mem_clear();
	// test isEmpty after clear
	assert( image.isEmpty() );
	image.mem_plot( 0, 1 );
	// test mem_plot
	assert( !image.isEmpty() );
	image.mem_plot( 2, 1 );
	image.mem_plot( 4, 1 );
	// test getHit
	assert( image.getHit(2, 1) == 255 );
	assert( image.getHit(2, 2) == 0 );
	const std::string file = "testC.png";
	FILE* pf = fopen( file.c_str(), "wb" );
	assert( pf != NULL );
	assert( 1 == image.savePNG( pf, desc ) );
	{
	    Image i2 = image;
	    assert( i2.isColored() );
	    const std::string file = "testC2.png";
	    FILE* pf = fopen( file.c_str(), "wb" );
	    assert( pf != NULL );
	    assert( 1 == i2.savePNG( pf, desc ) );
	}
    }
    {
        ImageDensity image( 100, 80, true );
	// test isEmpty after construction
	assert( image.isEmpty() );
	image.mem_plot( 0, 1 );
	image.mem_clear();
	// test isEmpty after clear
	assert( image.isEmpty() );
	image.mem_plot( 0, 1 );
	// test mem_plot
	assert( !image.isEmpty() );
	for ( int i = 0; i < 32; ++i ) {
	    image.mem_plot( 2, 1 );
	}
	image.mem_coul( 2, 1,0.5 );
	image.mem_plot( 4, 1 );
	// test getHit
	assert( image.getHit(2, 1) == 32 );
	assert( image.getHit(2, 2) == 0 );
	//Image::mem_draw must be commented in ImageDensity::mem_draw
	image.mem_draw();
	const std::string file = "testCD.png";
	FILE* pf = fopen( file.c_str(), "wb" );
	assert( pf != NULL );
	assert( 1 == image.savePNG( pf, desc ) );
	{
	    Image i2 = image;
	    assert( i2.isColored() );
	    const std::string file = "testCD2.png";
	    FILE* pf = fopen( file.c_str(), "wb" );
	    assert( pf != NULL );
	    assert( 1 == i2.savePNG( pf, desc ) );
	}
    }
    std::cerr << "End Test of Image\n";
}
*/
