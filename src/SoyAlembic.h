#pragma once


#include "TParser.h"
#include <SoyThread.h>
#include <future>
#include <Alembic/abc/IArchive.h>
#include <Alembic/abc/IObject.h>
#include <Alembic/AbcGeom/IPolyMesh.h>


class TJsonWriter;

namespace Alembic
{
	class TArchive;
	std::shared_ptr<Geo::TNode>	ParseGeo(Abc::IObject& Object);
	void						ParseMesh(Geo::TNode& Node,AbcGeom::IPolyMesh& Mesh);
}


class Alembic::TArchive : public TParser
{
public:
	TArchive(const TParserParams& Params);
	~TArchive();
	
	virtual std::shared_ptr<Geo::TNode>	GetNode(const std::string& NodeName) override;
	virtual void	GetMeta(std::stringstream& Meta) override;
	void			GetMeta(Geo::TNode& Node,TJsonWriter& Json);

	void			ParseScene();


public:
	Abc::IArchive	mArchiveReader;

	std::shared_ptr<Geo::TNode>	mScene;
	Soy::TSemaphore				mSceneParsing;
	//std::future					mParsingFuture;
};



