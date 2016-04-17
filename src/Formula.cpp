// glito/Formula.cpp  v1.0  2002.03.23
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

#include <cstdlib>
#include <cmath>
#include <cctype>
// isdigit
//#include <cstdarg>
// FormulaVar

#include "Formula.hpp"
//#include "IndentedString.hpp"
// toString

//#include <iostream>

using namespace std;

Operation::Operation( list<string>::const_iterator iToken,
		      const list<string>::const_iterator& end,
		      const vector<string>& paramString ) {
    if ( iToken == end ) {
	throw string("tokens are missing");
    }
    const string token( *iToken );
    if ( token == "+" || token == "-" || token == "*" || token == "/" ||
	 token == "<" || token == "test" || token == "atan2" || token == "pow" ) {
	if ( token == "+" ) {
	    type = PLUS;
	} else if ( token == "-" ) {
	    type = MINUS;
	} else if ( token == "*" ) {
	    type = TIMES;
	} else if ( token == "/" ) {
	    type = DIVIDE;
	} else if ( token == "<" ) {
	    type = LESS;
	} else if ( token == "test" ) {
	    type = TEST;
	} else if ( token == "atan2" ) {
	    type = ATAN2;
	} else if ( token == "pow" ) {
	    type = POW;
	}
	a = new Operation( ++iToken, end, paramString );
	b = new Operation( a->notParsed, end, paramString );
	notParsed = b->notParsed;
    } else if ( token == "abs" || token == "sin" || token == "cos" || token == "tan" ||
		token == "atan" || token == "ln" || token == "sign" ||
		token == "square" || token == "sqrt" ) {
	if ( token == "abs" ) {
	    type = ABS;
	} else if ( token == "sin" ) {
	    type = SIN;
	} else if ( token == "cos" ) {
	    type = COS;
	} else if ( token == "tan" ) {
	    type = TAN;
	} else if ( token == "atan" ) {
	    type = ATAN;
	} else if ( token == "ln" ) {
	    type = LN;
	} else if ( token == "sign" ) {
	    type = SIGN;
	} else if ( token == "square" ) {
	    type = SQUARE;
	} else if ( token == "sqrt" ) {
	    type = SQRT;
	}
	a = new Operation( ++iToken, end, paramString );
	notParsed = a->notParsed;
    } else if ( token == "rand" ) {
	type = RAND;
	notParsed = ++iToken;
    } else if ( isdigit(token[0]) || token[0] == '-' && isdigit(token[1]) ) {
	type = NUMBER;
	number = atof( token.c_str() );
	notParsed = ++iToken;
    } else { // parameter
	int i = 0;
	while ( i < paramString.size() && token != paramString[i] ) {
	    ++i;
	}
	if ( i == paramString.size() ) {
	    throw string("token \"") + token + "\" is neither a parameter nor an operation";
	}
	type = PARAM;
	paramIndex = i;
	notParsed = ++iToken;
    }
}

Operation::~Operation() {
    if ( type >= PLUS && type <= SQRT ) {
	delete a;
	if ( type >= PLUS && type <= POW ) {
	    delete b;
	}
    }
}

float
Operation::apply( const vector<float>& param ) const {
    switch( type ) {
    case PARAM: return param[paramIndex];
    case NUMBER: return number;
    case RAND: return (float)rand()/RAND_MAX;
    case PLUS: return a->apply(param) + b->apply(param);
    case MINUS: return a->apply(param) - b->apply(param);
    case TIMES: {
 	float tmp = a->apply(param);
	if ( tmp == 0 ) {
	    return 0;
	} else {
	    return tmp * b->apply(param);
	}
    }
    case DIVIDE: {
 	float tmp = b->apply(param);
 	if ( tmp == 0 ) {
 	    throw exception();
 	}
	return a->apply(param) / tmp;
    }
    case LESS: return a->apply(param) < b->apply(param);
    case TEST: {
 	float tmp = a->apply(param);
	return tmp == 0 ? b->apply(param) : tmp;
    }
    case ATAN2: return atan2( a->apply(param), b->apply(param) );
    case POW: return pow( a->apply(param), b->apply(param) );
    case ABS: return fabs( a->apply(param) );
    case SIN: return sin( a->apply(param) );
    case COS: return cos( a->apply(param) );
    case TAN: return tan( a->apply(param) );
    case ATAN: return atan( a->apply(param) );
    case LN: return log( a->apply(param) );
    case SIGN: return a->apply(param) >= 0 ? 1 : -1;
    case SQUARE: {
	const float tmp = a->apply(param);
	return tmp*tmp;
    }
    case SQRT: return sqrt( a->apply(param) );
    default: abort();
    }
}

