var p = new Pin(2),
	v = 0;

setInterval(function() {
	console.log(v);
	p.write(v = !v);
}, 1000);
