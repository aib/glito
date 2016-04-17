// glito/Glito.cpp  v1.1  2004.09.05
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

#ifdef DEBUG
# include <iostream>
#endif
# include <iostream>
#include <fstream>
 
#ifndef M_PI
# define M_PI		3.14159265358979323846	/* pi */
#endif
#ifndef M_PI_2
# define M_PI_2		1.57079632679489661923	/* pi/2 */
#endif

#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/fl_draw.H>

#include "IndentedString.hpp"
#include "Glito.hpp"

#ifdef ENABLE_NLS
# include <libintl.h>
# define _(String) gettext (String)
#else
# define _(String) (String)
#endif

#ifdef HAVE_LIBPNG
Snapshot::Snapshot() : width(128), height(96), iterations(1000000), path() {
}

void
Snapshot::setPath() {
    const char *p = fl_dir_chooser( _("Choose a directory for fast saving"), NULL );
    if ( p != NULL ) {
        if ( fl_filename_isdir(p) ) {
	    path = p;
	    path += "/";
	} else {
	    fl_alert( _("You did not enter a valid directory:\n%s"), p );
	}
    }
}

bool
Snapshot::checkPath() {
    if ( path.empty() ) {
        setPath();
    }
    return !path.empty();
}

void
Snapshot::save( const Image& thumbnail, const string& comment ) const {
    time_t* timeSecond = new time_t;
    time(timeSecond);
    struct tm* time = localtime(timeSecond);
    const string fileName = IS::translate(1900+time->tm_year)
      + (1+time->tm_mon < 10 ? "0" : "") + IS::translate(1+time->tm_mon)
      + (time->tm_mday < 10 ? "0" : "") + IS::translate(time->tm_mday)
      + "_"
      + (time->tm_hour < 10 ? "0" : "") + IS::translate(time->tm_hour) + "h"
      + (time->tm_min < 10 ? "0" : "") + IS::translate(time->tm_min) + "m"
      + (time->tm_sec < 10 ? "0" : "") + IS::translate(time->tm_sec) + "s"
      + ".png";
    delete time;
    delete timeSecond;
    FILE *fp = fopen( (path + fileName).c_str() , "wb" );
    const bool success =  fp != NULL && thumbnail.savePNG( fp, comment );
    if (!success ) {
        fl_alert( _("Saving PNG file failed.") );
    }
}

string
Snapshot::toXML( int level = 0 ) const {
    IS::ToXML in(level);
    in.elementI( "width", width );
    in.elementI( "height", height );
    in.elementI( "iterations", iterations );
    IS::ToXML res(level);
    return res.elementIR( "snapshot", in.getValue() ).getValue();
}

void
Snapshot::fromXML( const string& paramXML ) {
    const string stringXML = IS::ToXML::extractFirst( paramXML, "snapshot" );
    if ( !stringXML.empty() ) {
        width = atoi(IS::ToXML::extractFirst( stringXML, "width" ).c_str());
	height = atoi(IS::ToXML::extractFirst( stringXML, "height" ).c_str());
	iterations = atoi(IS::ToXML::extractFirst( stringXML, "iterations" ).c_str());
    }
}

void
Glito::setSnapshotPath() {
    snapshot.setPath();
}

void
Glito::saveSnapshot() {
    if ( snapshot.checkPath() ) {
        const State oldState = state;
        state = SAVEPNG;
        Image* thumbnail = buildImage( snapshot.getWidth(), snapshot.getHeight(), 0, 0, 0 );
        Zoom zoom( skel.findFrame( pointsForFraming, _x, _y, _color ),
	           snapshot.getWidth(), snapshot.getHeight(), skel.getZoomFunction() );
        iterBuildPoints( skel, zoom, *thumbnail, snapshot.getIterations() );
        thumbnail->mem_draw();
        snapshot.save( *thumbnail, skel.toXML() );
        delete thumbnail;
	state = oldState;
    } else {
        // cancel was pressed during selection of the snapshot directory
        fl_alert( _("Saving PNG file failed.") );
    }
}
#endif // HAVE_LIBPNG

