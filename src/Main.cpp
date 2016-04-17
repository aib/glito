// glito/Main.cpp  v1.1  2004.09.05
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

#include <iostream>

#include <FL/Fl.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Help_Dialog.H>
#include <FL/Fl_Value_Input.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Float_Input.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Round_Button.H>
#include <FL/Fl_Pixmap.H>
/*
#include <FL/x.H>
#include "icon.xbm"
*/
#include "IndentedString.hpp"
#include "Glito.hpp"
#include "Skeleton.hpp"
// include config.h:
#include "Image.hpp"

#ifdef HAVE_UNISTD_H
# include <unistd.h>
// getopt
#endif

#ifdef ENABLE_NLS
# include <libintl.h>
# define _(String) gettext (String)
#else
# define _(String) (String)
#endif

// default paramFile
string paramFile = "param.xml";

// set in main()
string manualFile;

Glito* glito = (Glito*)0;

//////////////////////////////////////////////////////////////////////
// Menu File

void skeleton_open_cb( Fl_Widget* w, void* ) {
#ifdef HAVE_LIBPNG
    const char *p = fl_file_chooser( _("Open Skeleton"), "*.{ifs,png}", NULL );
    if ( p != NULL ) {
	string skeleton;
        try {
	    skeleton = ImageGray::getDescriptionFromPNG(p);
	} catch ( const int e ) {
	    if ( e == 1 ) { // not a PNG file. Maybe an IFS file
	        skeleton = IS::readStringInFile(p);
	    }
	}
#else
    const char *p = fl_file_chooser( _("Open Skeleton"), "*.ifs", NULL );
    if ( p != NULL ) {
	string skeleton = IS::readStringInFile(p);
#endif
	if ( !glito->skel.fromXML(skeleton) ) {
	    fl_alert( _("Failed to open \"%s\"."), p );
	} else {
	    glito->setSystemType();
	    glito->state = PREVIEW;
	    glito->needRedraw = true;
	}
    }
}

void skeleton_save_cb( Fl_Widget* w, void* ) {
    const char *p = fl_file_chooser( _("Save Skeleton"), "*.ifs", "*.ifs" );
    if ( p != NULL ) {
	IS::saveStringToFile( p, glito->skel.toXML() );
    }
}

void export_fractint_cb( Fl_Widget* w, void* ) {
    const char *p = fl_file_chooser( _("Export Skeleton to Fractint"), "*.ifs", "*.ifs" );
    if ( p != NULL ) {
	const string file(p);
	IS::saveStringToFile( p, glito->skel.toFractint(file.substr(0,file.find('.'))) );
    }
    glito->needRedraw = true;
}

#ifdef HAVE_LIBPNG
void set_snapshotPath_cb( Fl_Widget* w, void* ) {
    glito->setSnapshotPath();
}

void saveSnapshot_cb( Fl_Widget* w, void* ) {
    glito->saveSnapshot();
    glito->needRedraw = true;
}
#endif // HAVE_LIBPNG

void saveImage_cb( Fl_Widget* w, void* f ) {
    const Image::imageFormat format = (Image::imageFormat)(int)f;
    fl_message( _("Format: '%s'\nResolution: %d x %d\nUse the arrows on the keyboard to move the image.\nPress space bar to save the calculated image."),
		Image::formatToString(format).c_str(),
		glito->imageSavedWidth, glito->imageSavedHeight );
    glito->startSave(format);
}

void colorBW_cb( Fl_Widget*, void* ) {
    glito->setColored( !glito->isColored() );
    glito->needRedraw = true;
}

void readColorMap_cb( Fl_Widget*, void* ) {
    const char* p = fl_file_chooser( _("Open Color Map (pov-ray format)"), "*.map", "*.map" );
    if ( p != NULL ) {
        const string colorText = IS::readStringInFile(p);
	if ( !IS::extractFirst( colorText, "color_map {", "}" ).empty() ) {
	    Image::readColorMap( colorText );
	    glito->state = PREVIEW;
	    glito->needRedraw = true;
	} else {
	    fl_alert( _("Failed to open \"%s\"."), p );
	}
    }
}

void readDefinedMap_cb( Fl_Widget* w, void* v ) {
    if ( !glito->isColored() ) {
        glito->setColored(true);
    }
    const int map = (int)v;
    Image::readDefinedMap( map );
    glito->needRedraw = true;
}

void open_parameters_cb( Fl_Widget*, void* ) {
    const char* p = fl_file_chooser( _("Open Parameters"), "*.xml", "*.xml" );
    if ( p != NULL ) {
	const string param = IS::readStringInFile(p);
	if ( !IS::ToXML::extractFirst( param, "parameters" ).empty() ) {
	    glito->readParameters( param );
	    glito->state = PREVIEW;
	    glito->needRedraw = true;
	} else {
	    fl_alert( _("Failed to open \"%s\"."), p );
	}
    }
}
void save_parameters_cb( Fl_Widget* w, void* ) {
    const char* p = fl_file_chooser( _("Save Parameters"), "*.xml", paramFile.c_str() );
    // bug in fltk ? the second time, paramFile is not shown in the file_chooser
    if ( p != NULL ) {
	paramFile = string(p);
	IS::ToXML()
	    .elementIR( "parameters", glito->parametersToXML(0) )
	    .save( paramFile );
    }
}

void framesPerCycle_param( Fl_Value_Slider* o, void* m ) {
    Glito* glito = (Glito*)m;    
    const int value = (int)o->value();
    if ( value >= 2 ) {
	glito->framesPerCycle = value;
	glito->state = PREVIEW;
	glito->needRedraw = true;
    }
}
void previewSize_param( Fl_Value_Slider* o, void* m ) {
    Glito* glito = (Glito*)m;    
    glito->setPreviewSize( o->value() );
}
void closeEdge_param( Fl_Value_Slider* o, void* m ) {
    Glito* glito = (Glito*)m;    
    glito->setCloseEdge( o->value() );
}
void rotationShift_param( Fl_Value_Slider* o, void* m ) {
    Glito* glito = (Glito*)m;    
    glito->rotationShift = o->value();
}
void intervalFrame_param( Fl_Value_Slider* o, void* m ) {
    Glito* glito = (Glito*)m;    
    glito->intervalFrame = (int)o->value();
}
void intervalMotionDetection_param( Fl_Value_Slider* o, void* m ) {
    Glito* glito = (Glito*)m;    
    glito->intervalMotionDetection = (int)o->value();
}

void trueDensity_param( Fl_Button* w, void* m ) {
    Glito* glito = (Glito*)m; 
    const bool b = !glito->trueDensity;
    w->value( b );
    glito->trueDensity = b;
    const int index = w->parent()->find(*w);
    if ( glito->trueDensity ) {
	w->parent()->child( index + 1 )->deactivate(); // "or pseudo-density:"
	w->parent()->child( index + 2 )->deactivate(); // input pseudo-density
    } else {
	w->parent()->child( index + 1 )->activate(); // "or pseudo-density:"
	w->parent()->child( index + 2 )->activate(); // input pseudo-density
    }
    glito->resetImage( glito->w(), glito->h() );
    glito->resetSmallImage( glito->w(), glito->h() );
}
void logProbaHitMax_param( Fl_Value_Slider* o, void* ) {
    ImagePseudoDensity::pseudoDensity.setProba(o->value());
}

void imageSavedWidth_param( Fl_Int_Input* o, void* m ) {
    Glito* glito = (Glito*)m;    
    const int value = atoi( o->value() );
    if ( 1 <= value && value <= 15000 ) {
	glito->imageSavedWidth = value;
    } else {
	fl_alert( _("Width out of range.") );
    }
}
void imageSavedHeight_param( Fl_Int_Input* o, void* m ) {
    Glito* glito = (Glito*)m;    
    const int value = atoi( o->value() );
    if ( 1 <= value && value <= 15000 ) {
	glito->imageSavedHeight = value;
    } else {
	fl_alert( _("Height out of range.") );
    }
}
void animationSavedWidth_param( Fl_Int_Input* o, void* m ) {
    Glito* glito = (Glito*)m;    
    const int value = atoi( o->value() );
    if ( 1 <= value && value <= 15000 ) {
	glito->animationSavedWidth = value;
    } else {
	fl_alert( _("Width out of range.") );
    }
}
void animationSavedHeight_param( Fl_Int_Input* o, void* m ) {
    Glito* glito = (Glito*)m;    
    const int value = atoi( o->value() );
    if ( 1 <= value && value <= 15000 ) {
	glito->animationSavedHeight = value;
    } else {
	fl_alert( _("Height out of range.") );
    }
}
void pointsForFraming_param( Fl_Int_Input* o, void* m ) {
    Glito* glito = (Glito*)m;    
    const int value = atoi( o->value() );
    if ( 1 <= value ) {
	glito->pointsForFraming = value;
    } else {
	fl_alert( _("Value out of range.") );
    }
}
void animationFraming_param( Fl_Value_Slider* o, void* m ) {
    Glito* glito = (Glito*)m;    
    glito->animationFraming = o->value();
}
void pointsPerFrame_param( Fl_Int_Input* o, void* m ) {
    Glito* glito = (Glito*)m;    
    const int value = atoi( o->value() );
    if ( 1 <= value ) {
	glito->pointsPerFrame = value;
    } else {
	fl_alert( _("Value out of range.") );
    }
}

void clockNumber_param( Fl_Button* w, void* m ) {
    Glito* glito = (Glito*)m;
    glito->clockNumber = !glito->clockNumber;
    const int index = w->parent()->find(*w);
    if ( glito->clockNumber ) {
	w->parent()->child( index + 1 )->activate();   // clock
	w->parent()->child( index + 3 )->deactivate(); // pointsPerFrame
	w->parent()->child( index + 4 )->deactivate(); // calibrate
    } else {
	w->parent()->child( index - 1 )->deactivate(); // clock
	w->parent()->child( index + 1 )->activate();   // pointsPerFrame
	w->parent()->child( index + 2 )->activate();   // calibrate
    }
}

void blackWhite_param( Fl_Button* w, void* ) {
    const bool b = !ImageGray::background.isBlack();
    w->value( !b );
    ImageGray::background.setBlack( b );
    glito->needRedraw = true;
}

void transparency_param( Fl_Button* w, void* t ) {
    const int trns = (int)t;
    if ( trns == 0 ) {
	ImageGray::transparency.setNoTransparency();
    } else if ( trns == 1 ) {
	ImageGray::transparency.setSimpleTransparency();
    } else if ( trns == 2 ) {
	ImageGray::transparency.setAlphaTransparency();
    }
}

void calibrate_param( Fl_Button* w, void* m ) {
    const int measureTime = 8; // 8 seconds
    static int frames = 16; // 16 frames per second
    if ( fl_ask( _("Do you want the program to calibrate the number\nof pixels to calculate for each frame?\n\n(Esc to cancel the process)") ) ) {
	const char* answer = fl_input( _("How many frames per second?"),
				       IS::translate(frames).c_str() );
	if ( answer ) {
	    frames = atoi(answer);
	    Glito* glito = (Glito*)m;
	    glito->state = CALIBRATE;
	    glito->calibrate( measureTime, frames );
	    glito->state = PREVIEW;
 	    glito->redraw();
	    const int index = w->parent()->find(*w);
	    Fl_Int_Input* input = (Fl_Int_Input*)w->parent()->child( index - 1 );
	    input->value( IS::translate(glito->pointsPerFrame).c_str() );
	}
    }
}

Fl_Window* paramWindow = (Fl_Window*)0;

void initParamWindow() {
    const int heightInput = 28;
    const int wideLabel = 205;
    const int wideInput = 130;
    const int between = 5;
    const int Ybetween = 3;
    const int YbetweenMore = 6;
    const int alignStyle = FL_ALIGN_INSIDE | FL_ALIGN_RIGHT;
    const Fl_Boxtype inputStyle = FL_PLASTIC_DOWN_BOX;
    int x = 10;
    int y = 10;
    const int wide = x+wideLabel+between+wideInput+x;
    const int height =  y + 14*(heightInput+Ybetween) - Ybetween + 9*YbetweenMore + y;
    paramWindow = new Fl_Window( wide, height, _("Parameters") );
    Fl_Group* win2 = new Fl_Group( 0, 0, wide, height );
    paramWindow->resizable(win2);
    {
	Fl_Box* o = new Fl_Box( x, y, wideLabel, heightInput, _("Schema size:") );
	o->align( alignStyle );
    }
    {
	Fl_Value_Slider* o = new Fl_Value_Slider( x+wideLabel+between, y, wideInput, heightInput );
	o->box(inputStyle);
	o->type(FL_HORIZONTAL);
	o->minimum(0);
	o->maximum(1);
	o->value(glito->getCloseEdge());
	o->step(0.01);
	o->callback( (Fl_Callback*)closeEdge_param, glito );
    }
    y += heightInput + Ybetween;
    {
	Fl_Box* o = new Fl_Box( x, y, wideLabel, heightInput, _("Preview Size:") );
	o->align( alignStyle );
    }
    {
	Fl_Value_Slider* o = new Fl_Value_Slider( x+wideLabel+between, y, wideInput, heightInput );
	o->box(inputStyle);
	o->type(FL_HORIZONTAL);
	o->minimum(0);
	o->maximum(1);
	o->value(glito->getPreviewSize());
	o->step(0.01);
	o->callback( (Fl_Callback*)previewSize_param, glito );
    }
    y += heightInput + Ybetween + YbetweenMore;
    {
	Fl_Box* o = new Fl_Box( x, y, wideLabel, heightInput, _("Rotation shift (rad):") );
	o->align( alignStyle );
    }
    {
	Fl_Value_Slider* o = new Fl_Value_Slider( x+wideLabel+between, y, wideInput, heightInput );
	o->box(inputStyle);
	o->type(FL_HORIZONTAL);
	o->minimum(0.002);
	o->maximum(0.2);
	o->value(glito->rotationShift);
	o->step(0.001);
	o->callback( (Fl_Callback*)rotationShift_param, glito );
    }
    y += heightInput + Ybetween + YbetweenMore;
    {
	Fl_Box* o = new Fl_Box( x, y, wideLabel, heightInput, _("Size of saved images:") );
	o->align( alignStyle );
    }
    {
	Fl_Int_Input* o = new Fl_Int_Input( x+wideLabel+between, y, wideInput/2-5, heightInput );
	o->box(inputStyle);
	o->value(IS::translate(glito->imageSavedWidth).c_str());
	o->callback( (Fl_Callback*)imageSavedWidth_param, glito );
    }
    {
	Fl_Box* o = new Fl_Box( x+wideLabel+between+wideInput/2-5, y, 10, heightInput, "x" );
	o->align( FL_ALIGN_INSIDE | FL_ALIGN_CENTER );
    }
    {
	Fl_Int_Input* o = new Fl_Int_Input( x+wideLabel+between+wideInput/2+5, y,
					    wideInput/2-5, heightInput );
	o->box(inputStyle);
	o->value(IS::translate(glito->imageSavedHeight).c_str());
	o->callback( (Fl_Callback*)imageSavedHeight_param, glito );
    }
    y += heightInput + Ybetween;
    {
	Fl_Box* o = new Fl_Box( x, y, wideLabel, heightInput, _("Size of animations:") );
	o->align( alignStyle );
    }
    {
	Fl_Int_Input* o = new Fl_Int_Input( x+wideLabel+between, y, wideInput/2-5, heightInput );
	o->box(inputStyle);
	o->value(IS::translate(glito->animationSavedWidth).c_str());
	o->callback( (Fl_Callback*)animationSavedWidth_param, glito );
    }
    {
	Fl_Box* o = new Fl_Box( x+wideLabel+between+wideInput/2-5, y, 10, heightInput, "x" );
	o->align( FL_ALIGN_INSIDE | FL_ALIGN_CENTER );
    }
    {
	Fl_Int_Input* o = new Fl_Int_Input( x+wideLabel+between+wideInput/2+5, y,
					    wideInput/2-5, heightInput );
	o->box(inputStyle);
	o->value(IS::translate(glito->animationSavedHeight).c_str());
	o->callback( (Fl_Callback*)animationSavedHeight_param, glito );
    }
    y += heightInput + Ybetween + YbetweenMore;
    {
	Fl_Box* o = new Fl_Box( x, y, wideLabel, heightInput, _("Frames per cycle (>1):") );
	o->align( alignStyle );
    }
    {
	Fl_Value_Input* o = new Fl_Value_Input( x+wideLabel+between, y, wideInput, heightInput );
	o->box(inputStyle);
	o->minimum(10);
	o->maximum(100);
	o->value(glito->framesPerCycle);
	o->step(1);
	o->callback( (Fl_Callback*)framesPerCycle_param, glito );
    }
    y += heightInput + Ybetween + YbetweenMore;
    {
	Fl_Box* o = new Fl_Box( x, y, wideLabel, heightInput, _("Motion detection (ms):") );
	o->align( alignStyle );
    }
    {
	Fl_Value_Slider* o = new Fl_Value_Slider( x+wideLabel+between, y, wideInput, heightInput );
	o->box(inputStyle);
	o->type(FL_HORIZONTAL);
	o->minimum(1);
	o->maximum(200);
	o->value(glito->intervalMotionDetection);
	o->step(1);
	o->callback( (Fl_Callback*)intervalMotionDetection_param, glito );
    }
    y += heightInput + Ybetween + YbetweenMore;
    {
	{
	    Fl_Button* o = new Fl_Button( x, y, 70, heightInput, _("Density") );
	    o->box(FL_PLASTIC_UP_BOX);
	    o->value( glito->trueDensity );
	    o->callback( (Fl_Callback*)trueDensity_param, glito );
	}
	{
	    Fl_Box* o = new Fl_Box( x, y, wideLabel+30, heightInput, _("or pseudo-density:") );
	    o->align( alignStyle );
	    if ( glito->trueDensity ) o->deactivate();
	}
	{
	    Fl_Value_Slider* o = new Fl_Value_Slider( x+wideLabel+30+between, y,
						      100, heightInput );
	    o->box(inputStyle);
	    if ( glito->trueDensity ) o->deactivate();
	    o->type(FL_HORIZONTAL);
	    o->minimum(0);
	    o->maximum(1);
	    o->value( ImagePseudoDensity::pseudoDensity.getProba() );
	    o->step(0.01);
	    o->callback( (Fl_Callback*)logProbaHitMax_param );
	}
    }
    y += heightInput + Ybetween + YbetweenMore;
    {
	Fl_Box* o = new Fl_Box( x, y, wideInput, heightInput, _("Points for framing:") );
	o->align( FL_ALIGN_INSIDE | FL_ALIGN_LEFT );
    }
    y += heightInput;
    {
	int xLine = 0;
	{
	    Fl_Int_Input* o = new Fl_Int_Input( x+xLine, y, 80, heightInput );
	    o->box(inputStyle);
	    o->value(IS::translate(glito->pointsForFraming).c_str());
	    o->callback( (Fl_Callback*)pointsForFraming_param, glito );
	}
	xLine += 80;
	{
	    Fl_Box* o = new Fl_Box( x+xLine, y, 20, heightInput, " (x " );
	    o->align( FL_ALIGN_INSIDE | FL_ALIGN_CENTER );
	}
	xLine += 20;
	{
	    Fl_Value_Slider* o = new Fl_Value_Slider( x+xLine, y, 100, heightInput );
	    o->box(inputStyle);
	    o->type(FL_HORIZONTAL);
	    o->minimum(0);
	    o->maximum(1);
	    o->box(inputStyle);
	    o->value(glito->animationFraming);
	    o->step(0.01);
	    o->callback( (Fl_Callback*)animationFraming_param, glito );
	}
	xLine += 100;
	{
	    Fl_Box* o = new Fl_Box( x+xLine, y, 110, heightInput, _(" for animation)") );
	    o->align( FL_ALIGN_INSIDE | FL_ALIGN_LEFT );
	}
    }
    y += heightInput + 2*Ybetween + YbetweenMore;
    {
	Fl_Box* o = new Fl_Box( x, y, wide-20, heightInput,
				_("Number of iterations limited by:") );
	o->align( FL_ALIGN_INSIDE | FL_ALIGN_LEFT );
    }
    y += heightInput;
    {
	Fl_Round_Button* o = new Fl_Round_Button( x, y, wideInput, heightInput,
						  _("Interval per frame (ms)") );
	o->box(FL_NO_BOX);
	o->type(FL_RADIO_BUTTON);
	o->when(FL_WHEN_CHANGED);
	o->value(glito->clockNumber);
	o->callback( (Fl_Callback*)clockNumber_param, glito );
    }
    {
	Fl_Value_Slider* o = new Fl_Value_Slider( x+wideLabel+between, y, wideInput, heightInput );
	o->box(inputStyle);
	if ( !glito->clockNumber ) o->deactivate();
	o->type(FL_HORIZONTAL);
	o->minimum(1);
	o->maximum(200);
	o->value(glito->intervalFrame);
	o->step(1);
	o->callback( (Fl_Callback*)intervalFrame_param, glito );
    }
    y += heightInput + Ybetween;
    {
	int xLine = 0;
	{
	    Fl_Round_Button* o = new Fl_Round_Button( x, y, 127, heightInput,
						      _("Points per frame") );
	    o->box(FL_NO_BOX);
	    o->type(FL_RADIO_BUTTON);
	    o->when(FL_WHEN_CHANGED);
	    o->value(!glito->clockNumber);
	    o->callback( (Fl_Callback*)clockNumber_param, glito );
	}
	xLine += 127 + between;
	{
	    Fl_Int_Input* o = new Fl_Int_Input( x+xLine, y, 80, heightInput );
	    o->box(inputStyle);
	    if ( glito->clockNumber ) o->deactivate();
	    o->value(IS::translate(glito->pointsPerFrame).c_str());
	    o->callback( (Fl_Callback*)pointsPerFrame_param, glito );
	}
	xLine += 80 + 2;
	{
	    Fl_Button* o = new Fl_Button( x+xLine, y, wide - (x+xLine) - 10,
					  heightInput, _("Calibrate") );
	    o->box(FL_PLASTIC_UP_BOX);
	    if ( glito->clockNumber ) o->deactivate();
	    o->callback( (Fl_Callback*)calibrate_param, glito );
	}
    }
    y += heightInput + Ybetween + 2*YbetweenMore;
    {
	int xLine = 0;
	{
	    Fl_Button* o = new Fl_Button( x, y, 90, heightInput, _("Black/White") );
	    o->box(FL_PLASTIC_UP_BOX);
	    o->value( !ImageGray::background.isBlack() );
	    o->callback( (Fl_Callback*)blackWhite_param );
	}
	xLine += 90 + 2*between + 5;
	{
	    Fl_Choice* o = new Fl_Choice( x+xLine+110, y, 95, heightInput,
					  _("Transparency:") );
	    o->add( ImageGray::transparency.stringNone().c_str(),
		    0, (Fl_Callback*)transparency_param, (void*)0 );
	    o->add( ImageGray::transparency.stringSimple().c_str(),
		    0, (Fl_Callback*)transparency_param, (void*)1 );
	    o->add( ImageGray::transparency.stringAlpha().c_str(),
		    0, (Fl_Callback*)transparency_param, (void*)2 );
	    if ( ImageGray::transparency.useAlphaTransparency() ) {
		o->value(2);
	    } else if ( ImageGray::transparency.useSimpleTransparency() ) {
		o->value(1);
	    } else {
		o->value(0);
	    }
	}
    }
    y += heightInput + Ybetween;
    {
	// invisible box to allow correct resizing
	Fl_Box* o = new Fl_Box( 0, y, 100, 0 );
	win2->resizable(*o);
    }
    win2->end();
    paramWindow->end();
}

void parameters_cb( Fl_Widget* w, void* ) {
    if ( !paramWindow ) {
        initParamWindow();
    }
    paramWindow->show();
}

void quit_cb( Fl_Widget*, void* ) {
    exit(0);
}

//////////////////////////////////////////////////////////////////////
// Menu Skeleton

void skeleton_new_cb( Fl_Widget* w, void* ) {
    static int nb_tot = 2;
    const char* p = fl_input( _("Number of functions (1->%d) ?"),
			      IS::translate(nb_tot).c_str(),
			      Skeleton::NBM - 1 );
    if ( p != NULL ) {
	nb_tot = atoi(p);
	if ( 1 <= nb_tot && nb_tot <= Skeleton::NBM - 1) {
	    glito->skel = Skeleton( nb_tot );
	    glito->state = PREVIEW;
	    glito->needRedraw = true;
	} else {
	    fl_alert( _("The number of functions must be between 1 and %d"),
		      Skeleton::NBM - 1 );
	}
    }
}

void skeleton_random_cb( Fl_Widget* w, void* ) {
    glito->skel.random();
    if ( glito->state == LARGEVIEW ) {
	glito->drawLargeView();
    } else {
	glito->state = PREVIEW;
	glito->needRedraw = true;
    }	
}

void skeleton_dimension_cb( Fl_Widget* w, void* ) {
    glito->printDimension = !glito->printDimension;
    glito->needRedraw = true;
}
void skeleton_coordinates_cb( Fl_Widget* w, void* ) {
    glito->printCoordinates = !glito->printCoordinates;
    glito->needRedraw = true;
}

void outMemory_cb( Fl_Widget* w, void* memory ) {
    const int numSkelet = (int)memory;
    if ( numSkelet == 1 ) {
	glito->skel = glito->skel1;
    } else if ( numSkelet == 2 ) {
	glito->skel = glito->skel2;
    } else if ( numSkelet == 3 ) {
	glito->skel = glito->skel3;
    } else if ( numSkelet == 4 ) {
	glito->skel = glito->skel4;
    }
    if ( glito->state == LARGEVIEW ) {
	glito->drawLargeView();
    } else {
	glito->state = PREVIEW;
    }	
    glito->needRedraw = true;
}

void inMemory_cb( Fl_Widget* w, void* memory ) {
    const int numSkelet = (int)memory;
    if ( numSkelet == 1 ) {
	glito->skel1 = glito->skel;
    } else if ( numSkelet == 2 ) {
	glito->skel2 = glito->skel;
    } else if ( numSkelet == 3 ) {
	glito->skel3 = glito->skel;
    } else if ( numSkelet == 4 ) {
	glito->skel4 = glito->skel;
    }
    glito->needRedraw = true;
}

//////////////////////////////////////////////////////////////////////
// Menu Function

void function_cut_cb( Fl_Widget* w, void* ) {
    glito->cut();
    glito->state = PREVIEW;
    glito->needRedraw = true;
}
void function_copy_cb( Fl_Widget* w, void* ) {
    glito->copy();
}
void function_paste_cb( Fl_Widget* w, void* ) {
    glito->paste();
    glito->state = PREVIEW;
    glito->needRedraw = true;
}

void function_reshape_cb( Fl_Widget* w, void* ) {
    glito->skel.reshape();
    glito->state = PREVIEW;
    glito->needRedraw = true;
}

void systemType_cb( Fl_Widget* w, void* s ) {
    Function::system = (systemType)(int)s;
    glito->setSystemType();
    if ( glito->state == LARGEVIEW ) {
	glito->drawLargeView();
    } else {
	glito->state = PREVIEW;
	glito->needRedraw = true;
    }	
}

void set_formula( Fl_Button* b, void* g ) {
    const int buttonPosition = 8;
    // we check the value of buttonPosition:
    assert( b->contains( (Fl_Input*)b->parent()->child(buttonPosition) ) );
    try {
	Function::formulaPoint = FormulaPoint(
	    ( (Fl_Input*)b->parent()->child(buttonPosition-3) )->value(),
	    ( (Fl_Input*)b->parent()->child(buttonPosition-1) )->value(),
	    "x y x1 y1 x2 y2 xc yc"
	    );
    }
    catch ( const string& error ) {
	fl_alert( _("Error occured during parsing:\n%s"), error.c_str() );
    }
    Glito* glito = (Glito*)g;
    if ( glito->state == LARGEVIEW ) {
	glito->drawLargeView();
    } else {
	glito->state = PREVIEW;
    }	
    glito->needRedraw = true;
}

void edit_formula_cb( Fl_Widget* w, void* ) {
    const int heightInput = 28;
    const int heightText = 18;
    const int wideLabel = 50;
    const int wideInput = 450;
    const int Ybetween = 3;
    const int YbetweenMore = 6;
    const int alignStyle = FL_ALIGN_INSIDE | FL_ALIGN_RIGHT;
    const int textStyle = FL_ALIGN_INSIDE | FL_ALIGN_LEFT;
    const Fl_Boxtype inputStyle = FL_PLASTIC_DOWN_BOX;
    int x = 10;
    int y = 10;
    const int wide = x+wideLabel+wideInput+x;
    const int height = y + 3*heightText + Ybetween + 3*(heightInput+Ybetween)
	- Ybetween + YbetweenMore + y;
    Fl_Window* win = new Fl_Window( wide, height, _("Formulas editor") );
    Fl_Group* win2 = new Fl_Group( 0, 0, wide, height );
    win->resizable(win2);
    {
	Fl_Box* o =
	    new Fl_Box( x, y, wideLabel+wideInput, heightText,
			_("Enter the formulas of x(n+1) and y(n+1) using prefix notation.") );
	o->align( textStyle );
    }
    y += heightText;
    {
	Fl_Box* o =  new Fl_Box( x, y, wideLabel+wideInput, heightText,
				 _("Variables: x y x1 x2 y1 y2 xc yc rand, and any number.") );
	o->align( textStyle );
    }
    y += heightText;
    {
	Fl_Box* o = new Fl_Box( x, y, wideLabel+wideInput, heightText, _("Functions: ") );
	o->align( textStyle );
    }
    {
	Fl_Box* o = new Fl_Box( x + (int)( fl_width(_("Functions: ")) ), y,
				wideLabel+wideInput, heightText,
				"+ - * / < pow abs atan2 sin cos tan atan ln sign square sqrt." );
	o->align( textStyle );
    }
    y += heightText + Ybetween;
    {
	Fl_Box* o = new Fl_Box( x, y, wideLabel, heightInput, "x <- " );
	o->align( alignStyle );
    }
    {
	Fl_Input* o = new Fl_Input( x+wideLabel, y, wideInput, heightInput );
	o->box(inputStyle);
	o->value( Function::formulaPoint.getStringX().c_str() );
    }
    y += heightInput + Ybetween;
    {
	Fl_Box* o = new Fl_Box( x, y, wideLabel, heightInput, "y <- " );
	o->align( alignStyle );
    }
    {
	Fl_Input* o = new Fl_Input( x+wideLabel, y, wideInput, heightInput );
	o->box(inputStyle);
	o->value( Function::formulaPoint.getStringY().c_str() );
    }
    y += heightInput + Ybetween + YbetweenMore;
    {
	Fl_Return_Button* o = new Fl_Return_Button( wide - x - 150, y, 150, heightInput,
						    _("Set formulas") );
	o->box(FL_PLASTIC_UP_BOX);
	o->callback( (Fl_Callback*)set_formula, glito );
    }
    win2->end();
    win->end();
    win->show();
}

void function_previous_cb( Fl_Widget* w, void* ) {
    glito->skel.shiftSelectedFunction( -1 );
    glito->needRedraw = true;
}
void function_next_cb( Fl_Widget* w, void* ) {
    glito->skel.shiftSelectedFunction( 1 );
    glito->needRedraw = true;
}

//////////////////////////////////////////////////////////////////////
// View mode

void view_large_cb( Fl_Widget* w, void* ) {
    if ( glito->state == PREVIEW ) {
        glito->state = LARGEVIEW;
	glito->drawLargeView();
    } else if ( glito->state == LARGEVIEW ) {
	glito->state = PREVIEW;
        glito->needRedraw = true;
    }
}

//////////////////////////////////////////////////////////////////////
// Mouse mode

void mouse_cb( Fl_Widget* w, void* ) {
    glito->mouseRotDil = !glito->mouseRotDil;
    glito->needRedraw = true;
}

//////////////////////////////////////////////////////////////////////
// Menu Animation

void animation_zoom_cb( Fl_Widget* w, void* ) {
    if ( Function::system == LINEAR ) {
	if ( glito->skel.selected() == 0 ) {
	    fl_alert( _("To zoom inside the general frame is not allowed!") );
	} else if ( !glito->skel.getFunction().isContracting() ) {
	    fl_alert( _("Selected function is not contracting!") );
	} else {
	    glito->state = ANIMATION;
	    glito->zoom();
	}
    } else {
	fl_alert( _("The zoom animation requires a \"linear\" system.") );
    }
}

void animation_transition_cb( Fl_Widget* w, void* ) {
    if ( glito->skel1.size() != glito->skel2.size() ) {
	fl_alert( _("Memory 1 and 2 must have the same number of functions!") );
    } else {
	glito->state = ANIMATION;
	glito->transition();
    }
}

void animation_rotation_cb( Fl_Widget* w, void* ) {
    glito->state = ANIMATION;
    glito->rotation();
}

void animation_stop_cb( Fl_Widget* w, void* ) {
    if ( glito->state == ANIMATION || glito->state == DEMO ) {
        glito->state = PREVIEW;
    }
    glito->needRedraw = true;
}

void saveAnimation_cb( Fl_Widget* w, void* f ) {
    const int anim = (int)f; 
    if ( anim == 0 ) { // zoom
	if ( Function::system == LINEAR ) {
	    if ( glito->skel.selected() == 0 ) {
		fl_alert( _("To zoom inside the general frame is not allowed!") );
		return;
	    }
	} else {
	    fl_alert( _("The zoom animation requires a \"linear\" system.") );
	    return;
	}
    }
    const Image::imageFormat format = Image::MNG;
    fl_message( _("Format: '%s'\nResolution: %d x %d\n\nPress space bar to cancel calculation."),
		Image::formatToString(format).c_str(),
		glito->animationSavedWidth, glito->animationSavedHeight );
    glito->startSave( format, anim );
}

//////////////////////////////////////////////////////////////////////
// Menu Help

void demo_cb( Fl_Widget* w, void* ) {
    glito->state = DEMO;
    glito->demonstration();
}

void documentation_cb( Fl_Widget* w, void* ) {
    Fl_Help_Dialog* help = new Fl_Help_Dialog();
    help->load(manualFile.c_str());
    help->show();
}

void about_cb( Fl_Widget* w, void* ) {
    fl_message( "Glito v%s Copyright (C) 2002-2003 Emmanuel Debanne\n%s", VERSION,
		_("Glito comes with absolutely no warranty.\nGlito is free software.\nFor details, click on Help -> Documentation.") );
}

void usage() {
    cerr << _("Usage:") << " glito [-p " << _("paramFile")
	 << ".xml] [" << _("skeletonFile") << ".ifs]\n"
	 << _("Report bugs to <glito@debanne.net>.\n");
}

static const char *idata_file_open[] = {
"16 16 11 1",
"# c #000000",
"g c #c0c0c0",
"e c #303030",
"a c #ffa858",
"b c #808080",
"d c #a0a0a4",
"f c #585858",
"c c #ffdca8",
"h c #dcdcdc",
"i c #ffffff",
". c None",
"....###.........",
"....#ab##.......",
"....#acab####...",
"###.#acccccca#..",
"#ddefaaaccccca#.",
"#bdddbaaaacccab#",
".eddddbbaaaacab#",
".#bddggdbbaaaab#",
"..edgdggggbbaab#",
"..#bgggghghdaab#",
"...ebhggghicfab#",
"....#edhhiiidab#",
"......#egiiicfb#",
"........#egiibb#",
"..........#egib#",
"............#ee#"
};
static Fl_Pixmap icon_file_open(idata_file_open);

static const char *idata_save_file[] = {
"18 18 7 1",
". c None",
"# c #1c1c1c",
"d c #69a5a5",
"a c #838383",
"e c #bcbcbc",
"c c #f8eee6",
"b c #fcf9f6",
"..................",
"..................",
"...############...",
"..#a#bbbbbbcb#a#..",
"..#a#bddddddb#a#..",
"..#a#bbbbbbbb#a#..",
"..#a#bddddddb#a#..",
"..#a#bbbbbbbb#a#..",
"..#aa########aa#..",
"..#aaaaaaaaaaaa#..",
"..#aaa#######aa#..",
"..#aa#eeee#aa#a#..",
"..#aa#e##e#aa#a#..",
"..#aa#e#ae#aa#a#..",
"..#aa#eeee#aa#a#..",
"...############...",
"..................",
".................."
};
static Fl_Pixmap icon_save_file(idata_save_file);

static const char *idata_save_fast[] = {
"18 18 8 1",
". c None",
"b c #1c1c1c",
"e c #69a5a5",
"c c #838383",
"f c #bcbcbc",
"# c #ec8a20",
"d c #fcf9f6",
"a c #ffff00",
"..................",
"...........#aa....",
"...bbbbbbb##abb...",
"..bcbdddd#aadbcb..",
"..bcbdee###aaacb..",
"..bcbddddd##aacb..",
"..bcbdeee##aabcb..",
"..bcbdddd#addbcb..",
"..bccbbb####aacb..",
"..bccccccc#aaccb..",
"..bcccbbbb#abccb..",
"..bccbfff#accbcb..",
"..bccbfb#abccbcb..",
"..bccbfbcfbccbcb..",
"..bccbffffbccbcb..",
"...bbbbbbbbbbbb...",
"..................",
".................."
};
static Fl_Pixmap icon_save_fast(idata_save_fast);

static const char *idata_param[] = {
"18 18 11 1",
". c None",
"c c #173a38",
"g c #38250a",
"# c #3c3c3e",
"e c #7cc2c0",
"f c #906c33",
"d c #bde5e3",
"i c #debd96",
"h c #ebebec",
"a c #eeeeee",
"b c #f4f4f4",
"..................",
".......####.......",
"......#abb#ccc....",
"....##bb###cdc....",
"...#bbb##.cdcccc..",
"...#bb##.cdeccdc..",
".##bb##fgcdeeeec..",
"#hbb##fifgdeeecc..",
"##b##.gfifgeccc...",
".##....gfifgc.....",
"......cdgfifg.....",
".....cdecgfifg....",
"....cdecc.gfifg...",
"...cdecc...gfifg..",
"...cecc.....gfig..",
"....cc.......gg...",
"..................",
".................."
};
static Fl_Pixmap icon_param(idata_param);

static const char *idata_colorbw[] = {
"16 16 13 1",
". c None",
"# c #000000",
"k c #0000c0",
"j c #00c000",
"a c #303030",
"b c #585858",
"c c #808080",
"e c #a0a0a0",
"f c #c3c3c3",
"h c #dcdcdc",
"d c #ff0000",
"g c #ff8000",
"i c #ffff00",
".....###########",
".....###########",
".....#aaaaaaaaa#",
".....#bbbbbbbbb#",
"######ccccccccc#",
"#dddd#eeeeeeeee#",
"#ddddd#ffffffff#",
"#gggggg#fffffff#",
"#ggggggg#hhhhhh#",
"#iiiiiiii#hhhhh#",
"#iiiiiiiii#....#",
"#jjjjjjjjjj#####",
"#jjjjjjjjjj#....",
"#kkkkkkkkkk#....",
"#kkkkkkkkkk#....",
"############...."
};
static Fl_Pixmap icon_colorbw(idata_colorbw);

static const char *idata_colormap[] = {
"16 16 7 1",
". c None",
"b c #0058c0",
"e c #00c000",
"# c #404000",
"a c #f3e7be",
"c c #ff0000",
"d c #ffff00",
"................",
"....#...........",
"...#a#......##..",
"..#aaa#....#aa#.",
".#aa#a#...#aaaa#",
"#aa#.#a#..#abba#",
"#aa#.#aa##abbba#",
"#aaa#aaaaaaabba#",
"#aaaaaaaaaaaaaa#",
".#aaccaaaadddaa#",
".#acccaaeaadda#.",
".#acccaeeeadda#.",
"..#aaaaeeeaaa#..",
"...#aaaaaaaa#...",
"....########....",
"................"
};
static Fl_Pixmap icon_colormap(idata_colormap);

static const char *idata_hazard[] = {
"16 16 12 1",
". c None",
"g c #080808",
"h c #161616",
"# c #303030",
"j c #323434",
"f c #4b5e67",
"b c #818691",
"c c #86a7b8",
"d c #adb1b1",
"a c #b6bdcd",
"i c #f7ffff",
"e c #f8feff",
"....###########.",
"..##aabbaaaabbb#",
".#aabaaabbaaaac#",
"#deeeeeeaabaacf#",
"#ddgdeeeeeeecff#",
"#dgggeeeeeeecfc#",
"#edgdeeeeeeeccc#",
"#eeeeddeeeeeccc#",
"#eeeehgdeeeeccc#",
"#eieejgdeeeecc#.",
"#eieeeeeeeeecf#.",
"#eieeeeejjeeff#.",
"#eeeeeedggdcff#.",
"#eeeeeeejjecf#..",
"######edddd##...",
"......jdjjj....."
};
static Fl_Pixmap icon_hazard(idata_hazard);

static const char *idata_previous[] = {
"16 17 9 1",
". c None",
"a c #000203",
"g c #0d8ac4",
"e c #1183b8",
"f c #137cb2",
"# c #1b466e",
"d c #58b3d9",
"b c #8097b9",
"c c #e1ecf8",
"................",
"................",
"........#.......",
".......#a.......",
"......#ba.......",
".....#bca.......",
"....#bcdaaaaaa..",
"...#bcdddcccda..",
"..#bcddeeeeeda..",
"..a#fdgeddddda..",
"...a#edeeeeeea..",
"....a#edaaaaaa..",
".....a#ea.......",
"......a#a.......",
".......aa.......",
"........a.......",
"................"
};
static Fl_Pixmap icon_previous(idata_previous);

static const char *idata_next[] = {
"16 17 9 1",
". c None",
"a c #000203",
"f c #0d8ac4",
"e c #1183b8",
"g c #137cb2",
"# c #1b466e",
"d c #58b3d9",
"b c #8097b9",
"c c #e1ecf8",
"................",
"................",
".......#........",
".......a#.......",
".......ab#......",
".......acb#.....",
"..aaaaaadcb#....",
"..adcccdddcb#...",
"..adeeeeeddcb#..",
"..adddddefdg#a..",
"..aeeeeeede#a...",
"..aaaaaade#a....",
".......ae#a.....",
".......a#a......",
".......aa.......",
".......a........",
"................"
};
static Fl_Pixmap icon_next(idata_next);

static const char *idata_larger[] = {
"18 17 7 1",
". c None",
"# c #000000",
"b c #585858",
"d c #800000",
"a c #c3c3c3",
"e c #fffffc",
"c c #ffffff",
"..................",
".################.",
".#aaaaaaaaaaaaaa#.",
".#bbbbbbbbbbbbbb#.",
".#cccccccccccccc#.",
".#cdddccccccdddc#.",
".#cddccccccccddc#.",
".#cdcdecccccdcdc#.",
".#ccccdccccccccc#.",
".#cccccccccdcccc#.",
".#cdcdccccccdcdc#.",
".#cddccccccccddc#.",
".#cdddccccccdddc#.",
".#cccccccccccccc#.",
".################.",
"..................",
".................."
};
static Fl_Pixmap icon_larger(idata_larger);

static const char *idata_rotdil[] = {
"16 16 4 1",
". c None",
"# c #000000",
"b c #585858",
"a c #c0c000",
"................",
".....######.....",
"...#####aaa#b...",
"..####...#aa#...",
"..##...#..#aa#..",
".##....#...#a#..",
".##....#.#aaaaa#",
".##.####b.#aaa#.",
".##....#...###..",
"..#....#....#...",
"..##...#........",
"...##...........",
"....###.........",
"......###.......",
"................",
"................"
};
static Fl_Pixmap icon_rotdil(idata_rotdil);

void prepareButton( Fl_Button* o ) {
    o->box( FL_PLASTIC_UP_BOX );
    o->clear_visible_focus(); // button is not traversable with tabs or arrows
}

//////////////////////////////////////////////////////////////////////
// main

int main( int argc, char **argv ) {
    // I18N
#ifdef HAVE_SETLOCALE
    setlocale( LC_MESSAGES, "" ); // with glibc read LC_ALL first
    setlocale( LC_NUMERIC, "POSIX" ); // to avoid incompatibility between ifs files.
#endif
#ifdef ENABLE_NLS
#ifdef WIN32
    bindtextdomain( PACKAGE, "locale" );
#else
    bindtextdomain( PACKAGE, LOCALEDIR );
#endif
    textdomain( PACKAGE );
#endif
    // localization of some strings of FLTK:
    fl_no = _("No");
    fl_yes = _("Yes");
    fl_ok = _("OK");
    fl_cancel = _("Cancel");
    fl_close = _("Close");
    Fl_File_Chooser::add_favorites_label = _("Add to Favorites");
    Fl_File_Chooser::all_files_label = _("All Files (*)");
    Fl_File_Chooser::custom_filter_label = _("Custom Filter");
    Fl_File_Chooser::existing_file_label = _("Please choose an existing file!");
    Fl_File_Chooser::favorites_label = _("Favorites");
    Fl_File_Chooser::filename_label = _("Filename:");
#ifdef WIN32
    Fl_File_Chooser::filesystems_label = _("My Computer");
#else
    Fl_File_Chooser::filesystems_label = _("File Systems");
#endif
    Fl_File_Chooser::manage_favorites_label = _("Manage Favorites");
    Fl_File_Chooser::preview_label = _("Preview");
    Fl_File_Chooser::show_label = _("Show:");
    string skeletonToOpen;
#ifdef HAVE_UNISTD_H
    while ( true ) {
	int c = getopt( argc, argv, "vhp:" );
	if ( c == -1 ) {
	    break;
	}
	switch ( c ) {
	case 'p':
	    paramFile = optarg;
	    break;
	case 'v':
	    cerr << "Glito v" << VERSION << "\nCopyright (C) 2002-2003 Emmanuel Debanne\n"
		 << _("Glito is distributed under the terms of the GNU General Public License.\n");
	    return 0;
	default:
	case 'h':
	    usage();
	    return 0;
	}
    }
    if ( optind < argc ) {
	skeletonToOpen = argv[optind];
	++optind;
	if ( optind < argc ) {
	    usage();
	    return 0;
	}
    }
#endif
    // default size of the window
    int width = 600;
    int height = 450;
    const int menuHeight = 30;
    const int toolbarHeight = 27;
    const int toolHeight = 25;
    const string paramXML( IS::readStringInFile(paramFile) );
    if ( !paramXML.empty() ) {
	if ( !IS::ToXML::extractFirst( paramXML, "parameters" ).empty() ) {
	    width = atoi( IS::ToXML::extractFirst( paramXML, "frameWidth" ).c_str() );
	    height = atoi( IS::ToXML::extractFirst( paramXML, "frameHeight" ).c_str() );
	} else {
	    cerr << _("Failed to open: ") << paramFile << '\n';
	}
    }
    Fl_Window window( width, height + menuHeight + toolbarHeight );
    // if the window is closed, the application exits:
    window.callback(quit_cb);
    Fl_Menu_Bar menubar( 0, 0, width, menuHeight );
    menubar.box(FL_PLASTIC_UP_BOX);
    // in the main to allow gettext to do its job.
#ifdef WIN32
    manualFile = "";
#else
    manualFile = DOCDIR;
#endif
    manualFile += _("manual_en.html");
    static Fl_Menu_Item menutable[] = {
    {_("&File"),            0, 0, 0, FL_SUBMENU}, //0
    {_("&Open"),            FL_CTRL+'O', skeleton_open_cb},
    {_("&Save"),            FL_CTRL+'S', skeleton_save_cb},
    {_("&Export to Fractint"), 0, export_fractint_cb, 0, FL_MENU_DIVIDER},
#ifdef HAVE_LIBPNG
    {_("Set Fast Save Directory"), 0, set_snapshotPath_cb},
    {_("Fast Save"),        'f', saveSnapshot_cb, NULL, FL_MENU_DIVIDER},
    {_("Save PNG"),         0, saveImage_cb, (void*)Image::PNG},
#endif
    {_("Save PGM"),         0, saveImage_cb, (void*)Image::PGM},
    {_("Save BMP bitmap"),  0, saveImage_cb, (void*)Image::BMPB},
    {_("Save BMP gray"),    0, saveImage_cb, (void*)Image::BMPG, FL_MENU_DIVIDER},
    {_("Edit &Paramaters"), 'p', parameters_cb},
    {_("Open Paramaters"),  0, open_parameters_cb},
    {_("Sa&ve parameters"), 0, save_parameters_cb, 0, FL_MENU_DIVIDER},
    {_("&Quit"),	    FL_CTRL+'Q', quit_cb}, //10
    {0},
    {_("&Skeleton"),      0, 0, 0, FL_SUBMENU},
    {_("&New"),           FL_CTRL+'N', skeleton_new_cb},
    {_("&Random"),        'h', skeleton_random_cb, 0, FL_MENU_DIVIDER},
    {_("&Dimension"),     FL_CTRL+'D', skeleton_dimension_cb},
    {_("&Coordinates"),   0, skeleton_coordinates_cb, 0, FL_MENU_DIVIDER},
    {_("out Memory1"),    '1', outMemory_cb, (void*)1},
    {_("out Memory2"),    '2', outMemory_cb, (void*)2},
    {_("out Memory3"),    '3', outMemory_cb, (void*)3},
    {_("out Memory4"),    '4', outMemory_cb, (void*)4, FL_MENU_DIVIDER},//20
    {_("in Memory1"),     '5', inMemory_cb, (void*)1},
    {_("in Memory2"),     '6', inMemory_cb, (void*)2},
    {_("in Memory3"),     '7', inMemory_cb, (void*)3},
    {_("in Memory4"),     '8', inMemory_cb, (void*)4},
    {0},
    {_("&Function"),      0, 0, 0, FL_SUBMENU},
    {_("Pre&vious"),      'v', function_previous_cb},
    {_("&Next"),          'n', function_next_cb, 0, FL_MENU_DIVIDER},
    {_("&Cut"),           FL_CTRL+'X', function_cut_cb},
    {_("C&opy"),          FL_CTRL+'C', function_copy_cb},
    {_("&Paste"),         FL_CTRL+'V', function_paste_cb, 0, FL_MENU_DIVIDER},
    {_("&Reshape"),       's', function_reshape_cb, 0, FL_MENU_DIVIDER}, //32
    // the following items are modified by Glito::setSystemType()
    {_("&Linear"),        0, systemType_cb, (void *)LINEAR,     FL_MENU_RADIO|FL_MENU_VALUE}, //33
    {_("&Sinusoidal"),    0, systemType_cb, (void *)SINUSOIDAL, FL_MENU_RADIO}, //34
    {_("&Julia"),         0, systemType_cb, (void *)JULIA,      FL_MENU_RADIO}, //35
    {_("&Formulas"),      0, systemType_cb, (void *)FORMULA,    FL_MENU_RADIO|FL_MENU_DIVIDER}, //36
    {_("&Edit Formulas"), 0, edit_formula_cb},
    {0},
    {_("&Color"),         0, 0, 0, FL_SUBMENU},
    {_("Open Color Map"), 0, readColorMap_cb},
    {_("Color / B&W"),    0, colorBW_cb, 0, FL_MENU_DIVIDER},
    {_("Maps:")},
    {_("Autumn"),         0, readDefinedMap_cb, (void*)0},
    {_("Fast"),           0, readDefinedMap_cb, (void*)-1},
    {_("CMY"),            0, readDefinedMap_cb, (void*)1},
    {_("RGB"),            0, readDefinedMap_cb, (void*)2},
    {_("Light CMY"),      0, readDefinedMap_cb, (void*)3},
    {_("Dark"),           0, readDefinedMap_cb, (void*)4},
    {_("Rich"),           0, readDefinedMap_cb, (void*)5},
    {0},
    {_("&Animation"),     0, 0, 0, FL_SUBMENU},
    {_("&Zoom"),                      'z', animation_zoom_cb},
    {_("&Transition: Mem1 <-> Mem2"), 't', animation_transition_cb},
    {_("&Rotation"),                  'r', animation_rotation_cb},
    {_("&Stop"),                      ' ', animation_stop_cb, 0, FL_MENU_DIVIDER},
#ifdef HAVE_LIBMNG
    {_("Save Zoom as MNG"),       0, saveAnimation_cb, (void*)0},
    {_("Save Transition as MNG"), 0, saveAnimation_cb, (void*)1},
    {_("Save Rotation as MNG"),   0, saveAnimation_cb, (void*)2},
#endif
    {0},
    {_("&Help"),          0, 0, 0, FL_SUBMENU},
    {_("Docu&mentation"), FL_F+1, documentation_cb},
    {_("&Demo"),          0, demo_cb},
    {_("&About"),         0, about_cb},
    {0},
    {0}
    };
    menubar.menu(menutable);

    // the shortcuts are handled by the menu everywhere:
    //menubar.global();
    glito = new Glito( 0, menuHeight + toolbarHeight, width, height );
    if ( !IS::ToXML::extractFirst( paramXML, "parameters" ).empty() ) {
	glito->readParameters( paramXML );
    }
    if ( !skeletonToOpen.empty() ) {
 	if ( !glito->skel.fromXML( IS::readStringInFile(skeletonToOpen) ) ) {
	    cerr << _("Failed to open: ") << skeletonToOpen << '\n';
	}
 	glito->setSystemType();
    }
    Fl::visual( FL_DOUBLE | FL_INDEX );
    // to allow the preview of PNG files in the file chooser:
    Fl_File_Icon::load_system_icons();
    // the size of glito is changed when the size of the window is changed:
    window.add_resizable(*glito);

    // ********* toolbar ***********
    {
        Fl_Group* o = new Fl_Group( 0, menuHeight, width, toolbarHeight );
	o->resizable(NULL); // not resizable
	int x = 0;
	const int toolSpace = 9;
	const int toolInter = 1;
	{
	    Fl_Button* o = new Fl_Button( x, menuHeight+1, toolHeight, toolHeight );
	    prepareButton(o);
	    o->tooltip( _("Open") );
	    o->image( icon_file_open );
	    o->callback( (Fl_Callback*)skeleton_open_cb );
	    x += toolHeight + toolInter;
	}
#ifdef HAVE_LIBPNG
	{
	    Fl_Button* o = new Fl_Button( x, menuHeight+1, toolHeight, toolHeight );
	    prepareButton(o);
	    o->tooltip( _("Save PNG") );
	    o->image( icon_save_file );
	    o->callback( (Fl_Callback*)saveImage_cb, (void*)Image::PNG );
	    x += toolHeight + toolInter;
	}
	{
	    Fl_Button* o = new Fl_Button( x, menuHeight+1, toolHeight, toolHeight );
	    prepareButton(o);
	    o->tooltip( _("Fast save") );
	    o->image( icon_save_fast );
	    o->callback( (Fl_Callback*)saveSnapshot_cb );
	    x += toolHeight + toolInter;
	}
#endif
	x += toolSpace;
	{
	    Fl_Button* o = new Fl_Button( x, menuHeight+1, toolHeight, toolHeight );
	    prepareButton(o);
	    o->tooltip( _("Parameters") );
	    o->image( icon_param );
	    o->callback( (Fl_Callback*)parameters_cb );
	    x += toolHeight + toolInter;
	}
	x += toolSpace;
	{
	    Fl_Button* o = new Fl_Button( x, menuHeight+1, toolHeight, toolHeight );
	    prepareButton(o);
	    o->tooltip( _("Color / B&W") );
	    o->image( icon_colorbw );
	    o->callback( (Fl_Callback*)colorBW_cb );
	    x += toolHeight + toolInter;
	}
	{
	    Fl_Button* o = new Fl_Button( x, menuHeight+1, toolHeight, toolHeight );
	    prepareButton(o);
	    o->tooltip( _("Open Color Map") );
	    o->image( icon_colormap );
	    o->callback( (Fl_Callback*)readColorMap_cb );
	    x += toolHeight + toolInter;
	}
	x += toolSpace;
	{
	    Fl_Button* o = new Fl_Button( x, menuHeight+1, toolHeight, toolHeight );
	    prepareButton(o);
	    o->tooltip( _("Random") );
	    o->image( icon_hazard );
	    o->callback( (Fl_Callback*)skeleton_random_cb );
	    x += toolHeight + toolInter;
	}
	x += toolSpace;
	{
	    Fl_Button* o = new Fl_Button( x, menuHeight+1, toolHeight, toolHeight );
	    prepareButton(o);
	    o->tooltip( _("Previous function") );
	    o->image( icon_previous );
	    o->callback( (Fl_Callback*)function_previous_cb );
	    x += toolHeight + toolInter;
	}
	{
	    Fl_Button* o = new Fl_Button( x, menuHeight+1, toolHeight, toolHeight );
	    prepareButton(o);
	    o->tooltip( _("Next function") );
	    o->image( icon_next );
	    o->callback( (Fl_Callback*)function_next_cb );
	    x += toolHeight + toolInter;
	}
	x += toolSpace;
	{
	    Fl_Button* o = new Fl_Button( x, menuHeight+1, toolHeight, toolHeight );
	    prepareButton(o);
	    o->tooltip( _("Enlarge") );
	    o->image( icon_larger );
	    o->callback( (Fl_Callback*)view_large_cb );
	    x += toolHeight + toolInter;
	}
       	x += toolSpace;
	{
	    Fl_Light_Button* o = new Fl_Light_Button( x, menuHeight+1, toolHeight+20, toolHeight );
	    prepareButton(o);
	    o->tooltip( _("Rot/Dil") );
	    o->image( icon_rotdil );
	    o->callback( (Fl_Callback*)mouse_cb );
	    x += toolHeight + toolInter;
	}
	o->end();
    }


    window.end();
    window.show(argc,argv);
    /*
    Pixmap p = XCreateBitmapFromData( fl_display, DefaultRootWindow(fl_display),
				      icon_bits, icon_width, icon_height );
    window.icon((char *)p);
    */
    glito->show();
    menubar.show();
    { // seed for the random generator:
	time_t t;
	time(&t); 
	srand( (unsigned int)t );
    }
    return Fl::run();
}
