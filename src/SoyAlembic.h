#pragma once


#include "TParser.h"
#include <Alembic/abc/IArchive.h>

namespace Alembic
{
	class TArchive;
}


class Alembic::TArchive : public TParser
{
public:
	TArchive(const TParserParams& Params);
	
	virtual void	GetMeta(std::stringstream& Meta) override;

public:
	Abc::IArchive	mArchiveReader;
};



