const express     = require('express'); //express framework to have a higher level of methods
const ejs         = require('ejs');
const app         = express(); //assign app variable the express class/method
const WebSocket   = require('ws');
const http        = require('http');
const mongoose    = require('mongoose')

require('dotenv').config();

const mongo_url = process.env.MONGO_URI

app.set('view engine', 'ejs');
app.use(express.static(__dirname + '/public'));

//Connect to mongoDB database
mongoose.connect(mongo_url, { useNewUrlParser: true, useUnifiedTopology: true })

var db = mongoose.connection;
db.on('error', console.error.bind(console, 'connection error:'));
db.once('open', function() {
  console.log("connection to db on")
});

var collectionSchema = new mongoose.Schema(
{numOfPackets: String},
{timestamps: true}
)
var Collection = mongoose.model('Collection', collectionSchema)

const server = http.createServer(app);//create a server

require('dns').lookup(require('os').hostname(), function (err, add, fam) {
  console.log('addr: '+add);
})

/***************websocket setup***********/
//var expressWs = require('express-ws')(app,server);
const s = new WebSocket.Server({ server });

//when browser sends get request, send html file to browser
// viewed at http://localhost:30000
app.get('/', function(req, res) {
res.render('pages/index');
});

//*************ws  server************

//app.ws('/echo', function(ws, req) {
s.on('connection',function(ws,req){

/******* when server receives messsage from client trigger function with argument message *****/
ws.on('message',function(message){

console.log("Received: "+message);

s.clients.forEach(function(client){ //broadcast incoming message to all clients (s.clients)
if(client!=ws && client.readyState ){ //except to the same client (ws) that sent this message
client.send("");
}
});
 ws.send("From Server only to sender: "+ message); //send to client where message is from

saveData(message)

});

ws.on('close', function(){
console.log("lost one client");
});

//ws.send("new client connected");
console.log("new client connected");
});

// Save request to MongoDB database
function saveData(message){


var collectionOne = new Collection({
  numOfPackets: message
  }
)
 collectionOne.save()
}


server.listen(3001, '192.168.2.44');