Glito::Glito( int cornerX, int cornerY, int w, int h, const char *label )
    : Engine(cornerX, cornerY, w, h, label),
      rotationShift(0.05),
      closeEdge(0.6), previewSize(0.4),
      intervalMotionDetection(40),
      mouseRotDil(false),
#ifdef HAVE_LIBPNG
      snapshot(),
#endif // HAVE_LIBPNG
      printDimension(false), printCoordinates(false), needRedraw(true) {
    smallImage = buildImage( (int)(previewSize*w), (int)(previewSize*h) );
    setSchemaScale( w, h );
    end();
}

void
Glito::setCloseEdge( float value ) {
    closeEdge = value;
    setSchemaScale( w(), h() );
    needRedraw = true;
}

void
Glito::setPreviewSize( float value ) {
    previewSize = value;
    setSchemaScale( w(), h() );
    resetSmallImage( w(), h() );
    needRedraw = true;
}

void
Glito::setSchemaScale( int w, int h ) {
    const int smallSize = ( w < h ) ? w : h;
    if ( smallSize == h ) {
	schemaScale = SchemaScale( w - h/2, h/2, closeEdge*h );
    } else { // smallSize == w
	schemaScale = SchemaScale( w/2, h-w/2, closeEdge*w );
    }
}

void
Glito::startSave( const Image::imageFormat format, const int anim ) {
    if ( format == Image::BMPB ) {
	state = SAVEBMPB;
	drawLargeView();
    } else if ( format == Image::BMPG ) {
	state = SAVEBMPG;
	drawLargeView();
    } else if ( format == Image::PGM ) {
	state = SAVEPGM;
	drawLargeView();
    } else if ( format == Image::PNG ) {
	state = SAVEPNG;
	drawLargeView();
    } else if ( format == Image::MNG ) {
	state = SAVEMNG;
        make_current();
	fl_color(FL_DARK3);
	fl_rectf( 0, 0, w(), h() );
	needRedraw = true;
	if ( anim == 0 ) {
	    zoom();
	    save( state, IS::ToXML().elementIR( "zoom", skel.toXML(0,true) ).getValue() );
	} else if ( anim == 1 ) {
	    transition();
	    for ( int im = images.size()-1; im >= 0; --im ) {
		images.push_back( new Image( *images[im] ) );
	    } 
	    IS::ToXML skeletons;
	    // split because IS::ToXML().add().add().getValue() gives seg fault !
	    skeletons.add( skel1.toXML() ).add( skel2.toXML() );
	    save( state, IS::ToXML().elementIR( "transition",
						skeletons.getValue() ).getValue() );
	} else {
	    rotation();
	    save( state, IS::ToXML().elementIR( "rotation", skel.toXML(0,true) ).getValue() );
	}
    }
#ifdef DEBUG
    else {
	cerr << "Invalid format in Glito::startSave(const string&)!" << endl;
    }
#endif
}

void
Glito::resize( int XX, int YY, int WW, int HH ) {
    if ( WW != w() || HH != h() ) {
 	resetImage( WW, HH );
	setSchemaScale( WW, HH );
	resetSmallImage( WW, HH );
	Fl_Double_Window::resize( XX, YY, WW, HH );
	needRedraw = true;
    }
}

void
Glito::setColored( bool c ) {
    colored = c;
    resetImage( w(), h());
    resetSmallImage( w(), h() );
}

void
Glito::demonstration() {
    int counter = 0;
    if ( Function::system == FORMULA ) {
	Function::system = LINEAR;
	setSystemType();
	skel.randomForDemo();
    }
    Function::system = (systemType)((Function::system)%3);
    while ( state == DEMO ) {
	rotation();
	if ( Function::system == LINEAR ) {
	    zoom();
	}
	skel1 = skel;
	skel2 = skel;
	skel2.modifyForDemo();
	transition();
	skel = skel2;
	++ counter;
	if ( counter%2 == 0) {
	    Function::system = (systemType)((Function::system+1)%3);
	    setSystemType();
	    skel.randomForDemo();
	}
    }
}

