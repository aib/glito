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

#ifndef INDENTEDSTRING_HPP
#define INDENTEDSTRING_HPP

#include <string>
#include <stack>
// for extractFirst/ALL in ToXML:
#include <vector>
// we need it if we use #cont#:
#include <functional>

using namespace std;

namespace IS {
    
    /** translate basic type to string */
    //@{
    /// string
    string translate( const string& s );
    /// char*
    string translate( const char* s );
    /// int
    string translate( int val );
    /// unsigned int
    string translate( unsigned int val );
    /// ...
    string translate( long int val );
    string translate( long unsigned int val );
    string translate( bool val );
    string translate( double val );
    //@}

    /// enleve les ' ' et '/n' finaux
    string endCleanup( string s );
	
    /// save #toSave# in #fileName#
    void saveStringToFile( const string &fileName, const string &toSave );

    /// append #toSave# to #fileName#
    void appendStringToFile( const string &fileName, const string &toSave );

    /// return the content of #fileName#
    string readStringInFile( const string &fileName );
	
    /** for fast indented presentation of data
    */
    class IndentedString {
    public:    
	/// constructor used by derived classes
	IndentedString( const string bo, const bool to, const string ao, 
			const string bc, const bool tc, const string ac, 
			const string bCont, const string mCont, const string eCont,
			const string tabul, int level = 0 );
	
	/// default constructor
	IndentedString( int level = 0 );
	
	/// destructor empty
	~IndentedString() {}

	/// val, main output
	template< typename Value > IndentedString& add( Value val ) {
	    res.append( translate( val ) );
	    return *this;
	}
	
	/**  @memo add a tabulation at the beginning of each line
	@doc /t val.line(0) /n
	     /t val.line(1) /n
	     /t val.line(2)
	*/
	template< typename Value > IndentedString& addTab( Value val ) {
	    string s = translate( val );
	    res.append( tabulation );
	    for( int i = 0; i < s.size(); ++i ) {
		res += s[i];
		if ( s[i] == '\n' ) {
		    res.append( tabulation );
		}
	    }
	    return *this;
	}
	/// /t/t/t val /n
	template< typename Value > IndentedString& addI( Value val ) {
	    return tabLevel().add( endCleanup( translate(val) ) ).endLine();
	}
	/// <titre>val</titre>
	template< typename Value > IndentedString& element( const string titre, Value val ) {
	    return open(titre).add( endCleanup( translate(val) ) ).close();
	}
	/// /t/t/t <titre>val</titre> /n
	template< typename Value > IndentedString& elementI( const string titre, Value val ) {
	    return tabLevel().element( titre, val ).endLine();
	}
	/** @doc /t/t/t <titre> /n
	    /t/t/t/t val
	    /t/t/t </titre> /n
	*/ 
	template< typename Value > IndentedString& elementIR( const string titre, Value val ) {
	    tabLevel().open(titre).endLine();
	    tabLevel().addTab( endCleanup( translate(val) ) ).endLine();
	    return tabLevel().close().endLine();
	}
	
	/// add the string "<#s#> to #res# and prepare the stack for #close# 
	IndentedString& open( const string s );
	
	/// add the string "<#s#> where s is in the stack 
	IndentedString& close();
	
	/// add `/n` to #res#
	IndentedString& endLine() { return add( newLine ); }
	
	/// tab once
	IndentedString& tab() { return add( tabulation ); }
	
	/// tab #level# times
	IndentedString& tabLevel() { return add( tabTotal ); }
	
	/// tab #level# + #i# times
	IndentedString& tabMore( const int i = 1);
	
	/// return the XML string
	string getValue() const;
	
	/** @memo build a container representation from #iter# to #end#
	    @doc Something like < f(cont[0]), f(cont[1]), f(cont[2]) > is returned.
	    @param f translate cont[i] to a string.
	*/
	template< class Cont, class Op > IndentedString& 
	    contRange( typename Cont::const_iterator iter,
		       const typename Cont::const_iterator end,
		       Op f = translate/*<typename Cont::value_type>*/ ) {
	    add( beginCont );
	    if ( iter != end ) { // not empty
		typename Cont::const_iterator iterTmp = iter;
		++iterTmp;
		while ( iterTmp != end ) {
		    add( f( *iter ) ).add( middleCont );
		    ++iter;
		    ++iterTmp;
		}
		// comma gestion:
		if ( iterTmp == end ) {
		    add( f( *iter ) );
		}
	    } // else empty
	    return add( endCont );
	}
	
	/** @memo call #contRange# on the whole container
	    @doc Examples: ToString().cont< vector<A> >( v1,  bind2nd(mem_fun_ref(&A::toString),0) )
	    .cont< vector<A*> >( v2,  bind2nd(mem_fun(&A::toString),0) )
	    .cont< vector<int> >( v3, ptr_fun(translate<int>) ).print();
	*/
	template< class Cont, class Op > IndentedString&
	    cont( const Cont& v, Op f = translate ) {
	    return contRange< Cont >( v.begin(), v.end(), f );
	}
	
	/// save our string #res# in #fileName#
	void save( const string &fileName ) const;

	/// append our string #res# to #fileName#
	void saveAppend( const string &fileName ) const;
	
	void clear();
	
	/// cout de #res#
	IndentedString& print();
	
    protected:
	
    private:
	const string beforeOpen;
	const bool titleOpen;
	const string afterOpen;
	
	const string beforeClose;
	const bool titleClose;
	const string afterClose;

	const string beginCont;
	const string middleCont;
	const string endCont;
	
	/// "    "
	const string tabulation;
	
	/// "/n"
	const string newLine;    
	
	/// level of tabulation
	int level;
	
	/// contains the XML data
	string res;
	
	/// level times the string indent
	string tabTotal;
	
	/// stack used by #open# and #close# methods
	stack< string > openedBranchs;
    };
    
    /// a class for debugging
    class ToString : public IndentedString {
    public:
	ToString( int level = 0 ) 
	    : IndentedString( "", true, " = ", "", false, ";  ",
			      "<-- ", ", ", " -->", "    ", level ) {}
    };
    
    /// return the string between open and close in #str# 
    string extractFirst( const string& str, const string& open, const string& close );

    /// return all the strings between open and close in #str# 
    vector< string > extractAll( const string& str, const string& open, const string& close );

    /// a class for XML representation
    class ToXML : public IndentedString {
    public:
	ToXML( int level = 0 )
	    : IndentedString( "<", true, ">", "</", true, ">",
			      "", "", "", "    ", level ) {}

	/// return the string between <tag> and </tag> in #str# 
	static string extractFirst( const string& str, const string& tag );

	/// return all the strings between <tag> and </tag> in #str# 
	static vector< string > extractAll( const string& str, const string& tag );
    };

}

/** @memo translate an STL #pair# to an XML string.
    @doc This method is not in ToXML (and not even in the IS namespace)
    because of a bug in gcc 2.95.2
*/
template< class First, class Second > string static
IS_pairToXML( const pair< First, Second >& p, int level = 0 ) {
    return IS::ToXML(level)
	.elementIR( "pair", IS::ToXML( level )
		    .elementI( "first", p.first )
		    .elementI( "second", p.second )
		    .getValue() )
	.getValue();
}

// hack: to solve the problemaic call: ptr_fun(IS::translate<int>)
//using IS::translate;

#endif // INDENTEDSTRING_HPP
