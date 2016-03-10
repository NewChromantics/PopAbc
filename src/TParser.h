#pragma once
#include <string>
#include <SoyVector.h>
#include <HeapArray.hpp>
#include <SoyUnity.h>


//	very basic geometry classes.
//	might be time to move this to Soy as we already have vertex descriptions

namespace Geo
{
	class TNode;
	typedef vec3x<Unity::sint> TTriangle;		//	int to match unity
}

class Geo::TNode
{
public:
	TNode() :
		mpVertexPositions	( new Array<vec3f> ),
		mpTriangles			( new Array<TTriangle> ),
		mTriangles			( *mpTriangles ),
		mVertexPositions	( *mpVertexPositions )
	{
	}
	
	std::shared_ptr<Array<vec3f>>&		GetVertexPositionsPtr()	{	return mpVertexPositions;	}
	std::shared_ptr<Array<TTriangle>>&	GetTrianglesPtr()		{	return mpTriangles;	}

private:
	std::shared_ptr<Array<vec3f>>		mpVertexPositions;
	std::shared_ptr<Array<TTriangle>>	mpTriangles;

public:
	std::string							mName;
	float4x4							mTransform;
	Soy::Bounds3f						mBounds;
	Array<std::shared_ptr<TNode>>		mChildren;
	Array<vec3f>&						mVertexPositions;
	Array<TTriangle>&					mTriangles;
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

	virtual void						GetMeta(std::stringstream& Meta)=0;
	virtual std::shared_ptr<Geo::TNode>	GetNode(const std::string& NodeName)=0;
};

