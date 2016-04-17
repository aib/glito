// glito/IndentedString.cpp  v1.0  2002.03.23
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

#include <iostream>
#include <sstream>
#include <fstream>

#include "IndentedString.hpp"

using namespace IS;

IndentedString::IndentedString( const string bOpen, const bool tOpen, const string aOpen, 
				const string bClose, const bool tClose, const string aClose, 
				const string bCont, const string mCont, const string eCont,
				const string tabul, int lev ) 
    : beforeOpen(bOpen), titleOpen(tOpen), afterOpen(aOpen),
      beforeClose(bClose), titleClose(tClose), afterClose(aClose),
      beginCont(bCont), middleCont(mCont), endCont(eCont),
      tabulation(tabul), newLine("\n"), level(lev), res("") {
    for( int i = 0; i < level; ++i ) {
	tabTotal.append( tabulation );
    }
}

IndentedString::IndentedString( int lev )
    : beforeOpen(""), titleOpen(true), afterOpen(": "),
      beforeClose(""), titleClose(false), afterClose(", "),
      beginCont("< "), middleCont(", "), endCont(" >"),
      tabulation("    "), newLine("\n"), level(lev), res("") {
    for( int i = 0; i < level; ++i ) {
	tabTotal.append( tabulation );
    }
}

//not inline
string
IndentedString::getValue() const {
    return res;
}

IndentedString&
IndentedString::tabMore( const int i ) {
    res.append( tabTotal );
    for( int toI = 1; toI <= i; ++toI ) {
	res.append( tabulation );
    }
    return *this;
}

string
IS::endCleanup( string s ) {
    string::size_type last = s.find_last_not_of(" \n\r\t");
    s.resize(last+1); 
    return s;
}

void
IS::saveStringToFile( const string& fileName, const string& toSave ) {
    ofstream f;
    f.open( fileName.c_str(), ios::out );
    if ( !f ) {
	cout << "!! Error in IS::saveString !!\n" 
	     << "!! File: " << fileName << " not saved.  !!" << endl;
    } else {
	f << toSave;
    }
    f.close(); 
}

void
IS::appendStringToFile( const string& fileName, const string& toSave ) {
    ofstream f;
    f.open( fileName.c_str(), ios::app );
    if ( !f ) {
	cerr << "!! Error in IS::saveStringtoFile !!\n" 
	     << "!! File: " << fileName << " not updated.  !!" << endl;
    } else {
	f << toSave;
    }
    f.close(); 
}

string
IS::readStringInFile( const string &fileName ) {
    ifstream ifile( fileName.c_str() );
    if ( !ifile ) {
	return "";
    }
    ostringstream buf;
    char ch;
    while ( buf && ifile.get(ch) ) {
	buf.put(ch);
    }
    buf.put('\0');
    return buf.str();
}

void
IndentedString::save( const string &fileName ) const {
    IS::saveStringToFile( fileName, res );
}

void
IndentedString::saveAppend( const string &fileName ) const {
    IS::appendStringToFile( fileName, res );
}

IndentedString&
IndentedString::open( const string ch ) {
    res.append( beforeOpen + ch + afterOpen );
    openedBranchs.push( ch );
    return *this;
}

IndentedString& 
IndentedString::close() {
    res.append( beforeClose );
    if ( titleClose ) {
	res.append( openedBranchs.top() );
    }
    res.append( afterClose );
    openedBranchs.pop();
    return *this;
}

IndentedString&
IndentedString::print() {
    cout << res;
    return *this;
}

string 
IS::translate( const string& s ) {
    return s;
}

string 
IS::translate( const char* s ) {
    return string(s);
}

string 
IS::translate( int val ) {
    char toto[48];
    sprintf( toto, "%d", val);
    return string(toto);
}

string 
IS::translate( unsigned int val ) {
    char toto[48];
    sprintf( toto, "%d", val);
    return string(toto);
}

string 
IS::translate( long int val ) {
    char toto[48];
    sprintf( toto, "%ld", val);
    return string(toto);
}

string 
IS::translate( long unsigned int val ) {
    char toto[48];
    sprintf( toto, "%ld", val);
    return string(toto);
}

string 
IS::translate( bool val ) {
    if ( val ) {
	return "true";
    } else {
	return "false";
    }
}

string 
IS::translate( double val ) {
    char toto[48];
    sprintf( toto, "%f", val);
    return string(toto);
}

string
IS::extractFirst( const string& str, const string& open, const string& close ) {
    string::size_type a = str.find( open );
    if ( a == string::npos ) {
	return "";
    }
    a += open.length();
    const string::size_type b = str.find( close, a );
    return str.substr( a, b - a );
}

vector< string >
IS::extractAll( const string& str, const string& open, const string& close ) {
    vector< string > res;
    string::size_type a = 0;
    while ( a != string::npos && a < str.size() ) {
	a = str.find( open, a );
	if ( a != string::npos ) {
	    a += open.length();
//	    cout << "#1 a=" <<a<< " str.size=" << str.size() << endl;
	    string::size_type b = str.find( close, a );
	    if ( b != string::npos ) {
//		cout << tag << ": <<" << str.substr( a, b - a ) << ">>" << endl;
		res.push_back( str.substr( a, b - a ) );
		a = b + close.length();
	    } else {
		a = string::npos;
	    }
//	    cout << "#2 a=" <<a<<" b=" <<b << " str.size=" << str.size() << endl;
	}
    }
    return res;
}

string
ToXML::extractFirst( const string& str, const string& tag ) {
    const string tagOpen =  '<' + tag + '>';
    const string tagClose =  "</" + tag + '>';
    return IS::extractFirst( str, tagOpen, tagClose );
}

vector< string >
ToXML::extractAll( const string& str, const string& tag ) {
    const string tagOpen =  '<' + tag + '>';
    const string tagClose =  "</" + tag + '>';
    return IS::extractAll( str, tagOpen, tagClose );
}

/*
int main() {
    std::cerr << "Test of IndentedString\n";
    {
        const string s = "[0.0 color rgbt <1.0, 0.0, 0.5, 0.0>]\n"
	  "[1.0 color rgbt <0.0, 1.0, 0.3, 0.0>]";
	const string l = extractAll( s, "[", "]" )[0];
	assert( extractFirst( l, "", " ") == "0.0" );
	assert( extractFirst( l, "<", ",") == "1.0" );
	assert( extractAll( l, " ", ",")[1] == "0.0" );
	assert( extractAll( l, " ", ",")[2] == "0.5" );
	assert( extractAll( s, "[", "]")[1] == "1.0 color rgbt <0.0, 1.0, 0.3, 0.0>" );
    }
    {
        const std::string s = "<test>foo</test>";
	assert( ToXML::extractFirst( s, "test" ) == "foo" );
    }
    std::cerr << "End Test of IndentedString\n";
}
*/
