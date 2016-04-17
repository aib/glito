// glito/Function.hpp  v1.0  2002.03.23
/* Copyright (C) 1996,2002 Emmanuel Debanne
  
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

#ifndef FUNCTION_HPP
#define FUNCTION_HPP

#include "Formula.hpp"

/** orthonormal scale
*/
class SchemaScale {
public:
    /// empty constructor
    SchemaScale() {
    }
    /// set s according to Function::system
    SchemaScale( float cx, float cy, float s );

    float scaleX( float x ) const { return x*scale; }
    float scaleY( float y ) const { return -y*scale; }
    float inverseX( float x ) const { return (x-centerX)/scale; }
    float inverseY( float y ) const { return (y-centerY)/(-scale); }

    float transX( float x ) const {
	return centerX + x*scale;
    }
    float transY( float y ) const {
	return centerY - y*scale;
    }
protected:
    float centerX;
    float centerY;
    float scale;
};

enum systemType {
    LINEAR,
    SINUSOIDAL,
    JULIA,
    FORMULA
};

class Function {
public:
    /// linear, sinusoidal, formula or julia
    static systemType system;

    /// formulas for computing the image of a point
    static FormulaPoint formulaPoint;

    /// convert the kind of sytem to an XML string
    static std::string systemToXML( int level = 0 );

    static void systemFromXML( const std::string& paramXML );

    /// sets x1 to 1, y2 to 1 and others to 0  
    Function();

    /// constructor
    Function( float a, float b,  float c,  float d,  float e,  float f );
    
#ifdef DEBUG
    /// for debug purpose
    void print() const;
#endif

    /// calculates the image of x and y
    void nextPoint( float& x, float& y ) const;

    void previousPoint( float& x, float& y, bool recalculateDenom = true ) const;

    /// determinant of the matrice ((x1,y1) (x2,y2))
    float determinant() const { return x1*y2-y1*x2; }
    
    /// surface of the parallelogram
    float surface() const;

    /// sets the function to rate*f1 + (1-rate)*f2
    void weightedMix( const Function& f1, const Function& f2, float rate );

    /** set the function so that its trajectory is a spiral
	used by Glito::zoom
    */
    void spiralMix( const Function& f1, const Function& f2, float rate );

    /// return true if the affine function is contracting
    bool isContracting() const;

    void random( float vectorsRadius, float centerRadius );

    /// rotation of vectors (x1,y1) and (x2,y2)
    void rotate( const float alpha );

    void subframe( const Function& f );

    void drawParallelogram( const SchemaScale& scale, bool selected,
			    bool zoomFunction, bool mouseRotHom ) const;

    // { access to private data
//     float getx1() const { return x1; }
//     float getx2() const { return x2; }
//     float gety1() const { return y1; }
//     float gety2() const { return y2; }
//     float getxc() const { return xc; }
//     float getyc() const { return yc; }
    // }

    /// transform the parallelogram to a square of edge #edge#
    void makeSquare( float edge ) {
	x1 = edge;
	y1 = 0;
	x2 = 0;
	y2 = edge;
    }

    void printCoordinates( int x, int y ) const;

    /// converts the Function to an XML string  
    std::string toXML( int level = 0 ) const;

    /// retrieves a Function from an XML string  
    void fromXML( const std::string& sXML );

    /// converts the Function to a string for Fractint  
    std::string toFractint() const;

    /** set numX, numY and denom.
	used to quicken iterated previousPoint calculation
    */
    void calculateTemp() const {
	numX = x2*yc - y2*xc;
	numY = y1*xc - x1*yc;
	denom = 1 / determinant();
    }

    void mouseCandidate( float mx, float my, int button, bool mouseRotHom, bool normLimited );

    /** return true if the function is not (1,0,0,1,0,0)
	Used to check if the zoom function has been modified
	and must play a role in the calculation.
    */
    bool modified() const;
    
protected:
    /** modified by setFormulaParameters and nextPoint */
    mutable std::vector<float> formulaParameters;

    /** set parameters of formulaPoint
	called when x1, y1... are changed
    */
    void setFormulaParameters();

    /// mouse interaction
    // { 
    void rotationCandidate( const float mx, const float my );
    void dilationCandidate( const float mx, const float my, const bool normLimited );
    void moveCenter( const float mx, const float my );
    void vectorCandidate( float mx, float my, const bool normLimited );
    void edgeCandidate( float mx, float my, const bool normLimited );
    // }

    /// used by drawParallelogram
    void line( const SchemaScale& scale, float xa, float ya, float xb, float yb,
	       int sxa = 0, int sya = 0, int sxb = 0, int syb = 0 ) const;


    /** variables of the affine function:
	|Xn+1| = |x1 x2||Xn| + |xc|
        |Yn+1|   |y1 y2||Yn|   |yc|
     */
    // {
    float x1;
    float y1;
    float x2;
    float y2;
    float xc;
    float yc;
    // }
    
    /// used to quicken iterated previousPoint calculation
    mutable float numX;
    mutable float numY;
    mutable float denom;
    
};

#endif // FUNCTION_HPP
