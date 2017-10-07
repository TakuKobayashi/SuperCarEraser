using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using Newtonsoft.Json;

public class MainUI : MonoBehaviour {
	[SerializeField] Slider redSlider;
	[SerializeField] Slider blueSlider;

	public void ChangeRedSlider(string message){
		Debug.Log (message);
		redSlider.value = Random.Range (0, 100);
	}

	public void ChangeBlueSlider(string message){
		Debug.Log (message);
		blueSlider.value = Random.Range (0, 100);
	}
}