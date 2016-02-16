#include "SoyAlembic.h"
#include <Alembic/AbcCoreFactory/All.h>
#include <SoyDebug.h>



Alembic::TArchive::TArchive(const TParserParams& Params) :
	TParser	( Params )
{
	AbcCoreFactory::v7::IFactory Factory;
	
	mArchiveReader = Factory.getArchive( Params.mFilename );
	Soy::Assert( mArchiveReader.valid(), "Failed to create valid ABC archive reader");
}


