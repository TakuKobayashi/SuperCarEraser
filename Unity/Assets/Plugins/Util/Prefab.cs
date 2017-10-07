using UnityEngine;
using System.Collections;

#pragma warning disable 649
[System.Serializable]
public class Prefab
{
	public GameObject gameObject { get { return _gameObject; } }
	// This field is assigned from Editor.
	[SerializeField] GameObject _gameObject;

	public void SetGameObject(GameObject gameObject)
	{
		_gameObject = gameObject;
	}

	public static implicit operator UnityEngine.Object(Prefab prefab)
	{
		return prefab.gameObject;
	}
}

