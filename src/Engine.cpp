// glito/Engine.cpp  v1.1  2004.09.05
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
# include <ctime>
#endif

#include <cmath>
// cos...
 
#ifndef M_PI
# define M_PI		3.14159265358979323846	/* pi */
#endif

#include <FL/Fl.H>

#include "Engine.hpp"
#include "Image.hpp"

void
Julia::start( int nbFrames ) {
    if ( nbFrames == 1 ) {
	vSizeMax = 2000000; // around 4 MB
    } else {
	vSizeMax = 10000000/nbFrames; // around 20 MB
    }
    vx.clear();
    vy.clear();
    nbHitMin = 1;
}

void
Julia::handle( float&x , float& y, const int nbHit ) {
    if ( nbHit >= nbHitMin ) {
	if ( !vx.empty() ) {
	    pop(x,y);
	} else if ( nbHitMin < 254 ) {
	    ++nbHitMin;
	}
    } else {
	push(-x,-y);
    }
}

void
Julia::push( float x, float y ) {
    if ( vx.size() < vSizeMax ) {
	vx.push_back(x);
	vy.push_back(y);
    }
}

void
Julia::pop( float& x, float& y ) {
    x = vx.back();
    vx.pop_back();
    y = vy.back();
    vy.pop_back();
}

Zoom::Zoom( const MinMax& minmax, const int w, const int h,
	    const Function& f, int nbFrames ) : zoomFunction(f), framingCorrection(0.96) {
    if ( f.modified() ) {
	zoomFunctionModified = true;
	zoomFunction.calculateTemp();
    } else {
	zoomFunctionModified = false;
    }
    float inter;
    if ( minmax.w() * h > minmax.h() * w ) {
	inter = framingCorrection*w/(minmax.w()+0.00001); // to avoid division by 0
    } else {
	inter = framingCorrection*h/(minmax.h()+0.00001);
    }
    centerX = (int)(w/2 - minmax.centerX()*inter);
    centerY = (int)(h/2 + minmax.centerY()*inter);
    fx = inter;
    fy = -inter;
    julia.start(nbFrames);
}

void
Zoom::toScreen( float x, float y ) const {
    if ( zoomFunctionModified ) {
	zoomFunction.previousPoint( x, y, false );
    }
    screenX = (int)(centerX + x*fx);
    screenY = (int)(centerY + y*fy);
}

Engine::Engine( int cornerX, int cornerY, int w, int h, const char *label )
    : Fl_Double_Window(cornerX,cornerY,w,h,label),
      state(PREVIEW), framesPerCycle(50),
      pointsForFraming(100000), animationFraming(0.1),
      pointsPerFrame(20000), minimalBuiltPoints(1000),
      imageSavedWidth(800), imageSavedHeight(600),
      animationSavedWidth(160), animationSavedHeight(120),
      intervalFrame(40),
      clockNumber(true), skel2("triangle"),
      trueDensity(true), colored(false) {
    imageLarge = buildImage( w, h );
}

Engine::~Engine() {
    for ( std::vector< Image* >::const_iterator i = images.begin(); i != images.end(); ++i ) {
	delete *i;
    }
    delete imageLarge;
}

void
Engine::drawLargeView() {
    const bool saving = ( state >= SAVEPGM && state <= SAVEPNG );
    const int buildWidth = saving ? imageSavedWidth : w();
    const int buildHeight = saving ? imageSavedHeight : h();
    Zoom zoom( skel.findFrame( pointsForFraming, _x, _y, _color ),
	       buildWidth, buildHeight, skel.getZoomFunction() );
    int xcenter = 0;
    int ycenter = 0;
    if ( saving ) {
        // at least one point will be in the center when we save:
	zoom.toScreen( _x, _y );
	xcenter = w()/2 - zoom.screenX;
	ycenter = h()/2 - zoom.screenY;
	if ( xcenter + imageSavedWidth < w() ) {
	    xcenter = w() - imageSavedWidth;
	}
	if ( xcenter > 0 ) {
	    xcenter = 0;
	}
	if ( ycenter + imageSavedHeight < h() ) {
	    ycenter = h() - imageSavedHeight;
	}
	if ( ycenter > 0 ) {
	    ycenter = 0;
	}
    }
    resetImage( buildWidth, buildHeight, w(), h(), xcenter + ycenter*imageSavedWidth );
    unsigned long timer = clock();
    while ( state == LARGEVIEW || ( SAVEPGM <= state && state <= SAVEPNG ) ) {
 	if ( clockNumber ) {
 	    drawPoints( skel, zoom, *imageLarge, timer );
 	} else {
 	    drawPoints( skel, zoom, *imageLarge, pointsPerFrame );
 	}
    }
}

