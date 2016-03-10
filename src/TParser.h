#pragma once
#include <string>
#include <SoyVector.h>
#include <HeapArray.hpp>


//	very basic geometry classes.
//	might be time to move this to Soy as we already have vertex descriptions

namespace Geo
{
	class TNode;
}

class Geo::TNode
{
public:
	std::string							mName;
	float4x4							mTransform;
	Soy::Bounds3f						mBounds;
	Array<std::shared_ptr<TNode>>		mChildren;
	Array<vec3f>						mVertexPositions;
	Array<uint16>						mTriangleIndexes;
	std::map<std::string,std::string>	mMeta;			//	unhandled meta data
};



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

