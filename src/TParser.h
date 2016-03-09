#pragma once


#include <string>


class TParserParams
{
public:
	std::string		mFilename;
};


class TParser
{
public:
	TParser(const TParserParams& Params)
	{
		
	}
	virtual ~TParser()
	{
	}

	virtual void	GetMeta(std::stringstream& Meta)=0;
};