// string
// Operation::toString() const {
//     switch ( type ) {
//     case PARAM: return string( "p" ) + IS::translate( paramIndex );
//     case NUMBER: return IS::translate( number );
//     case RAND: return "rand";
//     case PLUS: return string("(") + a->toString() + " + " + b->toString() + ")";
//     case MINUS: return string("(") + a->toString() + " - " + b->toString() + ")";
//     case TIMES: return string("(") + a->toString() + "*" + b->toString() + ")";
//     case DIVIDE: return string("(") + a->toString() + "/" + b->toString() + ")";
//     case LESS: return string("(") + a->toString() + "<" + b->toString() + ")";
//     case TEST: return string("(test ") + a->toString() + " " + b->toString() + ")";
//     case ATAN2: return string("atan2(") + a->toString() + ", " + b->toString() + ")";
//     case POW: return a->toString() + "^" + b->toString();
//     case ABS: return string("|") + a->toString() + "|";
//     case SIN: return string("sin(") + a->toString() + ")";
//     case COS: return string("cos(") + a->toString() + ")";
//     case TAN: return string("tan(") + a->toString() + ")";
//     case ATAN: return string("atan(") + a->toString() + ")";
//     case LN: return string("ln(") + a->toString() + ")";
//     case SIGN: return string("sign(") + a->toString() + ")";
//     case SQUARE: return a->toString() + "^2";
//     case SQRT: return string("sign(") + a->toString() + ")";
//     default: abort();
//     }
// }

template <class Container>
void tokenize( const std::string& s, back_insert_iterator<Container> i ) {
    string::size_type begin = 0;
    string::size_type end = 0;
    while ( end != string::npos ) {
	begin = s.find_first_not_of( " ", end);
	if ( begin != string::npos ) {
	    end = s.find(" ",begin);
	    i = s.substr( begin, end-begin );
	} else {
	    break; //empty string
	}
    }
}

Formula::Formula( const std::string& sf, const std::string& sp ) {
    initialize( sf, sp );
}

Formula&
Formula::operator=( const Formula& other ) {
    delete operation;
    initialize( other.sFormula, other.sParameters );
    return *this;
}

void
Formula::initialize( const std::string& sf, const std::string& sp ) {
    sFormula = sf;
    sParameters = sp;
    list<string> tokens;
    tokenize( sFormula, back_inserter(tokens) );
    vector<string> paramString;
    tokenize( sParameters, back_inserter(paramString) );
    operation = new Operation( tokens.begin(), tokens.end(), paramString );
}

float
Formula::apply( const vector<float>& parameters ) const {
    return operation->apply(parameters);
}

// float
// FormulaVar::apply( int n, float* var, ... ) {
//     va_list ap;
//     va_start( ap, var );
//     parameters[0] = *var; 
//     for ( int i = 1; i < n; ++i ) {
// 	parameters[i] = *va_arg( ap, float* ); 
//     }
//     va_end( ap );
//     return Formula::apply( parameters );
// }

FormulaPoint&
FormulaPoint::operator=( const FormulaPoint& other ) {
    stringX = other.stringX;
    stringY = other.stringY;
    stringParameters = other.stringParameters;
    formulaX = Formula( stringX, stringParameters );
    formulaY = Formula( stringY, stringParameters );
    return *this;
}

void
FormulaPoint::apply( float& x, float& y, std::vector<float>& p ) const {
    p[0] = x;
    p[1] = y;
    float xbis = formulaX.apply( p );
    y          = formulaY.apply( p );
    x = xbis;
}

// for testing purpose:
// main() {
//     vector<string> p;
//     p.push_back("x");
//     p.push_back("y");
//     {
// 	list<string> s;
// 	s.push_front("x");
// 	s.push_front("y");
// 	s.push_front("+");
// 	Operation f( s.begin(), p );
// 	vector<float> v;
// 	v.push_back(2);
// 	v.push_back(3);
// 	cout << f.apply( v ) << endl;
//     }
//     {
// 	list<string> s;
// 	s.push_front("x");
// 	s.push_front("y");
// 	s.push_front("*");
// 	s.push_front("x");
// 	s.push_front("+");
// 	s.push_front("y");
// 	s.push_front("+");
// 	s.push_front("3.1416");
// 	s.push_front("+");
// 	s.push_front("-1");
// 	s.push_front("+");
// 	Operation f( s.begin(), p );
// 	vector<float> v;
// 	v.push_back(2);
// 	v.push_back(5);
// 	cout << f.apply( v ) << endl;
//     }
//     {
// 	list<string> aa;
// 	tokenize( "a b c d", back_inserter(aa) );
// 	tokenize( "a", back_inserter(aa) );
// 	tokenize( "", back_inserter(aa) );
// 	tokenize( " as ", back_inserter(aa) );
// 	tokenize( " asd bsd  ", back_inserter(aa) );
// 	tokenize( "asd   d ", back_inserter(aa) );
//     }
//     {
// 	Formula f( "* + y x + 3 5", "x y" );
// 	vector<float> v;
// 	v.push_back(2);
// 	v.push_back(3.00001);
// 	cout << f.apply( v ) << endl;
//     }
//     {
// 	FormulaVar f( "+ + x y z", "x y z" );
// 	vector<float> v;
// 	v.push_back(0);
// 	v.push_back(0);
// 	v.push_back(0);
// 	f.setParameters(v);
// 	float x = 1;
// 	float y = 4;
// 	cout << f.apply( 1, &x ) << endl;
// 	cout << f.apply( 2, &x, &y ) << endl;
//     }
//     {
// 	// fibonacci
// 	FormulaPoint s( "y", "+ y x", "x y" );
// 	float x = 1;
// 	float y = 1;
// 	vector<float> v(2);
// 	for ( int i = 0; i < 20; ++i ) {
// 	    cout << x << ' ';
// 	    s.apply( x, y, v );
// 	}
// 	cout << endl;
//     }
// }
