// glito/Function.cpp  v1.0  2002.03.23
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

#ifdef DEBUG
# include <iostream>
#endif

#include <cmath>
//sqrt, pow, fabs, cos, sin...
#include <complex>

#ifndef M_PI
# define M_PI		3.14159265358979323846	/* pi */
#endif

#include "FL/Enumerations.H"
// FL_LEFT_MOUSE...
#include "FL/fl_draw.H"
// fl_color, fl_line

#include "IndentedString.hpp"
#include "Function.hpp"

SchemaScale::SchemaScale( float cx, float cy, float s
    ) : centerX(cx), centerY(cy) {
    if ( Function::system == LINEAR ) {
	scale = s;
    } else if ( Function::system == SINUSOIDAL ) {
	scale = s/M_PI;
    } else { // JULIA, FORMULA
	scale = s/2;
    }
}

string
Function::systemToXML( int level ) {
    string s;
    if ( system == SINUSOIDAL ) {
	s = "sinusoidal";
    } else if ( system == LINEAR ) {
	s = "linear";
    } else if ( system == JULIA ) {
	s = "juliaLinear";
    } else if ( system == FORMULA ) {
	IS::ToXML formulasXML(level);
//	formulasXML.elementI( "parameters", "x1 y1 x2 y2 xc yc" );
	formulasXML.elementI( "nextX", formulaPoint.getStringX() );
	formulasXML.elementI( "nextY", formulaPoint.getStringY() );
	s = IS::ToXML(level).elementIR( "formula", formulasXML.getValue() ).getValue();
    }
    IS::ToXML res(level);
    if ( system == FORMULA ) { 
	res.elementIR( "systemType", s);
    } else {
	res.elementI( "systemType", s);
    }
    return res.getValue();
}

void
Function::systemFromXML( const string& paramXML ) {
    const string s = IS::ToXML::extractFirst( paramXML, "systemType" );
     if ( s == "sinusoidal" ) {
	system = SINUSOIDAL;
    } else if ( s == "juliaLinear" ) {
	system = JULIA;
    } else if ( s == "linear" ) {
	system = LINEAR;
    } else { // formula
	system = FORMULA;
	formulaPoint = FormulaPoint( IS::ToXML::extractFirst( paramXML, "nextX" ),
				     IS::ToXML::extractFirst( paramXML, "nextY" ),
				     "x y x1 y1 x2 y2 xc yc" );
    }
}

Function::Function() {
    x1 = 1;    x2 = 0;    y1 = 0;    y2 = 1;    xc = 0;    yc = 0;
    setFormulaParameters();
}

Function::Function( float a, float b, float c, float d, float e, float f
    ) : x1(a), y1(b), x2(c), y2(d), xc(e), yc(f) {
    setFormulaParameters();
}

float
Function::surface() const {
    return fabs( determinant() );
}

#ifdef DEBUG
void
Function::print() const {
    std::cout << x1 << ' ' << y1 << ' ' << x2 << ' ' << y2 << ' '
	      << xc << ' ' << yc << std::endl;
}
#endif

systemType
Function::system = LINEAR;

FormulaPoint
Function::formulaPoint = FormulaPoint( "+ * y y2 yc",
				       "+ cos * x x1 + xc sin + * x x2 * y y1",
				       "x y x1 y1 x2 y2 xc yc" );

void
Function::setFormulaParameters() {
    formulaParameters.clear();
    formulaParameters.push_back(0); // x
    formulaParameters.push_back(0); // y
    formulaParameters.push_back(x1);
    formulaParameters.push_back(y1);
    formulaParameters.push_back(x2);
    formulaParameters.push_back(y2);
    formulaParameters.push_back(xc);
    formulaParameters.push_back(yc);
}

void
sqrtComplex( float a, float b, float& x, float& y ) {
    float s = hypot( a, b );
    x = sqrt( (s + a)/2 );
    y = sqrt( (s - a)/2 );
    if ( b < 0 ) {
	x = -x;
    }
} 

