// glito/IndentedString.hpp  v1.0  2002.03.23
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

#ifndef FORMULA_HPP
#define FORMULA_HPP

#include <list>
#include <vector>
#include <string>

/** node of a tree which defines a formula
 */
class Operation {
public:
    /** @param iToken formula RPN
     */
    Operation( std::list<std::string>::const_iterator iToken,
	       const std::list<std::string>::const_iterator& end,
	       const std::vector<std::string>& paramString );
    
    ~Operation();

    float apply( const std::vector<float>& param ) const;

//    std::string toString() const;

private:
    enum operationType {
	PARAM,
	NUMBER,
	RAND, // 0 <= r <= 1
	PLUS,
	MINUS,
	TIMES,
	DIVIDE,
	LESS, // < a b is a < b
	TEST, // test a b is a==0 ? b : a
	ATAN2,
	POW,
	ABS,
	SIN,
	COS,
	TAN,
	ATAN,
	LN,
	SIGN, // 1 or -1
	SQUARE,
	SQRT
    };

    operationType type;

    std::list<std::string>::const_iterator notParsed;

    float number;

    int paramIndex;

    Operation* a;
    Operation* b;
};

class Formula {
public:
    Formula( const std::string& sf, const std::string& sp );

    ~Formula() {
	delete operation;
    }

    Formula& operator=( const Formula& other );

    void initialize( const std::string& sf, const std::string& sp );

    float apply( const std::vector<float>& parameters ) const;

//     std::string toString() const {
// 	return operation->toString();
//     }

private:
    std::string sFormula;
    std::string sParameters;

    Operation* operation;

};

/*
class FormulaVar : public Formula {
public:
    FormulaVar( const std::string& stringFormula, const std::string& stringParameters
	) : Formula(stringFormula, stringParameters) {
    }

    void setParameters( const std::vector<float>& p ) {
	parameters = p;
    }
    
    float apply( int n, float* var, ... );

private:
    std::vector<float> parameters;

};
*/

class FormulaPoint {
public:
    FormulaPoint( const std::string& sx, const std::string& sy, const std::string& sp ) :
	stringX(sx), stringY(sy), stringParameters(sp), formulaX( sx, sp ), formulaY( sy, sp ) {
    }

    FormulaPoint& operator=( const FormulaPoint& other );
    
    void apply( float& x, float& y, std::vector<float>& p ) const;
    
    std::string getStringX() const { return stringX; }
    std::string getStringY() const { return stringY; }

private:
    std::string stringX;
    std::string stringY;
    std::string stringParameters;

    Formula formulaX;
    Formula formulaY;

};

#endif // FORMULA_HPP
