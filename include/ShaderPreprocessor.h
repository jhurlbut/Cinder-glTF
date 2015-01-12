/*
 Copyright (c) 2014, Richard Eakin - All rights reserved.
 Portions have been adapted from code written by Paul Houx and Simon Gelfus

 Redistribution and use in source and binary forms, with or without modification, are permitted provided
 that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice, this list of conditions and
 the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
 the following disclaimer in the documentation and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "cinder/Exception.h"
#include "cinder/app/app.h"
#include <map>
#include <set>
#include <vector>

class ShaderPreprocessor {
  public:
	ShaderPreprocessor();

	std::string        parse( const ci::fs::path &path );

  private:
	void			init();
	std::string		parseRecursive( const ci::fs::path &path, const ci::fs::path &parentPath, std::set<ci::fs::path> &includeTree );
	ci::fs::path		findFullPath( const ci::fs::path &path, const ci::fs::path &parentPath );

	struct Source {
		std::string		mString;
		std::time_t		mTimeLastWrite;
	};

	std::map<ci::fs::path, Source>		mCachedSources;
	std::vector<ci::fs::path>			mSearchPaths;
};

class ShaderPreprocessorExc : public ci::Exception {
  public:
	ShaderPreprocessorExc( const std::string &description ) : mDescription( description )	{}
	virtual const char* what() const throw()	{ return mDescription.c_str(); }
  protected:
	std::string mDescription;
};
