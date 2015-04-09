wifiClient = new WifiClient();
wifiClient.connect("AccessPointName", "WPA2key", function(status) {
	if (status == "Connected") {
		console.log("Connected");
		runSockets();
	} else if (status == "AP not found") {
		console.log("Couldn't find AP, searching again..");
		wifiClient.reconnect();
	} else if (status == "Wrong password") {
		console.log("Couldn't connect to Wifi router, Wrong password.");
		wifiClient.disconnect();
	} else {
		wifiClient.reconnect();
	}
});

wifiClient.scan(function(networks){
	console.log("Found these networks " + networks);
});

wifiClient.request("http://www.google.com", function(response) {
	console.log("Got response: " + response);
});

function runSockets() {
	var socket = new WebSocket;
	var io = socket.connect("echo.websocket.org", 80);
	io.on('connection', function(socket) {
		socket.on('message', function(data) {
			console.log("Got message: " + data);
		});

		socket.emit('message', {
			message : "My Message"
		});
	});
}
