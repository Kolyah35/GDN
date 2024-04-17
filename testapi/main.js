const express = require('express')
const fs = require('fs')
const process = require('process')

const file = "../resources/testjson.json"

if (!fs.existsSync(file)) {
	console.error(`file ${file} doesn't exist. please create this file and restart testapi application`);

	process.exit(1);
}

const app = express()
const port = 8000

/**
 * {
    "Blocks": [
        {
            "ID": 1,
            "X": 15.0,
            "Y": 15.0,
            "Z": 1,
            "L": 0,
            "color": [255, 255, 255],
            "ScaleX": 1.1,
            "ScaleY": 1.1,
            "BaseColor": 1,
            "DetailColor": 1,
            "Groups": 0
        }
    ],
    "Colors": [
        {
            "ID": 1,
            "R": 255,
            "G": 0,
            "B": 0
        }
    ]
}
 */

let jsonstr = {
	"Blocks": [
        
    ],
    "Colors": [
        {
            "ID": 1,
            "R": 255,
            "G": 0,
            "B": 0
        }
    ]
}
let jsonblock = {
	"ID": 5,
	"X": 15.0,
	"Y": 15.0,
	"Z": 1,
	"L": 0,
	"color": [255, 255, 255],
	"ScaleX": 1.1,
	"ScaleY": 1.1,
	"BaseColor": 1,
	"DetailColor": 1,
	"Groups": 0,
    "Rotation": 50,
}

for (let i = 0; i < 64; i++) {
    jsonblock.X += 15;
	jsonstr.Blocks.push(JSON.parse(JSON.stringify(jsonblock)));
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

    await new Promise(resolve => setTimeout(resolve, 2500));

    console.log(`POST /api: sent ${jsonstr_str}`)
    res.send(jsonstr_str);
})

app.listen(port, () => {
	console.log(`app listening on port ${port}`)
})