int
Engine::drawPoints( const Skeleton& skelet, const Zoom& zoom, Image& image, unsigned long& clock0 ) {
    skelet.setXY( _x, _y, _color );
    int counter = 0;
    do {
	iterBuildPoints( skelet, zoom, image, minimalBuiltPoints );
	counter += minimalBuiltPoints;
    } while ( clock() - clock0 < intervalFrame * timecv );
    clock0 = clock();
    make_current();
    image.mem_draw();
    // we limit the frequence of checking because
    // it slows the program under Windows
    if ( Fl::ready() ) {
	Fl::check();
    }
    return counter;
}

void
Engine::drawPoints( const Skeleton& skelet, const Zoom& zoom, Image& image, const int imax ) {
    skelet.setXY( _x, _y, _color );
    iterBuildPoints( skelet, zoom, image, imax );
    make_current();
    image.mem_draw();
    // we limit the frequence of checking because
    // it slows the program under Windows
    if ( Fl::ready() ) {
	Fl::check();
    }
}

void
Engine::iterBuildPoints( const Skeleton& skelet, const Zoom& zoom,
			Image& image, const int imax ) const {
    if ( Function::system == LINEAR ) {
	for ( int i = 1; i <= imax; ++i ) {
	    skelet.nextPoint( _x, _y, _color );
	    zoom.toScreen( _x, _y );
	    image.mem_plot( zoom.screenX, zoom.screenY );
	    image.mem_coul( zoom.screenX, zoom.screenY, _color );
	}
    } else if ( Function::system == FORMULA || Function::system == SINUSOIDAL ) {
	// since initial conditions are important, we have to give a new seed to the orbit
	_x = (float)rand()*2/RAND_MAX - 1;
	_y = (float)rand()*2/RAND_MAX - 1;
	skelet.setXY( _x, _y, _color ); // put the seed in the orbit
	for ( int i = 1; i <= imax; ++i ) {
	    skelet.nextPoint( _x, _y, _color );
	    zoom.toScreen( _x, _y );
	    image.mem_plot( zoom.screenX, zoom.screenY );
	    image.mem_coul( zoom.screenX, zoom.screenY, _color );
	    if ( i % 1000 == 0 ) {
		_x = (float)rand()*2/RAND_MAX - 1;
		_y = (float)rand()*2/RAND_MAX - 1;
		skelet.setXY( _x, _y, _color );
	    }
	}
    } else { // JULIA
	Julia& j = zoom.julia;
	for ( int i = 1; i <= imax; ++i ) {
	    skelet.nextPoint( _x, _y, _color );
	    zoom.toScreen( _x, _y );
	    j.handle( _x, _y, image.getHit( zoom.screenX, zoom.screenY ) );
	    image.mem_plot( zoom.screenX, zoom.screenY );
	    image.mem_coul( zoom.screenX, zoom.screenY, _color );
	}
    }
}

void
Engine::resetImage( int w, int h, int wd, int wh, int s ) {
    delete imageLarge;
    imageLarge = buildImage( w, h, wd, wh, s );
}

Image*
Engine::buildImage( int w, int h, int wd, int wh, int s ) const {
    if ( trueDensity ) {
	return new ImageDensity( w, h, colored, wd, wh, s );
    } else {
	return new ImagePseudoDensity( w, h, colored, wd, wh, s );
    }
}