#ifdef DEBUG
string
Glito::getState() const {
    switch ( state ) {
    case ( PREVIEW ) :
	return "Preview";
    case ( LARGEVIEW ) :
	return "LargeView";
    case ( ANIMATION ) :
	return "Animation";
    case ( DEMO ) :
	return "Demo";
    case ( SAVEPGM ) :
	return "SavePGM";
    case ( SAVEBMPB ) :
	return "SaveBMP Bitmap";
    case ( SAVEBMPG ) :
	return "SaveBMP Gray";
    case ( SAVEPNG ) :
	return "SavePNG Gray";
    case ( SAVEMNG ) :
	return "SaveMNG Gray";
    default:
	cerr << "State unknown. Code: " << state <<".\n";
	return "";
    }
}
#endif

void
Glito::save( const State saveState, const string& description ) {
    const char *p;
//    assert ( !image->empty() );
    if ( saveState == SAVEPGM ) {
	p = fl_file_chooser( _("Pick a file"), "*.pgm", "*.pgm" );
    } else if ( saveState == SAVEPNG ) {
	p = fl_file_chooser( _("Pick a file"), "*.png", "*.png" );
    } else if ( saveState == SAVEMNG ) {
	p = fl_file_chooser( _("Pick a file"), "*.mng", "*.mng" );
    } else if ( saveState == SAVEBMPB || saveState == SAVEBMPG ) {
	p = fl_file_chooser( _("Pick a file"), "*.bmp", "*.bmp" );
    } else {
	return;
    }
    if ( p != NULL ) {
	if ( saveState == SAVEPGM ) {
	    std::ofstream f( p );
	    imageLarge->save( f, Image::PGM );
	} else if ( saveState == SAVEBMPB ) {
	    std::ofstream f( p );
	    imageLarge->save( f, Image::BMPB );
	} else if ( saveState == SAVEBMPG ) {
	    std::ofstream f( p );
	    imageLarge->save( f, Image::BMPG );
	}
#ifdef HAVE_LIBPNG
	else if ( saveState == SAVEPNG ) {
	    FILE *fp = fopen( p , "wb" );
	    if ( fp != NULL ) {
		if ( !imageLarge->savePNG( fp, description ) ) {
		    fl_alert( _("Saving PNG file failed.") );
		}
	    }
	}
#endif
#ifdef HAVE_LIBMNG
	else if ( saveState == SAVEMNG ) {
	    FILE *fp = fopen( p , "wb" );
	    if ( fp != NULL ) {
		if ( !ImageGray::saveMNG( images, fp, 1000/intervalFrame, description ) ) {
		    fl_alert( _("Saving MNG file failed.") );
		}
	    }
	}
#endif
    }
    state = PREVIEW;
}

void
Glito::drawSchema() const {
    fl_color( ImageGray::background.isBlack() ? FL_DARK2 : FL_LIGHT1 );
    // clear under smallImage:
    fl_rectf( 0, smallImage->h()+1, smallImage->w()+1, h() - (smallImage->h()-1) );
    // clear right part:
    fl_rectf( smallImage->w()+1, 0, w() - (smallImage->w()-1), h() );
    // centered square for the scale:
    fl_line_style(FL_DASH);
    fl_color(FL_DARK3);
    if ( Function::system == LINEAR ) {
	fl_rect( (int)schemaScale.transX(-0.5), (int)schemaScale.transY(0.5),
		 (int)schemaScale.scaleX(1), (int)schemaScale.scaleY(-1) );
    } else if ( Function::system == SINUSOIDAL ) {
	fl_rect( (int)schemaScale.transX(-M_PI_2), (int)schemaScale.transY(M_PI_2),
		 (int)schemaScale.scaleX(M_PI), (int)schemaScale.scaleY(-M_PI) );
    } else { // JULIA, FORMULA
	fl_line( (int)schemaScale.transX(-2), (int)schemaScale.transY(0),
		 (int)schemaScale.transX(2), (int)schemaScale.transY(0) );
	fl_line( (int)schemaScale.transX(-1), (int)schemaScale.transY(0)-3,
		 (int)schemaScale.transX(-1), (int)schemaScale.transY(0)+3);
	fl_line( (int)schemaScale.transX(1), (int)schemaScale.transY(0)-3,
		 (int)schemaScale.transX(1), (int)schemaScale.transY(0)+3);
	fl_line( (int)schemaScale.transX(0), (int)schemaScale.transY(-2),
		 (int)schemaScale.transX(0), (int)schemaScale.transY(2) );
	fl_line( (int)schemaScale.transX(0)-3, (int)schemaScale.transY(-1),
		 (int)schemaScale.transX(0)+3, (int)schemaScale.transY(-1));
	fl_line( (int)schemaScale.transX(0)-3, (int)schemaScale.transY(1),
		 (int)schemaScale.transX(0)+3, (int)schemaScale.transY(1));
    }
    fl_line_style(0);
    // skeleton:
    skel.drawSkeleton( schemaScale, mouseRotDil );
    // frame around smallImage:
    fl_color( ImageGray::background.isBlack() ? FL_GRAY : FL_BLACK ); 
    fl_rect( -1, -1, smallImage->w() + 2, smallImage->h() + 2 );
    if ( printDimension && Function::system == LINEAR ) {
	// dimension
	fl_draw( ("Dim = " + IS::translate(skel.dimension())).c_str(), 2, h() - 3 );
    }
    if ( printCoordinates ) {
	skel.getFunction().printCoordinates( 2, h() - 14*6 - 6 );
    }
}

