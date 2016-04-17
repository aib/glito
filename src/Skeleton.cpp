// glito/Skeleton.cpp  v1.1  2004.09.05
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

#ifdef DEBUG
# include <iostream>
#endif

#include <cmath>
//sqrt, pow, ceil, fabs, floor, cos, sin...

#ifndef M_PI
# define M_PI		3.14159265358979323846	/* pi */
#endif
#ifndef M_PI_2
# define M_PI_2		1.57079632679489661923	/* pi/2 */
#endif
#ifndef M_SQRT2
# define M_SQRT2	1.41421356237309504880	/* sqrt(2) */
#endif

#include "FL/Enumerations.H"
// FL_LEFT_MOUSE

#include "IndentedString.hpp"

#include "Skeleton.hpp"

#include "Image.hpp"

MinMax::MinMax() : built(false), xmin(100000), xmax(-100000), ymin(100000), ymax(-100000) {
}

void
MinMax::candidates( float x, float y ) {
    assert( !built );
    if ( x > xmax ) xmax = x;
    if ( x < xmin ) xmin = x;
    if ( y > ymax ) ymax = y;
    if ( y < ymin ) ymin = y;
}

void
MinMax::candidate( const MinMax& other ) {
    assert( other.built && !this->built );
    candidates( other.xmin, other.ymin );
    candidates( other.xmax, other.ymax );
}

void
MinMax::build() {
    built = true;
    width = xmax - xmin;
    height = ymax - ymin;
    centerX_ = (xmin+xmax)/2;
    centerY_ = (ymin+ymax)/2;
}

bool
MinMax::hasInfinity() const {
    return isinf(xmin) || isinf(xmax) || isinf(ymin) || isinf(ymax);
}

Skeleton::Skeleton( const string& name ) : selectedFunction(1) {
    if  ( name == "triangle" ) {
	f[0] = Function( 1, 0, 0, 1, 0, 0 );
	f[1] = Function(  0.5, 0.5,  0.5, -0.5, -0.25, 0 );
	f[2] = Function( -0.5, 0.5, -0.5, -0.5,  0.25, 0 );
	nb = 2;
    } else { //dragon
	f[0] = Function( 1, 0, 0, 1, 0, 0 );
	f[1] = Function(  0.5, 0.5, -0.5,  0.5, -0.25, 0 );
	f[2] = Function( -0.5, 0.5, -0.5, -0.5,  0.25, 0 );
	nb = 2;
    }
    probabilities();
}

// return -1 if not a square, return a >= 0 if a*a=n
int isSquare( int n ) {
    const float temp = sqrt( (float)n );
    if ( fabs( ceil(temp) * ceil (temp) - n ) < 0.00001 ) {
	return (int)ceil(temp);
    } else if ( fabs( floor(temp) * floor(temp) - n ) < 0.00001 ) {
	return (int)floor(temp);
    } else {
	return -1;
    }
}

Skeleton::Skeleton( int nbFunction ) : nb(nbFunction), selectedFunction(1) {
    assert ( nb >= 1 && nb <= NBM - 1 );
    f[0] = Function( 1, 0, 0, 1, 0, 0 );
    const float edge = sqrt((float)1/nb);
    // is nb the square of a number?
    const int squareRoot = isSquare(nb);
    for ( int i = 1; i <= nb; ++i ) {
	float xc;
	float yc;
	if ( squareRoot == -1 ) {
	    xc = 0.5 * cos( 2*M_PI * (i - 1) / nb );
	    yc = 0.5 * sin( 2*M_PI * (i - 1) / nb );
	} else {
	    xc = -0.5 + edge*( 0.5 + ((i-1)%squareRoot) );
	    yc = -0.5 + edge*( 0.5 + floor((float)(i-1)/squareRoot) );
	}
	f[i] = Function( edge, 0, 0, edge, xc, yc );
    }
    probabilities();
}

void
Skeleton::selectBiggest() {
    selectedFunction = 1;
    float areaMax=0;
    for( int n = 1; n <= nb; ++n ) {
	float area = f[n].surface();
	if ( area > areaMax ) {
	    areaMax = area;
	    selectedFunction = n;
	}
    }
}

void
Skeleton::reshape() {
    if ( selectedFunction == 0 ) {
	f[selectedFunction] = Function( 1, 0, 0, 1, 0, 0 );
    } else {
	if ( Function::system == LINEAR ) {
	    f[selectedFunction].makeSquare(0.5);
	} else if ( Function::system == SINUSOIDAL ) {
	    f[selectedFunction].makeSquare(M_PI_2);
	} else { // JULIA, FORMULA
	    f[selectedFunction].makeSquare(1);
	}
	probabilities();
    }
}

