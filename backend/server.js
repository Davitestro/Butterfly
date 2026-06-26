const express = require('express');
const http = require('http');
const cors = require('cors');
const fs = require('fs');
const path = require('path');

const BotManager = require('./botManager');
const WebSocketHandler = require('./websocket');

const app = express();
const server = http.createServer(app);

app.use(cors());
app.use(express.json());

const PORT = process.env.PORT || 3000;

const botManager = new BotManager();

// Load bots from file on startup
const bots = botManager.loadBots();
console.log(`Loaded ${bots.length} bots from file`);

// Set callbacks
const wsHandler = new WebSocketHandler(botManager);
wsHandler.setup(server);

// REST API endpoints
app.get('/api/bots', (req, res) => {
    const bots = botManager.getBots();
    console.log(`API: Returning ${bots.length} bots`);
    res.json(bots);
});

app.get('/api/bots/:id', (req, res) => {
    const bot = botManager.getBot(req.params.id);
    if (!bot) {
        return res.status(404).json({ error: 'Bot not found' });
    }
    res.json(bot);
});

app.post('/api/bots', (req, res) => {
    const bot = botManager.addBot(req.body);
    res.json(bot);
});

app.put('/api/bots/:id', (req, res) => {
    const bot = botManager.updateBot(req.params.id, req.body);
    if (!bot) {
        return res.status(404).json({ error: 'Bot not found' });
    }
    res.json(bot);
});

app.delete('/api/bots/:id', (req, res) => {
    const deleted = botManager.deleteBot(req.params.id);
    if (!deleted) {
        return res.status(404).json({ error: 'Bot not found' });
    }
    res.json({ success: true });
});

app.post('/api/bots/:id/start', (req, res) => {
    const ws = {
        send: (data) => {
            console.log('API start response:', data);
        },
        readyState: 1
    };
    const started = botManager.startBot(req.params.id, ws);
    if (!started) {
        return res.status(400).json({ error: 'Failed to start bot' });
    }
    res.json({ success: true });
});

app.post('/api/bots/:id/stop', (req, res) => {
    const stopped = botManager.stopBot(req.params.id);
    if (!stopped) {
        return res.status(400).json({ error: 'Failed to stop bot' });
    }
    res.json({ success: true });
});

server.listen(PORT, () => {
    console.log(`========================================`);
    console.log(`🚀 Minecraft Bot Manager Backend`);
    console.log(`📡 Server running on port ${PORT}`);
    console.log(`📊 WebSocket: ws://localhost:${PORT}`);
    console.log(`🌐 REST API: http://localhost:${PORT}/api`);
    console.log(`========================================`);
    console.log(`🤖 Loaded ${botManager.getBots().length} bots from file`);
});

process.on('SIGINT', () => {
    console.log('\nShutting down...');
    botManager.cleanup();
    process.exit();
});

process.on('SIGTERM', () => {
    console.log('\nShutting down...');
    botManager.cleanup();
    process.exit();
});