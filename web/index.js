var express = require('express');
var bodyParser = require('body-parser');

var app = express();
app.use(bodyParser.urlencoded({ extended: true }));
app.use(bodyParser.json());

var port = process.env.PORT || 4000;

//サーバーの立ち上げ
var http = require('http');

//指定したポートにきたリクエストを受け取れるようにする
var server = http.createServer(app).listen(port, function () {
	console.log('Server listening at port %d', port);
});

var WebSocketServer = require('ws').Server;
var wss = new WebSocketServer({server:server});

var connections = [];

// 初期値：作り込む時はちゃんとセットする。
var turn = 'red'; // 初回はredの攻撃
var hp = {"red":100,"blue":100};
var hpForUnity = {"type":"0","red":"100","blue":"100"};

wss.on('connection', function (ws) {
	console.log('connect!!');
	connections.push(ws);
	ws.send(JSON.stringify(hpForUnity));

	var turnInfo = {"type":"1", "next_turn":turn};
	ws.send(JSON.stringify(turnInfo));

	ws.on('close', function () {
		console.log('close');
		connections = connections.filter(function (conn, i) {
			return (conn === ws) ? false : true;
		});
	});
	ws.on('message', function (message) {
		setPostData(message);
	});
});


app.get('/jquery/jquery.js', function(req, res) {
	res.sendFile(__dirname + '/node_modules/jquery/dist/jquery.js');
});

app.get('/', function(req, res){
  console.log(req.query);
  console.log(req.body);
	res.sendFile(__dirname + '/index.html');
});

app.post('/red', function(req, res){
  console.log(req.body);
  res.sendStatus(200);

  setPostData(req.body);
});

app.post('/blue', function(req, res){
  console.log(req.body);
  res.sendStatus(200);

  setPostData(req.body);
});

function setPostData(message){
	console.log('message:', message);
	//var decodedArray = JSON.parse(message);
	var decodedArray = message;
	
	if(decodedArray['device']!=undefined && decodedArray['speed']!=undefined){
		if(decodedArray['device']==turn){
			// 攻撃側の動きなので無視
		}else{
			var attacked = decodedArray['device'];
			var speed = decodedArray['speed'];

			// ダメージ計算
			var damage = speed;

			// unityにjson送る
			var messageForUnity = {"type":"2", "attacked":attacked, "damage":damage};
			connections.forEach(function (con, i) {
				con.send(JSON.stringify(messageForUnity));
			});

			// ターン変更
			turn = decodedArray['device'];
			var messageForUnity = {"type":"1", "next_turn":turn};
			connections.forEach(function (con, i) {
				con.send(JSON.stringify(messageForUnity));
			});

			// HP減少を計算
			hp[attacked] -= damage;
			console.log('hp:', hp);
			if(hp[attacked]<0){
				// データを外部サーバにPOST
				/*
				type: "POST",
				url : 'https://testmmoos.herokuapp.com/ma_201710/save_result',
				'user_1_id' => 1,
				'user_2_id' => 2,
				'user_1_hp_first' => 100,
				'user_2_hp_first' => 100,
				'damages' => hp
				*/
				// damagesは{{"red":10},{"blue":15},,,,,,,,}的な配列

				// ゲームリセット
				turn = 'red';
				hp = {"red":100,"blue":100};
			}
		}
	}
}