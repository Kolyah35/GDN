const express = require('express')
const fs = require('fs')
const process = require('process')

const file = "../resources/testjson.json"

if (!fs.existsSync(file)) {
	console.error(`warn: file ${file} doesn't exist. please create this file and restart testapi application`);

	// process.exit(1);
}

const app = express()
const port = 8000

let jsonstr = {
	"Data": [
        
    ],
}
let jsonblock = {
	"ID": 5,
	"PosX": 15.0,
	"PosY": 15.0,
	"Color": {"id": 0, "R": 255, "G": 255, "B": 255, "Blending": true},
	"ScaleX": 1.1,
	"ScaleY": 1.1,
	"DetailColor": 1,
    "Rotate": 50,
}

for (let i = 0; i < 64; i++) {
    jsonblock.PosX += 15;
    jsonblock.PosY += 2;
	jsonstr.Data.push(JSON.parse(JSON.stringify(jsonblock)));
}

const jsonstr_str = JSON.stringify(jsonstr);
console.log(`generated response: ${jsonstr_str}`);

app.get('/api', (req, res) => {
	const example = fs.readFileSync(file);

    console.log(`GET /api: sent ${jsonstr_str}`)
	res.send(jsonstr_str);
})
app.post('/api', async (req, res) => {
    const example = fs.readFileSync(file);

    // await new Promise(resolve => setTimeout(resolve, 2500));

    console.log(`POST /api: sent ${jsonstr_str}`)
    res.send(jsonstr_str);
})

app.listen(port, () => {
	console.log(`app listening on port ${port}`)
})