void
Glito::draw() {
    static bool started = false;
    if ( !started ) {
        started = true;
#ifdef WIN32
	// hack for Windows to make the the layout appear:
	Fl_Group::size( w()-1, h() );
	Fl_Group::size( w()+1, h() );
#endif
	drawHandler();
    } else if ( state == PREVIEW ) {
        make_current();
	drawSchema();
	smallImage->mem_draw();
    }
}

bool
Glito::isMovingFunctionCenter() const {
    return !mouseRotDil && Fl::event_state(FL_BUTTON1)
      && !Fl::event_state(FL_BUTTON2) && !Fl::event_state(FL_BUTTON3);
}

int
Glito::handle( int event ) {
  /*    static int oldevent = -1;
    if ( oldevent != event ) {
        cout << "State: " << getState() << "  Event: " << event << endl;
	oldevent = event;
	}*/
//    if ( event == FL_PUSH ) { //necessary for wine
//	return 1;
//    }
    if ( event == FL_NO_EVENT ) {
        if ( state == PREVIEW ) {
	    make_current();
	    drawSchema();
	    smallImage->mem_draw();
	} else if ( state >= SAVEPGM && state <= SAVEMNG ) {
  	    if ( state == SAVEMNG && (animationSavedWidth < w() || animationSavedHeight < h()) 
		 || (imageSavedWidth < w() || imageSavedHeight < h()) ) {
	        make_current();
		fl_color(FL_DARK3);
		fl_rectf( 0, 0, w(), h() );
	    }
	}
    } else if ( event == FL_SHORTCUT ) {
	// recupere la menu_bar
	Fl_Menu_Bar* m = (Fl_Menu_Bar*)parent()->child(0);
	m->test_shortcut();
	return 1;
    } else if ( event == FL_FOCUS ) {
        needRedraw = true;
        // give the keyboard focus to this window:
	return 1;
    } else if ( state != PREVIEW ) {
	if ( event == FL_KEYBOARD ) {
	    const int c = Fl::event_key();
	    if ( c == FL_Escape ) {
	        if ( state == CALIBRATE ) {
		    resetImage( w(), h() );
		    state = PREVIEW;
		    needRedraw = true;
		    return 1;
		}
		static bool stopped = false;
	        if ( stopped ) {
		    stopped = false;
		} else {
		    stopped = true;
		    fl_color( FL_YELLOW );
		    make_current();
		    fl_draw( _("press ESC to continue"), 1, h()-4 );
		    while ( stopped ) {
		        if ( Fl::ready() ) Fl::check();
	            }
		}
	        return 1;
	    } else if ( c == ' ' || c == 'b' )  {
		if ( state == CALIBRATE ) {
		    return 1;
		}
	        if ( state >= SAVEPGM && state <= SAVEPNG ) {
		    save( state, skel.toXML() );		    
		}
		state = PREVIEW;
		needRedraw = true;
		return 1;
	    } else if ( c == FL_Up ) {
  	        imageLarge->shiftStart(0, shiftSizeImage);
		return 1;
	    } else if ( c == FL_Down ) {
  	        imageLarge->shiftStart(0, -shiftSizeImage);
		return 1;
	    } else if ( c == FL_Left ) {
  	        imageLarge->shiftStart(shiftSizeImage, 0);
		return 1;
	    } else if ( c == FL_Right ) {
  	        imageLarge->shiftStart(-shiftSizeImage, 0);
		return 1;
	    }
	}
	return 0;
    } else if ( state == PREVIEW ) {
        if ( event == FL_PUSH ) {
	    if ( isMovingFunctionCenter() ) { 
  	        mousePushedX = schemaScale.inverseX( Fl::event_x() );
	        mousePushedY = schemaScale.inverseY( Fl::event_y() );
	    }
	    return 1;
	} else if ( event == FL_DRAG ) {
	    const float mx = schemaScale.inverseX( Fl::event_x() );
	    const float my = schemaScale.inverseY( Fl::event_y() );
	    int button = Fl::event_button();
	    /// we emulate the third button if it does not exist:
	    if ( Fl::event_state(FL_BUTTON1) && Fl::event_state(FL_BUTTON3) ) {
		button = FL_MIDDLE_MOUSE;
	    }
	    if ( isMovingFunctionCenter() ) {
	        skel.mouseCandidate( mx-mousePushedX, my-mousePushedY, button, mouseRotDil );
		mousePushedX = mx;
		mousePushedY = my;
	    } else {
	        skel.mouseCandidate( mx, my, button, mouseRotDil );
	    }
	    needRedraw = true;
	    return 1;
	} else if ( event == FL_KEYBOARD ) {
	    const int c =  Fl::event_key();
	    if ( c == FL_Down || c == FL_Up ) {
		make_current();
		smallImage->mem_draw();
		if ( c == FL_Down ) { // rotation
		    if ( Fl::get_key(FL_Down) ) {
			skel.rotate(rotationShift);
		    } else {
		        return 1;
		    }
		} else if( c == FL_Up ) { // rotation
		    if ( Fl::get_key(FL_Up) ) {
			skel.rotate(-rotationShift);
		    } else {
		        return 1;
		    }
		}
		needRedraw = true;
		return 1;
	    } else if ( c == FL_Left || c == FL_Right ) {
		if ( c == FL_Right ) {
		    skel.shiftSelectedFunction( 1 );
		} else if( c == FL_Left ) {
		    skel.shiftSelectedFunction( -1 );
		}
		drawSchema();
		return 1;
	    } else if ( c == 'b' ) {
		state = LARGEVIEW;
		drawLargeView();
		return 1;
	    } else {
 		return 0;
 	    }
	}
    }
    return 0;
}

