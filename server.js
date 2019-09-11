const express = require('express');
const main = require('./src/cpp/build/Debug/main.node');

const app = express();
const port = +process.argv.slice(2)[0].split('=')[1];

app.use(express.urlencoded({ extended: true }));

app.use(express.json({ limit: '1mb' }));

app.use((req, res, next) => {
    res.header('Access-Control-Allow-Origin', '*');
    res.header('Access-Control-Allow-Methods', 'POST, OPTIONS, GET, PUT, DELETE');
    res.header('Access-Control-Allow-Headers', 'Origin, X-Requested-With, Content-Type, Accept, Authorization');
    next();
});

app.post('/', (req, res) => {
    const { code1, code2, types } = req.body;
    try {
        res.json({
            value: main.compareCFG(code1, code2, types)
        });
    } catch (e) {
        res.json({
            error: e.message
        });
    }
});

app.listen(port, () => console.log(`Example app listening on port ${port}!`));