#ifdef DEBUG
void
Skeleton::print() const {
    f[0].print();
    cout << "prob\tx1\ty1\tx2\ty2\txc\tyc\n";
    for ( int n = 1; n <= nb; ++n ) {
	cout << proba[n] << ' ';
	f[n].print();
    }
}
#endif

void
Skeleton::subframe( const Skeleton& other ) {
    assert ( other.selectedFunction >= 1 ); 
    selectedFunction = other.selectedFunction;
    nb = other.nb;
    f[selectedFunction] = Function( 1, 0, 0, 1, 0, 0 );
    for ( int n = 1; n <= nb; ++n ) {
	if ( n != selectedFunction ) {
	    f[n].subframe(other.f[selectedFunction]);
	}
    }
    // should not change the proba:
    probabilities();
}

void
Skeleton::probabilities() {
    float sum=0;
    for ( int n = 1; n <= nb; ++n ) {
	sum += f[n].surface();
    }
    for ( int n = 1; n <= nb; ++n ) {
	proba[n] = f[n].surface() / sum;
    }
}

void
Skeleton::drawSkeleton( const SchemaScale& schemaScale, bool mouseRotHom ) const {
    for ( int n = 0; n <= nb; ++n ) {
	if ( n != selectedFunction ) {
	    f[n].drawParallelogram( schemaScale, false, n == 0, mouseRotHom );
	}
    }
    // selectedFunction id the last drawn to prevent it from being recovered
    f[selectedFunction].drawParallelogram( schemaScale, true,
					   selectedFunction == 0, mouseRotHom );
}

void
Skeleton::setXY( float& x, float& y, float& color, int imax ) const {
    for ( int i = 0; i < imax; ++i ) {
	nextPoint( x, y, color );
    }
}

const MinMax
Skeleton::findFrame( const int imax, float& x, float& y, float& color ) const {
    MinMax minmax;
    // reset the point
    x = 0;
    y = 0;
    // the first iterations are not used
    setXY( x, y, color, imax/25 );
    for( int i = 0; i <= imax; ++i ) {
	nextPoint(x, y, color);
	minmax.candidates( x, y );
	if ( i % 1000 == 0 && ( Function::system == SINUSOIDAL || Function::system == FORMULA ) ) {
	    if ( minmax.hasInfinity() ) {
		// if the seed of the orbit leads to infinity box, we reset the box:
		minmax = MinMax();
	    }
	    x = (float)rand()*2/RAND_MAX - 1;
	    y = (float)rand()*2/RAND_MAX - 1;
	    setXY( x, y, color );
	}
    }
    minmax.build();
    return minmax;
}

string
Skeleton::toXML( int level, const bool withSelected ) const {
    IS::ToXML in(level);
    in.add( Function::systemToXML() );
    in.elementIR( "generalFrame", f[0].toXML(level) );
    in.elementI( "nbFunction", nb );
    if ( withSelected ) {
	in.elementI( "selectedFunction", selectedFunction );
    }
    for ( int n = 1; n <= nb; ++n ) {
	in.add( f[n].toXML() );
    }
    IS::ToXML res(level);
    return res.elementIR( "skeleton", in.getValue() ).getValue();
}

bool
Skeleton::fromXML( const string& sXML ) {
    vector< string > functions = IS::ToXML::extractAll( sXML, "function" );
    if ( functions.empty() ) {
	return false;
    }
    string snb( IS::ToXML::extractFirst( sXML, "nbFunction" ) );
    Function::systemFromXML( sXML );
    assert ( functions.size() == 1 + atoi( snb.c_str() ) );
    for ( int i = 0; i < functions.size(); ++i ) {
	f[i].fromXML(functions[i]);
    }
    nb = functions.size() - 1;
    selectedFunction = 1;
    probabilities();
    return true;
}

string
Skeleton::toFractint( const string& name ) const {
    string res( name + " { ; exported from Glito\n" );
    for ( int n = 1; n <= nb; ++n ) {
	res += "  " + f[n].toFractint() + IS::translate( proba[n] ) + "\n";
    }
    res += "}\n";
    return res;
}

float
Skeleton::dimension() const {
    float r[NBM]; // dilation scales
    for ( int n = 1; n <= nb; ++n ) {
	r[n] = sqrt( f[n].surface() );
    }
    float d1 = 0;
    float d2 = 50;
    float d = 1.5;
    // equation to solve: r[1]^d + ... + r[nb]^d = 1
    while ( d2-d1 > .00001 ) {
	float s=0;
	for ( int n = 1; n <= nb; ++n ) {
	    s += pow( r[n], d );
	}
	if ( s < 1 ) {
	    d2 = d;
	    d = (d1+d)/2;
	} else {
	    d1 = d;
	    d = (d+d2)/2;
	}
    }
    return d;
}