void
Glito::cut() {
    functionCopied = skel.getFunction();
    if ( !skel.remove() ) {
	fl_alert( _("Can not remove this function.\nThe system must contain 1 function at least.") );
    }
}
void
Glito::copy() {
    functionCopied = skel.getFunction();
}
void
Glito::paste() {
    if ( skel.size() + 1 < skel.NBM ) {
	skel.addFunction( functionCopied );
    } else {
	fl_alert( _("Can not paste this function.\nThe number of functions can not exceed %d."),
		  skel.NBM-1 );
    }
}

void
Glito::drawHandler() {
    while ( true ) {
        if ( state == PREVIEW ) {
	    drawPreview();
	}
    }
}

void
Glito::drawPreview() {
    const Zoom zoom( skel.findFrame( (int)(100 + previewSize*previewSize*pointsForFraming), _x, _y, _color ),
		     smallImage->w(), smallImage->h(), skel.getZoomFunction() );
    int counter = 1;
    const int refresh = (int)(minimalBuiltPoints/5 + previewSize*previewSize * pointsPerFrame);
    const unsigned long start0 = clock();
    unsigned long clock0 = clock();
    make_current(); 
    drawSchema();
    smallImage->mem_clear();
    while ( state == PREVIEW ) {
	iterBuildPoints( skel, zoom, *smallImage, minimalBuiltPoints/5 );
	counter += minimalBuiltPoints/5;
	if ( clockNumber ) {
	    if ( clock() - clock0 >= intervalFrame * timecv ) {
		clock0 = clock();
		counter = 0; // to avoid counter > max_int
		make_current();
		smallImage->mem_draw();
	    }
	} else if ( counter >= refresh ) {
	    counter = 0; // to avoid counter > max_int
	    make_current();
	    smallImage->mem_draw();
	}
	// clock() and Fl::check() cost a lot of time under windows
	if ( clock() - start0 >= intervalMotionDetection * timecv ) {
	     if ( Fl::ready() ) {
	         Fl::check();
	     }
	     if ( needRedraw ) {
	         break;
	     }
	}
    }
    needRedraw = false;
}

