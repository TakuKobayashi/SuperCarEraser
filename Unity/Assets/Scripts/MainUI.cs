﻿using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using Newtonsoft.Json;

public class MainUI : MonoBehaviour {
	[SerializeField] Slider redSlider;
	[SerializeField] Slider blueSlider;
	[SerializeField] Text turnText;
    [SerializeField] float reachSecond = 1.0f;

	public void AnnounceRedTurn(){
		turnText.gameObject.SetActive (true);
		turnText.color = Color.red;
		turnText.text = "Red Turn";
        StartCoroutine(ScrollTurnTextCorutine());
	}

	public void AnnounceBlueTurn(){
		turnText.gameObject.SetActive (true);
		turnText.color = Color.blue;
		turnText.text = "Blue Turn";
		StartCoroutine(ScrollTurnTextCorutine());
	}

    IEnumerator ScrollTurnTextCorutine(){
        Vector3 rightPosition = new Vector3(this.GetComponent<RectTransform>().sizeDelta.x / 2 + turnText.rectTransform.sizeDelta.x, 0, 0);
        float second = 0;
        while(second < reachSecond){
            turnText.transform.localPosition = Vector3.Lerp(rightPosition, Vector3.zero, second / reachSecond);
			Debug.Log(turnText.transform.localPosition);
            second += Time.deltaTime;
			yield return null;
        }
        yield return new WaitForSeconds(1.0f);
        Vector3 leftPosition = new Vector3(-rightPosition.x, 0 , 0);
		Debug.Log(leftPosition);
		second = 0;
		while (second < reachSecond)
		{
            turnText.transform.localPosition = Vector3.Lerp(Vector3.zero, leftPosition, second / reachSecond);
			second += Time.deltaTime;
			yield return null;
		}
        turnText.gameObject.SetActive(false);
    }

	public void ChangeRedSlider(string message){
		Debug.Log (message);
		redSlider.value = Random.Range (0, 100);
	}

	public void ChangeBlueSlider(string message){
		Debug.Log (message);
		blueSlider.value = Random.Range (0, 100);
	}
}