void
Engine::zoom() {
    const int imagesWidth  = ( state == SAVEMNG ) ? animationSavedWidth : w();
    const int imagesHeight = ( state == SAVEMNG ) ? animationSavedHeight : h();
    Skeleton skelSubframe;
    // we must check that we are not going to zoom inside the zoom function:
    if ( skel.selected() == 0 ) {
        // this section should be synchronized since the selected function
        // can be changed before its use by Skeleton::subframe
	skel.shiftSelectedFunction(1);
    }
    skelSubframe.subframe(skel);
    Function functionWork;
    const Zoom zoom( skel.findFrame( pointsForFraming, _x, _y, _color ),
		     imagesWidth, imagesHeight, skel.getZoomFunction(), framesPerCycle );
    for ( std::vector< Image* >::const_iterator i = images.begin(); i != images.end(); ++i ) {
	delete *i;
    }
    images.clear();
    for ( int i = 0; i < framesPerCycle; ++i ) {
	images.push_back( buildImage( imagesWidth, imagesHeight, w(), h() ) );
    }
    int idemo = 1;
    const int idemoMax = 3;
    unsigned long clock0 = clock();
    const float dilat0 = 1.0/skelSubframe.getFunction().surface();
    const float dilat1 = 1.0/skel.getFunction().surface();
    const float otherDilat = 1.0/(skel.sumSurfaces() - skel.getFunction().surface());
    while ( idemo <= idemoMax ) {
	for ( int k = 0; k < framesPerCycle; ++k ) {
	    const float rate = (float)k / framesPerCycle;
	    functionWork.spiralMix( skel.getFunction(), skelSubframe.getFunction(), rate );
	    functionWork.calculateTemp();
	    int pointsToCalculate = pointsPerFrame;
	    if ( state == SAVEMNG) {	
		// points(t) = points(0)*(1 + S/s(t) * (1/s(t) - 1/s(0))/(1/s(1) - 1/s(0)) ) 
		pointsToCalculate = (int)( ( 1.0 + (1/functionWork.surface()-dilat0)
					     / (dilat1-dilat0) * dilat1 / otherDilat
					       ) * pointsPerFrame );
		// to avoid explosion of computation time:
		if ( pointsToCalculate > 100 * pointsPerFrame ) {
		    pointsToCalculate = 100 * pointsPerFrame;
		}
	    }
	    for ( int i = 1; clockNumber || i < pointsToCalculate; ++i ) {
		skel.nextPoint( _x, _y, _color );
		float zx = _x;
		float zy = _y;
		functionWork.previousPoint( zx, zy, false );
		zoom.toScreen( zx, zy );
		images[k]->mem_plot( zoom.screenX, zoom.screenY );
		images[k]->mem_coul( zoom.screenX, zoom.screenY, _color );
		if ( clockNumber ) {
		    if ( i % minimalBuiltPoints == 0 ) {
			const unsigned long newClock = clock();
			if ( newClock - clock0 >= intervalFrame * timecv ) {
			    clock0 = newClock;
			    break;
			}
		    }
		}
	    }
	    make_current();
	    images[k]->mem_draw();
	    if ( Fl::ready() ) {
		Fl::check();
	    }
	    if ( state != ANIMATION && state != DEMO && state != SAVEMNG ) {
	        return;
	    }
	}
	if ( state == DEMO ) {
	    ++idemo;
	}
	if ( state == SAVEMNG ) {
	    return;
	}
    }
}

void
Engine::rotation() {
    const int imagesWidth  = ( state == SAVEMNG ) ? animationSavedWidth : w();
    const int imagesHeight = ( state == SAVEMNG ) ? animationSavedHeight : h();
    Skeleton skelWork = skel;
    std::vector<Zoom> zooms;
    for ( std::vector< Image* >::const_iterator i = images.begin(); i != images.end(); ++i ) {
	delete *i;
    }
    images.clear();
    const MinMax minmax = findFrameRotation( (int)(animationFraming*pointsForFraming) );
    for ( float k=-framesPerCycle; k <= framesPerCycle-1; ++k ) {
	skelWork.rotate( 2*M_PI/(2*framesPerCycle) );
	zooms.push_back( Zoom( minmax, imagesWidth, imagesHeight,
			       skelWork.getZoomFunction(), framesPerCycle ) );
	images.push_back( buildImage( imagesWidth, imagesHeight, w(), h() ) );
    }
    int idemo = 1;
    const int idemoMax = 2;
    unsigned long timer = clock();
    while ( idemo <= idemoMax ) {
	// to avoid a shift due to bad accuracy of rotate():
	skelWork = skel;
	for ( int k = -framesPerCycle; k <= framesPerCycle-1; ++k ) {
	    if ( clockNumber ) {
		drawPoints( skelWork, zooms[k+framesPerCycle],
			    *images[k+framesPerCycle], timer );
	    } else {
		drawPoints( skelWork, zooms[k+framesPerCycle],
			    *images[k+framesPerCycle], pointsPerFrame );
	    }
	    skelWork.rotate( 2*M_PI/(2*framesPerCycle) );
	    if ( state != ANIMATION && state != DEMO && state != SAVEMNG ) {
	        return;
	    }
	}
	if ( state == DEMO ) {
	    ++idemo;
	}
	if ( state == SAVEMNG ) {
	    return; // break
	}
    }
}

