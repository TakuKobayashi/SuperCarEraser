﻿using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using Newtonsoft.Json;

public class CarController : MonoBehaviour {
	[SerializeField] BattleCar redCar;
	[SerializeField] BattleCar blueCar;
	[SerializeField] MainUI mainUi;
	[SerializeField] AudioSource exhaustAudio;

	private float redHp = 100;
	private float blueHp = 100;

	void Start () {
		WebSocketManager.Instance.Connect ("ws://tk2-254-36888.vs.sakura.ne.jp:4000/");
		//WebSocketManager.Instance.OnReceiveMessage += OnReceiveMessage;
		redCar.SetOpponentCar (blueCar);
		blueCar.SetOpponentCar (redCar);
		StartCoroutine (move());
	}

	private IEnumerator move(){
		yield return new WaitForSeconds (1.0f);
		blueCar.Attack (-100f);
		exhaustAudio.Play ();
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
			redCar.Reset ();
			blueCar.Reset ();
		// ダメージを与えた
		}else if(receiveMessage["type"] == "2"){
            float damage = float.Parse(receiveMessage["damage"]);
            if(receiveMessage["attacked"] == "red"){
				float beforeRedHp = redHp;
                redHp = redHp - damage;
                StartCoroutine(mainUi.RedSliderScroll(redHp));
				blueCar.Attack(-(100f * damage / beforeRedHp));
				exhaustAudio.Play ();
            }else{
				float beforeBlueHp = blueHp;
				blueHp = blueHp - damage;
                StartCoroutine(mainUi.BlueSliderScroll(blueHp));
				redCar.Attack((100f * damage / beforeBlueHp));
				exhaustAudio.Play ();
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
