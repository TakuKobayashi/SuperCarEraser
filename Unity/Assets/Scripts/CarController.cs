using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using Newtonsoft.Json;

public class CarController : MonoBehaviour {
	[SerializeField] GameObject redCar;
	[SerializeField] GameObject blueCar;
	[SerializeField] MainUI mainUi;

	private float redHp = 100;
	private float blueHp = 100;

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
			blueHp = float.Parse (receiveMessage ["blue"]);
			mainUi.ChangeBlueSlider(blueHp);
			redHp = float.Parse (receiveMessage ["red"]);
			mainUi.ChangeRedSlider(redHp);
		// ターン変更
		}else if(receiveMessage["type"] == "1"){
			if (receiveMessage ["next_turn"] == "red") {
				mainUi.AnnounceRedTurn();
			} else {
				mainUi.AnnounceBlueTurn();
			}
		// ダメージを与えた
		}else if(receiveMessage["type"] == "2"){
            float damage = float.Parse(receiveMessage["damage"]);
            if(receiveMessage["attacked"] == "red"){
                redHp = redHp - damage;
                StartCoroutine(mainUi.RedSliderScroll(redHp));
            }else{
				blueHp = blueHp - damage;
                StartCoroutine(mainUi.BlueSliderScroll(blueHp));
            }
			if (redHp <= 0) {
				mainUi.ShowWinner (true);
			}else if(blueHp <= 0){
				mainUi.ShowWinner (false);
			}
        }

	}

	void OnDestroy(){
		WebSocketManager.Instance.OnReceiveMessage -= OnReceiveMessage;
	}
}
