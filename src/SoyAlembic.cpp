#include "SoyAlembic.h"
#include <Alembic/AbcCoreFactory/All.h>



Alembic::TArchive::TArchive(const TParserParams& Params) :
	TParser	( Params )
{
	AbcCoreFactory::v7::IFactory Factory;
	
	auto Archive = Factory.getArchive( Params.mFilename );
}

