var app = require('express')();
var http = require('http').createServer(app);
var io = require('socket.io')(http);
var start_num = 10001;
var max_prime = 10003;

app.get('/', function(req, res){
  res.sendFile(__dirname + '/public/index.html');
});

io.on('connection', function(socket){
  console.log('a user connected', socket.id);
  socket.emit("number", {"num":start_num, "max_prime":max_prime});
  start_num+=2;
  socket.on("result", function(data){
    console.log(data.max_prime, socket.id);
    max_prime = Math.max(max_prime, data.max_prime);
  });
  socket.emit("max_prime", {"max_prime":max_prime});
});

http.listen(3000, function(){
  console.log('listening on *:3000');
});