void
Engine::transition() {
    const int imagesWidth  = ( state == SAVEMNG ) ? animationSavedWidth : w();
    const int imagesHeight = ( state == SAVEMNG ) ? animationSavedHeight : h();
    Skeleton skelWork( skel1.size() );
    std::vector<Zoom> zooms;
    for ( std::vector< Image* >::const_iterator i = images.begin(); i != images.end(); ++i ) {
	delete *i;
    }
    images.clear();
    const MinMax minmax = findFrameTransition( (int)(animationFraming*pointsForFraming) );
    for ( float k=-framesPerCycle; k <= 0; ++k ) {
	const float rate = ( 1 - cos( M_PI*k/framesPerCycle ) ) / 2;
	skelWork.weightedMix( skel1, skel2, rate );
	zooms.push_back( Zoom( minmax, imagesWidth, imagesHeight,
			       skelWork.getZoomFunction(), framesPerCycle ) );
	images.push_back( buildImage( imagesWidth, imagesHeight, w(), h() ) );
    }
    int idemo = 1;
    const int idemoMax = 3;
    unsigned long timer = clock();
    while ( idemo <= idemoMax ) {
	for ( int k = -framesPerCycle; k <= framesPerCycle-1; ++k ) {
	    float rate = ( 1 - cos( M_PI*k/framesPerCycle ) ) / 2;
	    skelWork.weightedMix( skel1, skel2, rate );
	    const int i = (k > 0)? framesPerCycle - k: k+framesPerCycle;
	    if ( clockNumber ) {
		drawPoints( skelWork, zooms[i], *images[i], timer );
	    } else {
		drawPoints( skelWork, zooms[i], *images[i], pointsPerFrame );
	    }
	    if ( state != ANIMATION && state != DEMO && state != SAVEMNG ) {
	        return;
	    }
	    if ( state == DEMO && idemo == idemoMax && k == 0 ) {
		return; // break
	    }
	    if ( state == SAVEMNG && k == 0 ) {
		return; // break
	    }
	}
	if ( state == DEMO ) {
	    ++idemo;
	}
    }
}

const MinMax
Engine::findFrameRotation( const int nbit ) const {
    MinMax minmax;
    Skeleton skelWork = skel;
    for ( int k = -framesPerCycle; k < framesPerCycle; ++k ) {
	skelWork.rotate( 2*M_PI/(2*framesPerCycle) );
	minmax.candidate( skelWork.findFrame( nbit, _x, _y, _color ) );
    }
    minmax.build();
    return minmax;
}

const MinMax
Engine::findFrameTransition( const int nbit ) const {
    MinMax minmax;
    Skeleton skelWork( skel1.size() );
    for ( int k = -framesPerCycle; k <= 0; ++k ) {
	const float rate = ( 1 - cos( M_PI*k/framesPerCycle ) ) / 2;
	skelWork.weightedMix( skel1, skel2, rate );
	minmax.candidate( skelWork.findFrame( nbit, _x, _y, _color ) );
    }
    minmax.build();
    return minmax;
}

/*
// for test purpose
class TestEngine : public Engine {
public:
    TestEngine( int w, int h ) : Engine( 0, 0, w, h ) {
    }
protected:
    void draw() {
        state = DEMO;
	while ( true ) {
 	    skel.randomForDemo();
 	    rotation();
 	    if ( Function::system == LINEAR ) {
 		zoom();
 	    }
 	    skel1 = skel;
 	    skel2 = skel;
 	    skel2.modifyForDemo();
 	    transition();
 	    skel = skel2;
 	    Function::system = (systemType)((Function::system+1)%3);
 	}
    }
};
void quit_cb( Fl_Widget*, void* ) {
    exit(0);
}
int main( int argc, char **argv ) {
    int width = 400;
    int height = 300;
    Fl_Window window( width, height );
    // if the window is closed, the application exits:
    window.callback(quit_cb);
    TestEngine engine( width, height );
    Fl::visual( FL_DOUBLE | FL_INDEX );
    window.end();
    window.show(argc,argv);
    engine.show();
    return Fl::run();
}
*/
