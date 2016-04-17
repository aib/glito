// glito/Glito.hpp  v1.1  2004.09.05
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

#ifndef GLITO_HPP
#define GLITO_HPP

#include "Image.hpp"
#include "Engine.hpp"

#ifdef HAVE_LIBPNG
/**
 * Used to quickly save a fractal.
 * The user press one touch ('f'), and a PNG snapshot is saved
 * with the IFS skeleton written as a comment in the PNG file.
 * This file can then be reopened by Glito or edited under
 * a text editor to retrieve the skeleton.
 */
class Snapshot {
public:
    Snapshot();

    /// return false if path has not been set by the user
    bool checkPath();

    void setPath();

    void save( const Image& thumbnail, const string& comment ) const;

    int getWidth() const { return width; }

    int getHeight() const { return height; }

    int getIterations() const { return iterations; }

    string toXML( int level ) const;

    void fromXML( const string& paramXML );

private:
    /// directory where are saved the snapshots
    string path;

    /// width of the saved image
    int width;

    /// height of the saved image
    int height;

    /// number of points calculated before saving the image
    int iterations;

};
#endif // HAVE_LIBPNG

class Glito : public Engine {
public:
    /// constructor
    Glito( int cornerX, int cornerY, int w, int h, const char *label = 0 );

    ~Glito() {
	delete smallImage;
    }

    /// launch the computation
    void startSave( const Image::imageFormat format, const int anim = 0 );

    /// automatic demo
    void demonstration();

    /// build and draw #smallImage# indefinitely
    void drawPreview();

    /// cut the selected function of #skel#
    void cut();

    /// copy the selected function of #skel#
    void copy();

    /// paste the last function cut. Becomes the new selected function of #skel#
    void paste();

    /** compute pointsPerFrame so that it takes #intervalFrame# to compute #pointsPerFrame#
	set #intervalFrame# to 1000/#frames#
	pointsForFraming := 5*pointsPerFrame
	clockNumber set to false
    */
    void calibrate( const int measureTime, const int frames );

    void readParameters( const std::string& paramXML );

    std::string parametersToXML( const int level = 0 ) const;

    /// ask the user for the name. The format is defined by #state#
    void save( const State state, const std::string& description );

    /// change the menubar and schemaScale to reflect Function::system
    void setSystemType();

    void setCloseEdge( float value );
    float getCloseEdge() const { return closeEdge; }

    void setPreviewSize( float value );
    float getPreviewSize() const { return previewSize; }

#ifdef DEBUG    
    string getState() const;
#endif

    /// clear the screen and draw the Skeleton
    void drawSchema() const;

    /// true if we print the dimension on the screen
    bool printDimension;

    /// true if we print the coordinates of the current Function on the screen
    bool printCoordinates;

    /// angle of rotation applied when the keys "up arrow" and "down arrow" are hit
    float rotationShift;

    /** time to wait in milli-second before detecting an event
	used in drawPreviw to allow the calculation of a few points
	between each movement of the mouse
    */
    int intervalMotionDetection;

    /// true if the mouse can rotate or dilate a parallelogram
    bool mouseRotDil;

    void resetSmallImage( int width, int height ) {
	delete smallImage;
	smallImage = buildImage( (int)(previewSize*width), (int)(previewSize*height) );
    }

    // { memories
    Skeleton skel3;
    Skeleton skel4;
    // }

#ifdef HAVE_LIBPNG
    void setSnapshotPath();

    void saveSnapshot();
#endif // HAVE_LIBPNG

    void setColored( bool c );//coul
  
    /// true if skeleton has been modified and drawPreview() must be called again
    bool needRedraw;

protected:
    /// ratio between the smallest edge of the window and a line of length 1 in the schema scale
    float closeEdge;

    /// ratio between the size of the window and the size of the preview image
    float previewSize;

    /// after cut or copy, the function is stored in #functionCopied#
    Function functionCopied;

    SchemaScale schemaScale;

    /// set #schemaScale# to fit the window of size #w#*#h#
    void setSchemaScale( int w, int h );

    /// inherited from Fl_Double_Window
    // {
    /// called when the window is resized
    void resize( int XX, int YY, int WW, int HH );

    /// refresh the screen when damaged
    void draw();

    /// handle mouse and keyboard events
    int handle( int event );
    // }

    /// preview
    Image* smallImage;

    /// size of the shift when moving the saved image
    static const int shiftSizeImage = 20;

#ifdef HAVE_LIBPNG
    Snapshot snapshot;
#endif // HAVE_LIBPNG

    /// return true if the user wants to move the center of a function
    bool isMovingFunctionCenter() const;

    /// position of the mouse in the schema when dragging starts
    // {
    float mousePushedX;
    float mousePushedY;
    //}

    /// call drawPreview() when state is PREVIEW
    void drawHandler();
    
};

#endif // GLITO_HPP