void
Function::nextPoint( float& x, float& y ) const {
    if ( system == LINEAR ) { // linear
	float xbis = x1*x + x2*y + xc;
	y          = y1*x + y2*y + yc;
	x = xbis;
    } else if ( system == FORMULA ) { // formula
	try {
	    formulaPoint.apply( x, y, formulaParameters );
	}
	catch ( ... ) { // x, y not changed when a division by 0 occures
	}
    } else if ( system == SINUSOIDAL ) { // sinusoidal
	float xbis = x1*cos(x) + x2*sin(y) + xc;
 	y          = y1*sin(x) + y2*cos(y) + yc;
 	x = xbis;
    } else { // julia Zn = sqrt( Zn+1 - c) replaced by Zn = sqrt( mat*Zn+1 + c^2 )
	float xcs = -(xc*xc-yc*yc);
	float ycs = -2*xc*yc;
 	const float a = x1*x + x2*y - xcs;
 	const float b = y1*x + y2*y - ycs;
	sqrtComplex( a, b, x, y );
	if ( rand()%2 ) { // one of the two roots
	    x = -x;
	    y = -y;
	}
    }
}

void
Function::previousPoint( float& x, float& y, bool recalculateTemp ) const {
    if ( recalculateTemp ) {
	calculateTemp();
    }
    float xbis = ( y2*x - x2*y + numX )*denom;
    y          = ( x1*y - y1*x + numY )*denom;
    x = xbis;
}

void
Function::weightedMix( const Function& f1, const Function& f2, float rate ) {
    x1 = rate*f1.x1 + (1-rate)*f2.x1;
    y1 = rate*f1.y1 + (1-rate)*f2.y1;
    x2 = rate*f1.x2 + (1-rate)*f2.x2;
    y2 = rate*f1.y2 + (1-rate)*f2.y2;
    xc = rate*f1.xc + (1-rate)*f2.xc;
    yc = rate*f1.yc + (1-rate)*f2.yc;
    setFormulaParameters();
}

complex<float>
mix( const complex<float> z1, const complex<float> z2, float rate ) {
    const float theta1 = arg(z1);
    const float theta2 = arg(z2);
    float theta = theta1 - theta2;
    if ( theta > M_PI ) {
	theta -= 2*M_PI;
    }
    if ( theta <= -M_PI ) {
	theta += 2*M_PI;
    }
    return polar( (float)( pow( abs(z2), (float)(1-rate) ) * pow( abs(z1), rate ) ),
 		  theta * rate + theta2 );
}
void
Function::spiralMix( const Function& f1, const Function& f2, float rate ) {
    float xSpir = 0;
    float ySpir = 0;
    { // computing fix point of f1 = center of the spiral
	float xOld;
	float yOld;
	do {
	    xOld = xSpir;
	    yOld = ySpir;
	    f1.nextPoint( xSpir, ySpir );
	} while ( fabs(xOld-xSpir) > 10e-5*fabs(xOld) || fabs(yOld-ySpir) > 10e-5*fabs(yOld) );
    }
    const complex<float> spir( xSpir, ySpir );
    complex<float> c1;
    complex<float> vright1;
    complex<float> vleft1;
    if ( f1.determinant() >= 0 ) {
	c1 = complex<float>( f1.xc, f1.yc );
	vright1 = complex<float>( f1.x1, f1.y1 );
	vleft1 = complex<float>( f1.x2, f1.y2 );
    } else { // we zoom inside the image of f1 by f1 whose determinant is positive
	float a = f1.xc;
	float b = f1.yc;
	f1.nextPoint(a,b);
	c1 = complex<float>( a, b );
	a = f1.x1;
	b = f1.y1;
	f1.nextPoint(a,b);
	vright1 = complex<float>( a-f1.xc, b-f1.yc );
	a = f1.x2;
	b = f1.y2;
	f1.nextPoint(a,b);
	vleft1 = complex<float>( a-f1.xc, b-f1.yc );
    }
    const complex<float> b1( c1 - (vright1+vleft1)/(float)2 );
    const complex<float> right1( b1 + vright1 );
    const complex<float> left1( b1 + vleft1 );

    const complex<float> c2( f2.xc, f2.yc );
    const complex<float> vright2( f2.x1, f2.y1 );
    const complex<float> vleft2( f2.x2, f2.y2 );
    const complex<float> b2( c2 - (vright2+vleft2)/(float)2 );
    const complex<float> right2( b2 + vright2 );
    const complex<float> left2( b2 + vleft2 );

    const complex<float> cT = spir + mix(c1-spir,c2-spir,rate);

    xc = cT.real();
    yc = cT.imag();

    const complex<float> bT = spir + mix(b1-spir,b2-spir,rate);
    const complex<float> rightT = spir + mix(right1-spir,right2-spir,rate);
    const complex<float> leftT = spir + mix(left1-spir,left2-spir,rate);
    
    x1 = (rightT - bT).real();
    y1 = (rightT - bT).imag();
    x2 = (leftT - bT).real();
    y2 = (leftT - bT).imag();
    
    setFormulaParameters();
}

