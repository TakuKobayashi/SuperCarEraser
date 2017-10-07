using System;
using System.Collections.Generic;
using UnityEngine;
using WebSocketSharp;
using Newtonsoft.Json;

public class WebSocketManager : SingletonBehaviour<WebSocketManager>{

	private WebSocket ws = null;
    public Action<MessageEventArgs> OnReceiveMessage = null;
    private List<MessageEventArgs> receiveMessageQueue = new List<MessageEventArgs>();

	void Start () {
        Connect("wss://taptappun.net/streaming");
	}
	
	void Update () {
        if(receiveMessageQueue.Count > 0){
			for (int i = 0; i < receiveMessageQueue.Count; ++i)
			{
                if(OnReceiveMessage != null){
                    OnReceiveMessage(receiveMessageQueue[i]);
                }
    		}
            receiveMessageQueue.Clear();
        }
	}

    public void Connect(string wsUrl){
		// WebSocketのechoサーバ.
		this.ws = new WebSocket(wsUrl);

		// WebSocketをOpen.
		this.ws.OnOpen += (sender, e) =>
		{
			Debug.Log("[WS] Open");
            Dictionary<string, string> param = new Dictionary<string, string>();
            param.Add("action", "connection");
			param.Add("path", "twitter_sample");
            ws.Send(JsonConvert.SerializeObject(param));
		};

		// メッセージを受信.
		this.ws.OnMessage += (sender, e) =>
		{
            receiveMessageQueue.Add(e);
            Debug.Log("[WS]Receive message: " + e.Data);
		};

		// WebSoketにErrorが発生.
		this.ws.OnError += (sender, e) =>
		{
			Debug.Log("[WS]Error: " + e.Message);
		};

		// WebSocketがClose.
		this.ws.OnClose += (sender, e) =>
		{
			Debug.Log("[WS]Close");
		};

		// WebSocketに接続.
		ws.Connect();

	}

    public void Close(){
        if(ws != null){
            this.ws.Close();
        }
    }

	void OnDestroy()
	{
        Close();
	}
}
