#pragma once


#include "TParser.h"
#include <Alembic/abc/OArchive.h>

namespace Alembic
{
	class TArchive;
}


class Alembic::TArchive : public TParser
{
public:
	TArchive(const TParserParams& Params);
	
public:
	Abc::v7::ArchiveReaderPtr	mArchiveReader;
};