void
Glito::calibrate( const int measureTime, const int frames ) {
    Skeleton skelTemp;
    const MinMax minmax = skelTemp.findFrame( pointsForFraming, _x, _y, _color );
    const Zoom zoom( minmax, w(), h(), skelTemp.getZoomFunction() );
    const int progressionBarHeight = 10;
    resetImage( w(), h()-progressionBarHeight );
    unsigned long clock0 = clock();
    intervalFrame = 1000/frames;
    time_t duration = 0;
    int points;
    do {
	time_t time0 = time((time_t*)0);
	points = 0;
	make_current();
	for ( int i = 1; state == CALIBRATE && i <= frames * measureTime; ++i ) {
	    points += drawPoints( skelTemp, zoom, *imageLarge, clock0 );
	    float done = (float)i/frames/measureTime;
	    fl_color(FL_YELLOW);
	    fl_rectf( 0, h()-progressionBarHeight, (int)(done*w()), progressionBarHeight/2 );
	}
	duration = time((time_t*)0) - time0;
#ifdef DEBUG
	cout << "duration= " << duration << " interv= " << intervalFrame << endl;
#endif
	fl_color(FL_BLACK);
	fl_rectf( 0, h()-progressionBarHeight, w(), progressionBarHeight/2 );
	fl_color(FL_YELLOW);
	fl_rectf( 0, h()-progressionBarHeight/2,
		  measureTime*w()/duration, progressionBarHeight/2 );
	intervalFrame = intervalFrame * measureTime / duration; 
	if ( intervalFrame == 0 ) {
	    fl_alert( _("System to slow to show %d frames per second."), frames );
	    break;
	}
    } while ( duration > measureTime && state == CALIBRATE );
    if ( state == CALIBRATE ) {
        pointsPerFrame = points/duration/frames;
	pointsForFraming = 5*pointsPerFrame;
    }
    clockNumber = false;
    resetImage( w(), h() );
}

void
Glito::setSystemType() {
    setSchemaScale( w(), h() ); // because schemaScale depends on systemType
    Fl_Menu_Bar* m = (Fl_Menu_Bar*)parent()->child(0);
#ifdef HAVE_LIBPNG
    const int shift = 3;
#else
    const int shift = 0;
#endif
    const int begin = 33;
    m->mode( begin + 0 + shift, FL_MENU_RADIO );
    m->mode( begin + 1 + shift, FL_MENU_RADIO );
    m->mode( begin + 2 + shift, FL_MENU_RADIO );
    m->mode( begin + 3 + shift, FL_MENU_RADIO|FL_MENU_DIVIDER );
    if ( Function::system == 3 ) {
	m->mode( begin + Function::system + shift, FL_MENU_RADIO|FL_MENU_VALUE|FL_MENU_DIVIDER );
    } else {
	m->mode( begin + Function::system + shift, FL_MENU_RADIO|FL_MENU_VALUE );
    }
}

