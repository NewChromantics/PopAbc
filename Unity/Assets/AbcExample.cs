using UnityEngine;
using System.Collections;


[ExecuteInEditMode]
public class AbcExample : MonoBehaviour {

	public UnityEngine.UI.Text	mErrorText;
	public UnityEngine.UI.Text	mFilenameText;

	PopAbc			mAbc;


	void CreateMesh(string NodeName)
	{
		Mesh NewMesh = mAbc.LoadMesh (NodeName,0);

		GameObject Child = new GameObject (NodeName);
		if (NewMesh != null) {
			MeshFilter MeshComponent = Child.AddComponent<MeshFilter>();
			MeshComponent.mesh = NewMesh;
			Child.transform.SetParent( this.transform );
		}
	}

	void OnError(string Error)
	{
		Debug.LogError( Error );

		if ( mErrorText == null )
			return;

		mErrorText.text += Error + "\n";
	}

	public void LoadAbc()
	{
		Debug.Log("Load abc("+mFilenameText.text+")");
		LoadAbc (mFilenameText.text);

	}

	public void LoadAbc(string Filename)
	{
		mAbc = new PopAbc( Filename );

		string Meta = mAbc.GetMeta();
		Debug.Log ("Meta is " + Meta);

		CreateMesh ("octopus_lowShape");
	}

	void Update()
	{
		PopAbc.FlushDebug (Debug.Log);
	}

}
