#include "SoyAlembic.h"
#include <Alembic/AbcCoreFactory/All.h>
#include <SoyDebug.h>
#include <SoyJson.h>
#include <HeapArray.hpp>
#include <Alembic/abc/ICompoundProperty.h>




void MakeTriangles(ArrayBridge<Geo::TTriangle>&& Triangles,ArrayBridge<int>&& FaceIndexes)
{
	if ( FaceIndexes.GetSize() == 3 )
	{
		Geo::TTriangle Triangle( FaceIndexes[0], FaceIndexes[1], FaceIndexes[2] );
		Triangles.PushBack( Triangle );
		return;
	}

	if ( FaceIndexes.GetSize() == 4 )
	{
		Geo::TTriangle Triangle( FaceIndexes[0], FaceIndexes[1], FaceIndexes[2] );
		Triangles.PushBack( Triangle );
		Geo::TTriangle Triangleb( FaceIndexes[0], FaceIndexes[3], FaceIndexes[2] );
		Triangles.PushBack( Triangle );
		return;
	}

	std::Debug << "Unhandled face size: " << FaceIndexes.GetSize() << std::endl;
}


vec3f Convert(const Alembic::Abc::V3d& v)
{
	return vec3f( v.x, v.y, v.z );
}

void Alembic::ParseMesh(Geo::TNode& Node,AbcGeom::IPolyMesh& Mesh)
{
	using namespace AbcGeom;

	chrono_t SampleTime(0);
	auto& Schema = Mesh.getSchema();

	ISampleSelector SampleSelector( SampleTime, ISampleSelector::kNearIndex );
	IPolyMeshSchema::Sample Sample;

	if ( Schema.isConstant() )
	{
		Schema.get( Sample );
	}
	else if ( Mesh.getSchema().getNumSamples() > 0 )
	{
		Schema.get( Sample, SampleSelector );
	}
	/*
	// Get the stuff.
	P3fArraySamplePtr P = Sample.getPositions();
	Int32ArraySamplePtr indices = Sample.getFaceIndices();
	Int32ArraySamplePtr counts = Sample.getFaceCounts();

	Box3d bounds;
	bounds.makeEmpty();

    if ( m_boundsProp && m_boundsProp.getNumSamples() > 0 )
    {
        bounds = m_boundsProp.getValue( ss );
    }
	*/

	auto& pPositions = Sample ? Sample.getPositions() : nullptr;
	if ( pPositions )
	{
		auto Positions = GetRemoteArray( pPositions->get(), pPositions->size() );
		auto CopyVertex = [&Node](V3f& v)	
		{	
			Node.mVertexPositions.PushBack( Convert(v) );	
			return true;	
		};
		GetArrayBridge( Positions ).ForEach( CopyVertex );
	}

	//	gr: i think faces are lists of faces per mesh?
	auto& pFaces = Sample ? Sample.getFaceCounts() : nullptr;
	auto& pIndexes = Sample ? Sample.getFaceIndices() : nullptr;
	if ( pFaces && pIndexes )
	{
		auto Faces = GetRemoteArray( pFaces->get(), pFaces->size() );
		auto AllIndexes = GetRemoteArray( pIndexes->get(), pIndexes->size() );

		//	iterate meshes to make ours
		size_t VertexIndex = 0;
		for ( int f=0;	f<Faces.GetSize();	f++ )
		{
			auto VertexCount = Faces[f];
			//	check bounds here
			auto FaceArray = GetRemoteArray( &AllIndexes[VertexIndex], VertexCount );

			MakeTriangles( GetArrayBridge( Node.mTriangles ), GetArrayBridge(FaceArray) );

			VertexIndex += VertexCount;
		}
	}
 
		/*
    // Make triangles.
    size_t faceIndexBegin = 0;
    size_t faceIndexEnd = 0;
    for ( size_t face = 0; face < numFaces; ++face )
    {
        faceIndexBegin = faceIndexEnd;
        size_t count = (*m_meshCounts)[face];
        faceIndexEnd = faceIndexBegin + count;

        // Check this face is valid
        if ( faceIndexEnd > numIndices ||
             faceIndexEnd < faceIndexBegin )
        {
            std::cerr << "Mesh update quitting on face: "
                      << face
                      << " because of wonky numbers"
                      << ", faceIndexBegin = " << faceIndexBegin
                      << ", faceIndexEnd = " << faceIndexEnd
                      << ", numIndices = " << numIndices
                      << ", count = " << count
                      << std::endl;

            // Just get out, make no more triangles.
            break;
        }
		
        // Checking indices are valid.
        bool goodFace = true;
        for ( size_t fidx = faceIndexBegin;
              fidx < faceIndexEnd; ++fidx )
        {
            if ( ( size_t ) ( (*m_meshIndices)[fidx] ) >= numPoints )
            {
                std::cout << "Mesh update quitting on face: "
                          << face
                          << " because of bad indices"
                          << ", indexIndex = " << fidx
                          << ", vertexIndex = " << (*m_meshIndices)[fidx]
                          << ", numPoints = " << numPoints
                          << std::endl;
                goodFace = false;
                break;
            }
        }
		
        // Make triangles to fill this face.
        if ( goodFace && count > 2 )
        {
            m_triangles.push_back(
                Tri( ( unsigned int )(*m_meshIndices)[faceIndexBegin+0],
                     ( unsigned int )(*m_meshIndices)[faceIndexBegin+1],
                     ( unsigned int )(*m_meshIndices)[faceIndexBegin+2] ) );
            for ( size_t c = 3; c < count; ++c )
            {
                m_triangles.push_back(
                    Tri( ( unsigned int )(*m_meshIndices)[faceIndexBegin+0],
                         ( unsigned int )(*m_meshIndices)[faceIndexBegin+c-1],
                         ( unsigned int )(*m_meshIndices)[faceIndexBegin+c]
                         ) );
            }
        }
		
    }
	*/

	//	generate triangles

	auto& Bounds = Sample.getSelfBounds();
	Node.mBounds.min = Convert(Bounds.min);
	Node.mBounds.max = Convert(Bounds.max);

	Node.mMeta["VertexCount"] = Soy::StreamToString( std::stringstream() << Node.mVertexPositions.GetSize() );
	Node.mMeta["TraingleCount"] = Soy::StreamToString( std::stringstream() << Node.mTriangles.GetSize() );
}



std::shared_ptr<Geo::TNode> Alembic::ParseGeo(Abc::IObject& Object)
{
	using namespace AbcGeom;

	auto pNode = std::make_shared<Geo::TNode>();
	auto& Node = *pNode;


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
				ParseMesh( Node, pmesh );
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