bool
Function::isContracting() const {
    return x1*x1 + y1*y1 < 1 && x2*x2 + y2*y2 < 1 &&
	x1*x1 + y1*y1 + x2*x2 + y2*y2 < 1 + determinant()*determinant();
}

void
Function::random( float c, float r ) {
    // contraction test
    do {
	do {
	    x1 = c*((float)rand()*2/RAND_MAX-1);
	    y1 = c*((float)rand()*2/RAND_MAX-1);
	} while ( system == LINEAR && x1*x1 + y1*y1 > 0.999 );
	do {
	    x2 = c*((float)rand()*2/RAND_MAX-1);
	    y2 = c*((float)rand()*2/RAND_MAX-1);
	} while ( system == LINEAR && x2*x2 + y2*y2 > 0.999 );
    } while ( system == LINEAR && !isContracting() );
    xc = r*((float)rand()*2/RAND_MAX-1);
    yc = r*((float)rand()*2/RAND_MAX-1);
    setFormulaParameters();
}

void
Function::rotate( const float alpha ) {
    const float cosa = cos(alpha);
    const float sina = sin(alpha);
    {
	const float oldX1 = x1;
	x1 = oldX1*cosa - y1 * sina;
	y1 = oldX1*sina + y1 * cosa;
    }
    {
	const float oldX2 = x2;
	x2 = oldX2*cosa - y2 * sina;
	y2 = oldX2*sina + y2 * cosa;
    }
    setFormulaParameters();
}

void
Function::moveCenter( const float mx, const float my ) {
    xc += mx;
    yc += my;
}

void
Function::rotationCandidate( const float mx, const float my ) {
// M-C Base-C
    const float basex = xc - (x1+x2)/2;
    const float basey = yc - (y1+y2)/2;
    const float c_base_angle = atan2( basey-yc, basex-xc );
    const float c_mouse_angle = atan2( my-yc, mx-xc );
    rotate( c_mouse_angle - c_base_angle );
}

void
Function::dilationCandidate( const float mx, const float my, const bool normLimited ) {
    const float basex = xc - (x1+x2)/2;
    const float basey = yc - (y1+y2)/2;
    const float normCM = hypot( mx-xc, my-yc);
    const float normCB = hypot( basex-xc, basey-yc );
    const float dilationCandidate = normCM / normCB;
    float dilation;
    if ( !normLimited ) {
	dilation = dilationCandidate;
    } else {
	const float norm1 = hypot( x1, y1 );
	const float norm2 = hypot( x2, y2 );
	if ( norm1 < norm2 ) {
	    if ( norm2 * dilationCandidate > 1 ) {
		dilation = 1/norm2;
	    } else {
		dilation = dilationCandidate;
	    }
	} else {
	    if ( norm1 * dilationCandidate > 1 ) {
		dilation = 1/norm1;
	    } else {
		dilation = dilationCandidate;
	    }
	}
    }
    x1 *= dilation;
    y1 *= dilation;
    x2 *= dilation;
    y2 *= dilation;
}

