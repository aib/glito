// glito/Skeleton.hpp  v1.1  2004.09.05
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

#ifndef SKELETON_HPP
#define SKELETON_HPP

#include <cassert>

#include "Function.hpp"

/** tool class to get the smallest bounding box of candidate points 
 */
class MinMax {
public:
    /// constructor
    MinMax();

    /// the box is enlarged to contain the point (x,y)
    void candidates( float x, float y );

    /// the box is enlarged to contain the box #minmax#
    void candidate( const MinMax& minmax );

    /// must be called before any call to access methods
    void build();

    /// access methods
    // {
    float w() const { assert(built); return width; }
    float h() const { assert(built); return height; }
    float centerX() const { assert(built); return centerX_; }
    float centerY() const { assert(built); return centerY_; }
    // }

    bool hasInfinity() const;

private:
    bool built;

    float xmin;
    float xmax;
    float ymin;
    float ymax;
    float width;
    float height;
    float centerX_;
    float centerY_;

};

/**
 * A Skeleton is a set of affine Functions plus a zoom function  
 */
class Skeleton {
public:
    /// default constructor: dragon
    Skeleton( const std::string& name = "dragon" );

    /** creates a skeleton of nb functions positionned as a circle
	or as a puzzle if nb is square
    */
    Skeleton( int nb );

    /// apply random(c) to the functions and check it does not explode
    void random();
    /// apply random(c) to only one function and check det > 0
    void modifyForDemo();
    /// call random() and check det > 0 and dimension
    void randomForDemo();

    /// return the Hausdorff dimension of this IFS
    float dimension() const;

    /** zoom to skel.selectedFunction */
    void subframe( const Skeleton& skel );

#ifdef DEBUG
    /// print in console. for debug purpose
    void print() const;
#endif

    ///  used in Main.cpp
    int selected() const { return selectedFunction; }

    /// reshape the selected parallelogramm to a square centered in 0
    void reshape();

    /// shifts the selectedFunction by k
    void shiftSelectedFunction( int k ) {
	selectedFunction = ( selectedFunction + nb + 1 + k ) % ( nb + 1 );
    }

    // should be a constructor?
    void weightedMix( const Skeleton& s1, const Skeleton& s2, float rate );

    /// maximal number of functions + 1 (for the zoom frame)
    static const int NBM = 65;

    /// number of functions in the IFS
    int size() const { return nb; }

    /// converts the Skeleton to an XML string, withSelected used to save a MNG rotation or zoom
    std::string toXML( int level = 0, const bool withSelected = false ) const;

    /** retrieves a Skeleton from an XML string
	@return true if succeed
    */
    bool fromXML( const std::string& sXML );

    /// converts a Skeleton to a string for Fractint  
    std::string toFractint( const std::string& name ) const;

    /// removes the selectedFunction of the Skeleton. Return true if success
    bool remove();

    /// returns a copy of the selectedFunction
    Function getFunction() const;

    const Function& getZoomFunction() const { return f[0]; }

    /// add a function to the Skeleton
    void addFunction( const Function& f );

    /// rotate the selectedFunction of the skeleton. v(x1, y1) v(x2, y2) are changed
    void rotate( const float alpha );

    /** calculate the image of (x, y) and its color
	The function is chosen randomly according to the proba
    */
    void nextPoint( float& x, float& y, float& color ) const;

    /// draw the parallelogramms
    void drawSkeleton( const SchemaScale& schemaScale, bool mouseRotHom ) const;

    /// mouse interaction
    void mouseCandidate( float mx, float my, int button, bool mouseRotHom );

    /// set x and y so that we are sure (x,y) belongs to the IFS of the Skeleton
    void setXY( float& x, float& y, float& color, int imax = 30 ) const;

    /// calculate #imax# points ans return the smallest bounding box
    const MinMax findFrame( const int imax, float& x, float& y, float &color ) const;

    /** used by Glito::zoom
	@return the sum of the surfaces of the functions
    */
    float sumSurfaces() const {
	float s = 0;
	for ( int i = 1; i <= nb; ++i ) {
	    s += f[i].surface();
	}
	return s;
    }

private:
    /// calculates the proba according to the durface of each Function
    void probabilities();

    /// our functions. f[0] is the zoom function.
    Function f[NBM];

    /// set selectedFunction to the one with the biggest surface
    void selectBiggest();

    /// probabilities of the functions. proba[0] is not used.
    float proba[NBM];

    /// belongs to [0, bn]
    int selectedFunction;

    /// number of functions. belongs to [0, NBM]
    int nb;
};

#endif // SKELETON_HPP
