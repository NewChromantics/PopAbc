#include "SoyAlembic.h"
#include <Alembic/AbcCoreFactory/All.h>
#include <SoyDebug.h>
#include <SoyJson.h>
#include <Alembic/abc/IObject.h>
#include <HeapArray.hpp>


std::string GetObjectJsonString(Alembic::Abc::IObject& Node)
{
	TJsonWriter Json;
	Json.Push("Name", Node.getName() );

	//	gr: full name appears just to be a path
	//Json.Push("FullName", Node.getFullName() );
	
	Array<std::string> ChildJsons;
	for ( int i=0;	i<Node.getNumChildren();	i++ )
	{
		auto& Child = Node.getChild(i);
		auto ChildJson = GetObjectJsonString( Child );
		ChildJsons.PushBack( ChildJson );
	}

	if ( !ChildJsons.IsEmpty() )
		Json.Push("Children", GetArrayBridge( ChildJsons ) );

	Json.Close();

	return Json.mStream.str();
}


Alembic::TArchive::TArchive(const TParserParams& Params) :
	TParser	( Params )
{
	AbcCoreFactory::v7::IFactory Factory;
	
	mArchiveReader = Factory.getArchive( Params.mFilename );
	Soy::Assert( mArchiveReader.valid(), "Failed to create valid ABC archive reader");
}

void Alembic::TArchive::GetMeta(std::stringstream& Stream)
{
	auto& Root = mArchiveReader.getTop();
	auto Json = GetObjectJsonString( Root );
	Stream << Json;
}

