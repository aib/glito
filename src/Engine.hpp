// glito/Engine.hpp  v1.1  2004.09.05
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

#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <vector>

#include <FL/Fl_Double_Window.H>

#include "Skeleton.hpp"
class Image;

#ifdef WIN32
const float timecv = 1; // clock() returns milli-seconds
#else
const float timecv = 1000; // clock() returns micro-seconds
#endif

// order cares
enum State {
    PREVIEW,
    LARGEVIEW,
    ANIMATION,
    DEMO,
    CALIBRATE,
    SAVEPGM,
    SAVEBMPB,
    SAVEBMPG,
    SAVEPNG,
    SAVEMNG
};

/**  variables for Julia orbit to accelerate the calculation
 */
class Julia {
public:
    void start( int nbFrames );
    void handle( float&x , float& y, const int nbHit );

private:
    void push( float x, float y );
    void pop( float& x, float& y );

    std::vector<float> vx;
    std::vector<float> vy;
    int nbHitMin;
    int vSizeMax;
};

class Zoom {
public:
    /// constructor
    Zoom( const MinMax& minmax, const int w, const int h, const Function& f, int nbFrames = 1 );
    
    /// result is stored in screenX and screenY
    void toScreen( float x, float y ) const;

    mutable int screenX;
    mutable int screenY;

    mutable Julia julia;

protected:
    Function zoomFunction;

    bool zoomFunctionModified;

    float fx;
    float fy;
    int centerX;
    int centerY;

    /**
     * add a border to avoid for some pixel of the fractals to be out of the image.
     * example: 0.96 That means that the fractale is 96% smaller to leave place for
     * a border of 4%.
     */
    float framingCorrection;

};

class Engine : public Fl_Double_Window {
public:
    Engine( int cornerX, int cornerY, int w, int h, const char *label = 0 );

    /// delete images and imageLarge
    ~Engine();

    // { animations
    void transition();

    void rotation();

    void zoom();
    // }

    /// build and draw #image# indefinitely
    void drawLargeView();

    /// state ( preview, animation, saving, ... )
    State state;

    /// current skeleton
    Skeleton skel;

    // { memories
    Skeleton skel1;
    Skeleton skel2;
    // }

    /** nb of frames for each cycle of an animation
	multiplied by 2 for a rotation or a transition)
    */
    int framesPerCycle;

    /// number of points to calculate to find the smallest bounding box of an IFS
    int pointsForFraming;

    /// coefficient to dicrease the number of pointsForFraming for the frames of an animation
    float animationFraming;

    /// number of points before a redraw
    int pointsPerFrame;

    /** number of milli-second before updating a frame
	(or changing to an other frame for an animation)
    */
    int intervalFrame;

    /// true if the we stop the calculation of a frame after #intervalFrame# milli-second
    bool clockNumber;

    /// true if we use ImageDensity and not ImagePseudoDensity
    bool trueDensity;

    /// true if we use colored ImageDensity
    bool colored;

    void resetImage( int w, int h, int wd = -1, int wh = -1, int s = -1 );

    // { size of the image or animation to save
    int imageSavedWidth;
    int imageSavedHeight;
    int animationSavedWidth;
    int animationSavedHeight;
    //}

    bool isColored() const { return colored; }

protected:
    /** images where is stored the animation. They are here to avoid
	a memory leak in case of interruption of an animation
    */
    std::vector< Image* > images;

    /// large view
    Image* imageLarge;

    /// return a new pointer of ImageDensity or ImagePseudoDensity
    Image* buildImage( int w, int h, int wd = -1, int wh = -1, int s = -1 ) const;

    mutable float _x;
    mutable float _y;
    mutable float _color;

    /** number of points to build before checking events
	set to 1000 by default. divided by 5 for preview mode.
	Can be changed by the user only by modifying the file of parameters
     */
    int minimalBuiltPoints;

    /// called only by drawPoints(...) and Glito::drawPreview
    void iterBuildPoints( const Skeleton& skelet, const Zoom& zoom, Image& image, const int imax ) const;

    /// build #imax# points and draw #image#
    void drawPoints( const Skeleton& skelet, const Zoom& zoom, Image& image, const int imax );

    /** build points during #intervalFrame# and draw #image#
	@return the number of built points. used by calibrate()
    */
    int drawPoints( const Skeleton& skelet, const Zoom& zoom, Image& image, unsigned long& clock0 );

private:
    const MinMax findFrameRotation( int nbit ) const;

    const MinMax findFrameTransition( int nbit ) const;

};

#endif // ENGINE_HPP
