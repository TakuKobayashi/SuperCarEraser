using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using Newtonsoft.Json;

public class CarController : MonoBehaviour {
	[SerializeField] GameObject redCar;
	[SerializeField] GameObject blueCar;
	[SerializeField] MainUI mainUi;

	void Start () {
		WebSocketManager.Instance.Connect ("ws://tk2-254-36888.vs.sakura.ne.jp:4000/");
		WebSocketManager.Instance.OnReceiveMessage += OnReceiveMessage;
	}

	IEnumerator StayScroll()
	{
		yield return new WaitForSeconds(3.0f);
        mainUi.AnnounceRedTurn();
		yield return new WaitForSeconds(10.0f);
        mainUi.AnnounceBlueTurn();
	}

	public void OnReceiveMessage(string message){
        Dictionary<string, string> receiveMessage = JsonConvert.DeserializeObject<Dictionary<string, string>>(message);
        // 初回のHP
        if(receiveMessage["type"] == "0"){
		// ターン変更
		}else if(receiveMessage["type"] == "1"){
		// ダメージを与えた
		}else if(receiveMessage["type"] == "2"){
            
        }
		Debug.Log(message);
		mainUi.ChangeBlueSlider(message);
		mainUi.ChangeRedSlider(message);
	}

	void OnDestroy(){
		WebSocketManager.Instance.OnReceiveMessage -= OnReceiveMessage;
	}
}
