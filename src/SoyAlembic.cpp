#include "SoyAlembic.h"
#include <Alembic/AbcCoreFactory/All.h>
#include <SoyDebug.h>
#include <SoyJson.h>
#include <Alembic/abc/IObject.h>
#include <HeapArray.hpp>
#include <Alembic/abc/ICompoundProperty.h>
#include <Alembic/AbcGeom/IPolyMesh.h>

void GetMeta(std::map<std::string,std::string>& Meta,Alembic::Abc::IObject& Node)
{
	using namespace Alembic::AbcGeom;

	Meta["Name"] = Json::EscapeString( Node.getName() );

	//	gr: full name appears just to be a path
	//Meta["FullName"] = Node.getFullName();

	auto Properties = Node.getProperties();

	auto PropertyCount = Properties.getNumProperties();
	if ( PropertyCount > 0 )
	{
		TJsonWriter PropertyJson;
		for ( int i=0;	i<PropertyCount;	i++ )
		{
			auto& Property = Properties.getPropertyHeader(i);

			//	gr: to get value, need to cast and get() with a visitor pattern
			//	Abc::IScalarProperty visible( props, "visible" );
			std::stringstream PropertyValue;
			PropertyValue << Property.getDataType();
			PropertyJson.Push( Property.getName().c_str(), PropertyValue );
		}
		PropertyJson.Close();

		Meta["Properties"] = PropertyJson.mStream.str();
	}
	
	try
	{
		Meta["SchemaObject"] = Json::EscapeString( Node.getHeader().getMetaData().get("schemaObjTitle") );	
		Meta["Schema"] = Json::EscapeString( Node.getHeader().getMetaData().get("schema") );	
	
		//	am I a mesh?
	    if ( IPolyMesh::matches( Node.getHeader() ) )
	    {
			//	get as mesh
	        IPolyMesh pmesh( Node, kWrapExisting );
		    if ( pmesh )
			{
			    // dptr.reset( new IPolyMeshDrw( pmesh ) );
				Meta["PolyMesh"] = "true";
			}
		}
	}
	catch (std::exception& e)
	{
		Meta["Error"] += "Exception: ";
		Meta["Error"] += e.what();
		Meta["Error"] += "; ";
	}

		/*
		   // Get the stuff.
    P3fArraySamplePtr P = psamp.getPositions();
    Int32ArraySamplePtr indices = psamp.getFaceIndices();
    Int32ArraySamplePtr counts = psamp.getFaceCounts();

    Box3d bounds;
    bounds.makeEmpty();

    if ( m_boundsProp && m_boundsProp.getNumSamples() > 0 )
    {
        bounds = m_boundsProp.getValue( ss );
    }


		IPolyMesh pmesh( Node );
		//IPolyMesh pmesh( Node, ohead.getName() );
		if ( pmesh )
		{
			p
		}
		*/

}



std::string GetObjectJsonString(Alembic::Abc::IObject& Node)
{
	TJsonWriter Json;

	std::map<std::string,std::string> Meta;
	try
	{
		GetMeta( Meta, Node );
	}
	catch(std::exception& e)
	{
		Meta["Error"] += "Exception: ";
		Meta["Error"] += e.what();
		Meta["Error"] += "; ";
	}

	for ( auto it=Meta.begin();	it!=Meta.end();	it++ )
	{
		Json.PushJson( it->first.c_str(), it->second );
	}
	
	
	Array<std::string> ChildJsons;
	for ( int i=0;	i<Node.getNumChildren();	i++ )
	{
		auto& Child = Node.getChild(i);
		auto ChildJson = GetObjectJsonString( Child );
		ChildJsons.PushBack( ChildJson );
	}

	if ( !ChildJsons.IsEmpty() )
		Json.PushJson("Children", GetArrayBridge( ChildJsons ) );

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

