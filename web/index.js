var express = require('express');
var app = express();

var port = process.env.PORT || 4000;

//サーバーの立ち上げ
var http = require('http');

//指定したポートにきたリクエストを受け取れるようにする
var server = http.createServer(app).listen(port, function () {
	console.log('Server listening at port %d', port);
});

var io = require('socket.io').listen(server);

var WebSocketServer = require('ws').Server;
var wss = new WebSocketServer({server:server});

var connections = [];

// 初期値：作り込む時はちゃんとセットする。
var turn = 'red'; // 初回はredの攻撃
var hp = {"red":100,"blue":100};

wss.on('connection', function (ws) {
	console.log('connect!!');
	connections.push(ws);

	ws.on('close', function () {
		console.log('close');
		connections = connections.filter(function (conn, i) {
			return (conn === ws) ? false : true;
		});
	});
	ws.on('message', function (message) {
		console.log('message:', message);

		var decodedArray = JSON.parse(message);
		if(decodedArray['device']!=undefined && decodedArray['speed']!=undefined){
			if(decodedArray['device']==turn){
				// 攻撃側の動きなので無視
			}else{
				var attacked = decodedArray['device'];
				var speed = decodedArray['speed'];

				// ダメージ計算
				var damage = speed;

				// unityにjson送る
				var messageForUnity = '{"attacked":"'+attacked+'", "damage":"'+damage+'"}';
				connections.forEach(function (con, i) {
					con.send(messageForUnity);
				});

				// ターン変更
				turn = decodedArray['device'];

				// HP減少を計算
				hp[attacked] -= damage;
				console.log('hp:', hp);
				if(hp[attacked]<0){
					// unityにゲーム終了のjson送る
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
  console.log(req.query);
  console.log(req.body);
  res.sendStatus(200)
});

app.post('/blue', function(req, res){
  console.log(req.query);
  console.log(req.body);
  res.sendStatus(200)
});

//サーバーと接続されると呼ばれる
io.on('connection', function(socket){
	console.log('a user connected');
	//接続している、人達(socket)がサーバーにメッセーッジを送った時にcallbackされるイベントを登録
	//第一引数はイベント名
	socket.on('message', function(msg){
		//受け取った人以外でつながっている人全員に送る場合(broadcastを使う)
		//socket.broadcast.emit('message', 'hello');
		//受け取った人含めて全員に送る場合
		//位第一引数のイベント名に対して送る
		//socket.broadcast.emit('message', msg);
		io.emit('message', msg);
		console.log('message: ' + msg);
	});

	//サーバーとの接続が遮断されると呼ばれる
	socket.on('disconnect', function(){
		console.log('user disconnected');
	});
});