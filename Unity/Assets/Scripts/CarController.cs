using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class CarController : MonoBehaviour {
	[SerializeField] GameObject redCar;
	[SerializeField] GameObject blueCar;
	[SerializeField] MainUI mainUi;

	void Start () {
		WebSocketManager.Instance.Connect ("wss://taptappun.net/streaming");
		WebSocketManager.Instance.OnReceiveMessage += OnReceiveMessage;
        StartCoroutine(StayScroll());
	}

	IEnumerator StayScroll()
	{
		yield return new WaitForSeconds(3.0f);
        mainUi.AnnounceRedTurn();
		yield return new WaitForSeconds(10.0f);
        mainUi.AnnounceBlueTurn();
	}

	public void OnReceiveMessage(string message){
		Debug.Log(message);
		mainUi.ChangeBlueSlider(message);
		mainUi.ChangeRedSlider(message);
	}

	void OnDestroy(){
		WebSocketManager.Instance.OnReceiveMessage -= OnReceiveMessage;
	}
}