void
Function::vectorCandidate( float mx, float my, const bool normLimited ) {
    // c - (v1+v2)/2
    const float basex = xc - (x1+x2)/2;
    const float basey = yc - (y1+y2)/2;
    float norm2;
    if ( !normLimited ) {
	// for the function 0 (zoom), we needn't limite the norm to 1
	norm2 = 0;
    } else {
	norm2 = (mx-basex)*(mx-basex) + (my-basey)*(my-basey);
    }
    // system to solve: K-Base = lambda*(M-Base)
    // and (kx-basex)*(kx-basex)+(ky-basey)*(ky-basey) = 1
    // solution: lambda = 1/sqrt(norm2) et K = lambda*M + (1-lambda)*Base
    float kx;
    float ky;
    if ( norm2 > 1 ) {
	// pour eviter les problemes d'arrondis:
	const float lambda = 1.0/sqrt(norm2)*0.999999;
	kx = lambda*mx + (1.0-lambda)*basex;
	ky = lambda*my + (1.0-lambda)*basey;
	norm2 = 1;
    } else {
        kx = mx;
	ky = my;
    }
    // closer from vector 1 than vector 2 ?
    if ( pow( mx - (basex+x1), 2 ) + pow( my - (basey+y1), 2 ) <
	 pow( mx - (basex+x2), 2 ) + pow( my - (basey+y2), 2 ) ) {
	// the norm is smaller or decreases:
	if ( norm2 <= 1 || norm2 < x1*x1 + y1*y1 ) {
	    x1 = kx-basex;
	    y1 = ky-basey;
	}
    } else {
	if ( norm2 <= 1 || norm2 < x2*x2 + y2*y2 ) {
	    x2 = kx-basex;
	    y2 = ky-basey;
	}
    }
} 

void
Function::edgeCandidate( float mx, float my, const bool normLimited ) {
    const float basex = xc - (x1+x2)/2;
    const float basey = yc - (y1+y2)/2;
    vectorCandidate( mx, my, normLimited );
    xc = basex + (x1+x2)/2;
    yc = basey + (y1+y2)/2;
} 

void
Function::mouseCandidate( float mx, float my, int button,
			  bool mouseRotHom, bool normLimited ) {
    if ( mouseRotHom ) {
	if ( button == FL_LEFT_MOUSE || button == FL_MIDDLE_MOUSE ) {
	    rotationCandidate( mx, my );
	}
	if ( button == FL_RIGHT_MOUSE || button == FL_MIDDLE_MOUSE ) {
	    dilationCandidate( mx, my, normLimited );
	}
    } else {
	if ( button == FL_LEFT_MOUSE ) {
	    moveCenter( mx, my );
	} else if ( button == FL_RIGHT_MOUSE ) {
	    vectorCandidate( mx, my, normLimited );
	} else if ( button == FL_MIDDLE_MOUSE ) {
	    edgeCandidate( mx, my, normLimited );
	}
    }
    setFormulaParameters();
}

void
Function::subframe( const Function& f ) {
    const float den = 1 / f.determinant();
    {
	const float oldX1 = x1;
	x1 = ( f.y2*oldX1 - f.x2*y1 )*den;
	y1 = ( f.y1*oldX1 - f.x1*y1 )*-den;
    }
    {
	const float oldX2 = x2;
	x2 = ( f.y2*oldX2 - f.x2*y2 )*den;
	y2 = ( f.y1*oldX2 - f.x1*y2 )*-den;
    }
    xc = ( f.y2*(xc-f.xc) - f.x2*(yc-f.yc) )*den;
    yc = ( f.y1*(xc-f.xc) - f.x1*(yc-f.yc) )*-den;
    setFormulaParameters();
}

void
Function::printCoordinates( int x, int y ) const {
    const int height = 14;
    fl_draw( ("x1 = " + IS::translate(x1)).c_str(), x, y + height*0 );
    fl_draw( ("y1 = " + IS::translate(y1)).c_str(), x, y + height*1 );
    fl_draw( ("x2 = " + IS::translate(x2)).c_str(), x, y + height*2 );
    fl_draw( ("y2 = " + IS::translate(y2)).c_str(), x, y + height*3 );
    fl_draw( ("xc = " + IS::translate(xc)).c_str(), x, y + height*4 );
    fl_draw( ("yc = " + IS::translate(yc)).c_str(), x, y + height*5 );
}

