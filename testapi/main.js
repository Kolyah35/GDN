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

app.get('/api', (req, res) => {
	const example = fs.readFileSync(file);

	res.send(example.toString("utf8"));
})
app.post('/api', (req, res) => {
    const example = fs.readFileSync(file);

    res.send(example.toString("utf8"));
})

app.listen(port, () => {
	console.log(`app listening on port ${port}`)
})