void
Glito::readParameters( const string& paramXML ) {
    setCloseEdge( atof(IS::ToXML::extractFirst( paramXML, "closeEdge" ).c_str()) );
    setPreviewSize( atof(IS::ToXML::extractFirst( paramXML, "previewSize" ).c_str()) );
    pointsForFraming = atoi(IS::ToXML::extractFirst( paramXML, "pointsForFraming" ).c_str());
    animationFraming = atof(IS::ToXML::extractFirst( paramXML, "animationFraming" ).c_str());
    minimalBuiltPoints = atoi(IS::ToXML::extractFirst( paramXML, "minimalBuiltPoints" ).c_str());
    pointsPerFrame = atoi(IS::ToXML::extractFirst( paramXML, "pointsPerFrame" ).c_str());
    intervalFrame = atoi(IS::ToXML::extractFirst( paramXML, "intervalFrame" ).c_str());
    intervalMotionDetection =
	atoi(IS::ToXML::extractFirst( paramXML, "intervalMotionDetection" ).c_str());
    rotationShift = atof(IS::ToXML::extractFirst( paramXML, "rotationShift" ).c_str());
    imageSavedWidth = atoi(IS::ToXML::extractFirst( paramXML, "imageSavedWidth" ).c_str());
    imageSavedHeight = atoi(IS::ToXML::extractFirst( paramXML, "imageSavedHeight" ).c_str());
    {
        // to stay compatible with 1.0 param files without animationSavedWidth
        const int cand = atoi(IS::ToXML::extractFirst( paramXML, "animationSavedWidth" ).c_str());
	if ( cand ) {
	    animationSavedWidth = cand;
	}
    }
    {
        const int cand = atoi(IS::ToXML::extractFirst( paramXML, "animationSavedHeight" ).c_str());
	if ( cand ) {
	    animationSavedHeight = cand;
	}
    }
    framesPerCycle = atoi(IS::ToXML::extractFirst( paramXML, "framesPerCycle" ).c_str());
    ImagePseudoDensity::pseudoDensity.setLogProbaHitMax(
	atof( IS::ToXML::extractFirst( paramXML, "logProbaHitMax" ).c_str() )
	);
    clockNumber = IS::ToXML::extractFirst( paramXML, "clockNumber" ) == "true";
    ImageGray::background.setBlack(
	IS::ToXML::extractFirst( paramXML, "blackBackground" ) == "true"
	);
    ImageGray::transparency.setTransparencyFromXML(
	IS::ToXML::extractFirst( paramXML, "transparency" )
	);
    trueDensity = IS::ToXML::extractFirst( paramXML, "trueDensity" ) == "true";
    resetImage( w(), h() );
    resetSmallImage( w(), h() );
    Function::systemFromXML( paramXML );
#ifdef HAVE_LIBPNG
    snapshot.fromXML( paramXML );
#endif // HAVE_LIBPNG
    setSystemType();
}

string
Glito::parametersToXML( const int level ) const {
    return IS::ToXML(level)
	.elementI( "frameWidth", w() )
	.elementI( "frameHeight", h() )
	.elementI( "closeEdge", closeEdge )
	.elementI( "previewSize", previewSize )
	.elementI( "pointsForFraming", pointsForFraming )
	.elementI( "animationFraming", animationFraming )
	.elementI( "minimalBuiltPoints", minimalBuiltPoints )
	.elementI( "pointsPerFrame", pointsPerFrame )
	.elementI( "intervalFrame", intervalFrame )
	.elementI( "intervalMotionDetection", intervalMotionDetection )
	.elementI( "rotationShift", rotationShift )
	.elementI( "imageSavedWidth", imageSavedWidth )
	.elementI( "imageSavedHeight", imageSavedHeight )
	.elementI( "animationSavedWidth", animationSavedWidth )
	.elementI( "animationSavedHeight", animationSavedHeight )
	.elementI( "framesPerCycle", framesPerCycle )
	.elementI( "logProbaHitMax", ImagePseudoDensity::pseudoDensity.getLogProbaHitMax() )
	.elementI( "clockNumber", clockNumber )
	.elementI( "blackBackground", ImageGray::background.isBlack() )
	.elementI( "transparency", ImageGray::transparency.transparencyToXML() )
	.elementI( "trueDensity", trueDensity )
	.add( Function::systemToXML(level) )
#ifdef HAVE_LIBPNG
      	.add( snapshot.toXML(level) )
#endif // HAVE_LIBPNG
	.getValue();
}