void
Skeleton::random() {
    float vectorsRadius;
    float centerRadius;
    if ( Function::system == SINUSOIDAL ) {
	vectorsRadius = 1.8;
	centerRadius = M_PI_2;
    } else if ( Function::system == JULIA ) {
	vectorsRadius= 1.8;
	centerRadius = 1;
    } else if ( Function::system == FORMULA ) {
	vectorsRadius= 1.8;
	centerRadius = 1;
    } else { // linear
	if ( nb >= 2 ) {
	    vectorsRadius = 1 / pow( (float)M_SQRT2, sqrt((float)nb-1)-1 );
	} else {
	    vectorsRadius = 1;
	}
	centerRadius = 0.5;
    }
    for ( int n = 1; n <= nb; ++n ) {
	f[n].random( vectorsRadius, centerRadius );
    }
    probabilities();
    float x = 0;
    float y = 0;
    float color = 0;
    // Is the check for explosion necessary?
    for ( int i = 0; i <= 20000 && fabs(x)+fabs(y) < 10000; ++i ) {
	nextPoint(x, y, color);
    }
    if ( fabs(x) + fabs(y) < 10000 ) {
	return;
    } else {
	random();
    }
}

void
Skeleton::modifyForDemo() {
    float vectorsRadius;
    float centerRadius;
    if ( Function::system == SINUSOIDAL ) {
	vectorsRadius = 1.8;
	centerRadius = M_PI_2;
    } else if ( Function::system == JULIA ) {
	vectorsRadius= 1.8;
	centerRadius = 1;
    } else { // linear
	if ( nb >= 2 ) {
	    vectorsRadius = 1 / pow( (float)M_SQRT2, sqrt((float)nb-1)-1 );
	} else {
	    vectorsRadius = 1;
	}
	centerRadius = 0.5;
    }
    int n = rand()%nb + 1;
    f[n].random( vectorsRadius, centerRadius );
    probabilities();
}

void
Skeleton::randomForDemo() {
    if ( Function::system == LINEAR ) {
        // 75% of 2, 25% of 3 :
	nb = ((rand()%100)<25) + 2;
    } else { // julia and sinusoidal
	// 33% of 1, 67% of 2
	nb = ((rand()%100)<67) + 1;
    }
    if ( Function::system == LINEAR ) {
	do {
	    random();
	} while ( dimension() < 0.5 || dimension() > 2.0 );
    } else {
	random();
    }
}

void
Skeleton::nextPoint( float& x, float& y, float& color ) const {
    int n = 1;
    if ( Function::system == FORMULA ) {
        // with formulas, the parameters don't correspond to a parallelogram, so we don't consider its area 
	n = 1 + (rand() % nb);
	color = ((float)n-1.0+color)/nb;
    } else {
        float probaSum = 0;
        const float random = (float)rand()/RAND_MAX;
	while ( probaSum + proba[n] < random) {
	    probaSum += proba[n];
	    ++n;
	}
	color = probaSum + color*proba[n];
    }
    f[n].nextPoint(x,y);
}

void
Skeleton::weightedMix( const Skeleton& s1, const Skeleton& s2, float rate ) { 
    assert( s1.nb == s2.nb && nb == s1.nb );
    for ( int n = 0; n <= nb; ++n ) {
	f[n].weightedMix( s1.f[n], s2.f[n], rate );
    }
    probabilities();
}

Function
Skeleton::getFunction() const {
    return f[selectedFunction];
}

void
Skeleton::addFunction( const Function& function ) {
    ++nb;
    assert( nb < NBM );
    f[nb] = function;
    selectedFunction = nb;
    probabilities();
}

bool
Skeleton::remove() {
    if ( nb >= 2 ) { // we musk keep at least 1 function in the IFS
	if ( selectedFunction < nb ) {
	    f[selectedFunction] = f[nb];
	} else {
	    --selectedFunction;
	}
	--nb;
	probabilities();
	return true;
    } else {
	return false;
    }
}

void
Skeleton::rotate( const float alpha ) {
    f[selectedFunction].rotate( alpha );
}
void
Skeleton::mouseCandidate( float mx, float my, int button, bool mouseRotHom ) {
    f[selectedFunction].mouseCandidate( mx, my, button, mouseRotHom,
					selectedFunction != 0 && Function::system == LINEAR );
    if ( button != FL_LEFT_MOUSE ) {
	probabilities();
    }
}
