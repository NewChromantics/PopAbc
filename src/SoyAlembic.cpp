#include "SoyAlembic.h"
#include <Alembic/AbcCoreFactory/All.h>
#include <SoyDebug.h>
#include <SoyJson.h>
#include <HeapArray.hpp>
#include <Alembic/abc/ICompoundProperty.h>
#include <Alembic/AbcGeom/IPolyMesh.h>



std::shared_ptr<Geo::TNode> Alembic::ParseGeo(Abc::IObject& Object)
{
	auto pNode = std::make_shared<Geo::TNode>();
	auto& Node = *pNode;

	using namespace Alembic::AbcGeom;

	Node.mName = Object.getName();

	//Meta["Name"] = Json::EscapeString( Node.getName() );
	//	gr: full name appears just to be a path
	//Meta["FullName"] = Node.getFullName();

	auto Properties = Object.getProperties();

	auto PropertyCount = Properties.getNumProperties();
	for ( int i=0;	i<PropertyCount;	i++ )
	{
		auto& Property = Properties.getPropertyHeader(i);

		//	gr: to get value, need to cast and get() with a visitor pattern
		//	Abc::IScalarProperty visible( props, "visible" );
		std::stringstream PropertyValue;
		PropertyValue << Property.getDataType();

		Node.mMeta[Property.getName()] = PropertyValue.str();
	}
		
	try
	{
		Node.mMeta["SchemaObject"] = Object.getHeader().getMetaData().get("schemaObjTitle");	
		Node.mMeta["Schema"] = Object.getHeader().getMetaData().get("schema");	
	
		//	am I a mesh?
	    if ( IPolyMesh::matches( Object.getHeader() ) )
	    {
			//	get as mesh
	        IPolyMesh pmesh( Object, kWrapExisting );
		    if ( pmesh )
			{
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
	*/
			    // dptr.reset( new IPolyMeshDrw( pmesh ) );
				Node.mMeta["PolyMesh"] = Json::ValueToString(true);
			}
		}
	}
	catch (std::exception& e)
	{
		std::stringstream Error;
		Error << "Exception: " << e.what() << "; ";
		Node.mMeta["Error"] = Error.str();
	}

	//	parse children 
	for ( int i=0;	i<Object.getNumChildren();	i++ )
	{
		auto& Child = Object.getChild(i);
		auto pChildNode = ParseGeo( Child );
		Soy::Assert( pChildNode!=nullptr, "not expecting null child geo");
		Node.mChildren.PushBack( pChildNode );
	}

	return pNode;
}



Alembic::TArchive::TArchive(const TParserParams& Params) :
	TParser	( Params )
{
	AbcCoreFactory::v7::IFactory Factory;
	
	mArchiveReader = Factory.getArchive( Params.mFilename );
	Soy::Assert( mArchiveReader.valid(), "Failed to create valid ABC archive reader");

	//	start parsing
	auto Future = std::async( [this]{ParseScene();} );
	//mParsingFuture = Future;

}


Alembic::TArchive::~TArchive()
{
	//	wait for semaphore but dont throw
	try
	{
		mSceneParsing.Wait();
	}
	catch(...)
	{
		std::Debug << "Exception waiting for Scene parsing to finish" << std::endl;
	}

	//	wait for future too (this should be covered by the semaphore?)
	//mParsingFuture.Wait();
}

void Alembic::TArchive::GetMeta(std::stringstream& Stream)
{
	static bool Block = true;

	if ( !Block )
	{
		//	gotta wait for the scene parsing to finish...
		if ( !mSceneParsing.IsCompleted() )
		{
			TJsonWriter Json;
			Json.Push("Error", "Not finished parsing");
			Json.Close();
			Stream << Json.mStream.str();
			return;
		}
	}

	//	catch the parsing error if there was one
	try
	{
		mSceneParsing.Wait();
		Soy::Assert(mScene!=nullptr, "Expected scene");

		TJsonWriter Json;
		GetMeta( *mScene, Json );
		Json.Close();
		Stream << Json.mStream.str();
		return;
	}
	catch(std::exception& e)
	{
		TJsonWriter Json;
		Json.Push("Error", e.what() );
		Json.Close();
		Stream << Json.mStream.str();
		return;
	}
}

void Alembic::TArchive::ParseScene()
{
	try
	{
		auto& Root = mArchiveReader.getTop();
		mScene = ParseGeo( Root );
		mSceneParsing.OnCompleted();
	}
	catch(std::exception& e)
	{
		mSceneParsing.OnFailed( e.what() );
	}
}



void Alembic::TArchive::GetMeta(Geo::TNode& Node,TJsonWriter& Json)
{
	Json.Push("Name", Node.mName );
	Json.Push("Bounds", Node.mBounds );
	//Json.Push("Transform", Node.mTransform);

	for ( auto it=Node.mMeta.begin();	it!=Node.mMeta.end();	it++ )
	{
		auto& Key = it->first;
		auto& Value = it->second;
		Json.Push( Key.c_str(), Value );
	}

	//	write children
	Array<std::string> ChildJsons;
	for ( int i=0;	i<Node.mChildren.GetSize();	i++ )
	{
		auto& Child = *Node.mChildren[i];
		TJsonWriter ChildJson;
		GetMeta( Child, ChildJson );
		ChildJson.Close();
		ChildJsons.PushBack( ChildJson.mStream.str() );
	}

	if ( !ChildJsons.IsEmpty() )
		Json.PushJson("Children", GetArrayBridge( ChildJsons ) );
}