void
Function::drawParallelogram( const SchemaScale& scale, bool selected,
			     bool zoomFunction, bool mouseRotHom ) const {
    const float baseX = xc - (x1+x2)/2;
    const float baseY = yc - (y1+y2)/2;
    const float oppositeX = xc + (x1+x2)/2;
    const float oppositeY = yc + (y1+y2)/2;
    const float rightX = xc + (x1-x2)/2;
    const float rightY = yc + (y1-y2)/2;
    const float leftX = xc + (-x1+x2)/2;
    const float leftY = yc + (-y1+y2)/2;
    if ( zoomFunction ) {
	if ( selected ) {
	    fl_color(FL_WHITE);
	} else {
	    fl_color(FL_DARK3);
	}
    } else {
	if ( selected ) {
	    fl_color(FL_YELLOW);
	} else {
	    fl_color(FL_DARK3);
	}
    }
    line( scale, oppositeX, oppositeY, rightX, rightY );
    line( scale, oppositeX, oppositeY, leftX, leftY );
    if ( zoomFunction ) {
	if ( selected ) {
	    fl_color(FL_RED);
	} else {
	    fl_color(FL_DARK3);
	}
    } else {
	if ( selected ) {
	    fl_color(FL_RED);
	} else {
	    fl_color(FL_BLUE);
	}
    }
    if ( mouseRotHom && selected ) {
	line( scale, xc, yc, baseX, baseY ); 
    }
    line( scale, baseX, baseY, rightX, rightY );
    line( scale, baseX, baseY, leftX, leftY );
    // parallelogram center marked by + ou -:
    line( scale, xc, yc, xc, yc, -3, 0, 3, 0 );
    if ( determinant() >= 0 ) {
	line( scale, xc, yc, xc, yc, 0, -3, 0, 3 );
    }
}

void
Function::line( const SchemaScale& scale, float xa, float ya, float xb, float yb,
		int sxa, int sya, int sxb, int syb ) const {
    fl_line( (int)scale.transX(xa) + sxa, (int)scale.transY(ya) + sya,
	     (int)scale.transX(xb) + sxb, (int)scale.transY(yb) + syb );
}

bool
Function::modified() const {
    return (fabs(x1-1) + fabs(y1) + fabs(x2) + fabs(y2-1) + fabs(xc) + fabs(yc) > 0.0001);
}

string
Function::toXML( int level ) const {
    return IS::ToXML(level)
	.elementIR( "function", IS::ToXML(level)
		    .elementI( "x1", x1 )
		    .elementI( "y1", y1 )
		    .elementI( "x2", x2 )
		    .elementI( "y2", y2 )
		    .elementI( "xc", xc )
		    .elementI( "yc", yc )
		    .getValue() )
	.getValue();
}

void
Function::fromXML( const string& sXML ) {
    string Tx1( IS::ToXML::extractFirst( sXML, "x1" ) );
    string Ty1( IS::ToXML::extractFirst( sXML, "y1" ) );
    string Tx2( IS::ToXML::extractFirst( sXML, "x2" ) );
    string Ty2( IS::ToXML::extractFirst( sXML, "y2" ) );
    string Txc( IS::ToXML::extractFirst( sXML, "xc" ) );
    string Tyc( IS::ToXML::extractFirst( sXML, "yc" ) );
    x1 = (float)atof(Tx1.c_str());
    y1 = (float)atof(Ty1.c_str());
    x2 = (float)atof(Tx2.c_str());
    y2 = (float)atof(Ty2.c_str());
    xc = (float)atof(Txc.c_str());
    yc = (float)atof(Tyc.c_str());
    setFormulaParameters();
}

string
Function::toFractint() const {
    return IS::translate(x1) + " " + IS::translate(x2) + " " +
	IS::translate(y1) + " " + IS::translate(y2) + " " +
	IS::translate(xc) + " " + IS::translate(yc) + " ";
}
