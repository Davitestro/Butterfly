const express = require('express');
const http = require('http');
const WebSocket = require('ws');
const cors = require('cors');
const fs = require('fs');
const path = require('path');
const mineflayer = require('mineflayer');

const app = express();
const server = http.createServer(app);
const wss = new WebSocket.Server({ server });

app.use(cors());
app.use(express.json());

const PORT = process.env.PORT || 3000;

// Bot storage
let bots = [];
let botInstances = {};
const BOTS_FILE = path.join(__dirname, 'bots.json');

// Load bots from file
function loadBots() {
    try {
        if (fs.existsSync(BOTS_FILE)) {
            const data = fs.readFileSync(BOTS_FILE, 'utf8');
            bots = JSON.parse(data);
            console.log(`Loaded ${bots.length} bots`);
        } else {
            // Create default bot for testing
            bots = [{
                id: '1',
                name: 'TestBot',
                server: 'localhost',
                port: 25565,
                username: 'testuser',
                password: '',
                status: 'stopped'
            }];
            saveBots();
        }
    } catch (err) {
        console.error('Error loading bots:', err);
        bots = [];
    }
}

// Save bots to file
function saveBots() {
    try {
        fs.writeFileSync(BOTS_FILE, JSON.stringify(bots, null, 2));
        console.log('Bots saved');
    } catch (err) {
        console.error('Error saving bots:', err);
    }
}

// Initialize
loadBots();

// REST API endpoints
app.get('/api/bots', (req, res) => {
    res.json(bots);
});

app.get('/api/bots/:id', (req, res) => {
    const bot = bots.find(b => b.id === req.params.id);
    if (!bot) {
        return res.status(404).json({ error: 'Bot not found' });
    }
    res.json(bot);
});

app.post('/api/bots', (req, res) => {
    const bot = {
        id: Date.now().toString(),
        name: req.body.name || 'Bot ' + (bots.length + 1),
        server: req.body.server || 'localhost',
        port: req.body.port || 25565,
        username: req.body.username || '',
        password: req.body.password || '',
        status: 'stopped'
    };
    bots.push(bot);
    saveBots();
    broadcastBots();
    res.json(bot);
});

app.put('/api/bots/:id', (req, res) => {
    const index = bots.findIndex(b => b.id === req.params.id);
    if (index === -1) {
        return res.status(404).json({ error: 'Bot not found' });
    }
    bots[index] = { ...bots[index], ...req.body };
    saveBots();
    broadcastBots();
    res.json(bots[index]);
});

app.delete('/api/bots/:id', (req, res) => {
    const index = bots.findIndex(b => b.id === req.params.id);
    if (index === -1) {
        return res.status(404).json({ error: 'Bot not found' });
    }
    
    // Stop bot if running
    if (botInstances[req.params.id]) {
        try {
            botInstances[req.params.id].end();
        } catch (err) {
            console.error('Error stopping bot:', err);
        }
        delete botInstances[req.params.id];
    }
    
    bots.splice(index, 1);
    saveBots();
    broadcastBots();
    res.json({ success: true });
});

app.post('/api/bots/:id/start', (req, res) => {
    const bot = bots.find(b => b.id === req.params.id);
    if (!bot) {
        return res.status(404).json({ error: 'Bot not found' });
    }
    
    if (botInstances[req.params.id]) {
        return res.status(400).json({ error: 'Bot already running' });
    }
    
    startBot(bot);
    res.json({ success: true });
});

app.post('/api/bots/:id/stop', (req, res) => {
    if (botInstances[req.params.id]) {
        try {
            botInstances[req.params.id].end();
        } catch (err) {
            console.error('Error stopping bot:', err);
        }
        delete botInstances[req.params.id];
        
        const bot = bots.find(b => b.id === req.params.id);
        if (bot) {
            bot.status = 'stopped';
            saveBots();
            broadcastBots();
        }
    }
    res.json({ success: true });
});

// Function to start bot
// Function to start bot
function startBot(botData) {
    console.log(`Starting bot: ${botData.name}`);
    
    const bot = mineflayer.createBot({
        host: botData.server,
        port: botData.port,
        username: botData.username,
        password: botData.password || undefined,
        auth: botData.password ? 'microsoft' : 'offline'
    });
    
    botInstances[botData.id] = bot;
    
    // Update status - FIX: use different variable name
    const botEntry = bots.find(b => b.id === botData.id);
    if (botEntry) {
        botEntry.status = 'connecting';
        saveBots();
        broadcastBots();
    }
    
    bot.on('login', () => {
        console.log(`Bot ${botData.name} logged in`);
        const b = bots.find(b => b.id === botData.id);
        if (b) {
            b.status = 'online';
            saveBots();
            broadcastBots();
        }
    });
    
    bot.on('error', (err) => {
        console.error(`Bot ${botData.name} error:`, err.message);
        const b = bots.find(b => b.id === botData.id);
        if (b) {
            b.status = 'error';
            saveBots();
            broadcastBots();
        }
    });
    
    bot.on('end', () => {
        console.log(`Bot ${botData.name} disconnected`);
        const b = bots.find(b => b.id === botData.id);
        if (b) {
            b.status = 'stopped';
            saveBots();
            broadcastBots();
        }
        delete botInstances[botData.id];
    });
}

// WebSocket handling
function broadcastBots() {
    const message = JSON.stringify({
        type: 'bots_update',
        data: bots
    });
    
    wss.clients.forEach(client => {
        if (client.readyState === WebSocket.OPEN) {
            client.send(message);
        }
    });
}

wss.on('connection', (ws) => {
    console.log('WebSocket client connected');
    
    // Send initial bot list
    ws.send(JSON.stringify({
        type: 'bots_update',
        data: bots
    }));
    
    ws.on('message', (message) => {
        try {
            const data = JSON.parse(message);
            console.log('Received:', data.type);
            
            switch(data.type) {
                case 'get_bots':
                    ws.send(JSON.stringify({
                        type: 'bots_update',
                        data: bots
                    }));
                    break;
                    
                case 'add_bot':
                    const newBot = {
                        id: Date.now().toString(),
                        name: data.name || 'Bot',
                        server: data.server || 'localhost',
                        port: data.port || 25565,
                        username: data.username || '',
                        password: data.password || '',
                        status: 'stopped'
                    };
                    bots.push(newBot);
                    saveBots();
                    broadcastBots();
                    break;
                    
                case 'update_bot':
                    const index = bots.findIndex(b => b.id === data.id);
                    if (index !== -1) {
                        bots[index] = { ...bots[index], ...data };
                        saveBots();
                        broadcastBots();
                    }
                    break;
                    
                case 'delete_bot':
                    const delIndex = bots.findIndex(b => b.id === data.id);
                    if (delIndex !== -1) {
                        if (botInstances[data.id]) {
                            botInstances[data.id].end();
                            delete botInstances[data.id];
                        }
                        bots.splice(delIndex, 1);
                        saveBots();
                        broadcastBots();
                    }
                    break;
                    
                case 'start_bot':
                    const botToStart = bots.find(b => b.id === data.id);
                    if (botToStart && !botInstances[data.id]) {
                        startBot(botToStart);
                    }
                    break;
                    
                case 'stop_bot':
                    if (botInstances[data.id]) {
                        botInstances[data.id].end();
                        delete botInstances[data.id];
                        const b = bots.find(b => b.id === data.id);
                        if (b) {
                            b.status = 'stopped';
                            saveBots();
                            broadcastBots();
                        }
                    }
                    break;
            }
        } catch (err) {
            console.error('WebSocket message error:', err);
        }
    });
    
    ws.on('close', () => {
        console.log('WebSocket client disconnected');
    });
});

// Start server
server.listen(PORT, () => {
    console.log(`Backend server running on port ${PORT}`);
});

// Cleanup on exit
process.on('SIGINT', () => {
    console.log('Shutting down...');
    Object.values(botInstances).forEach(bot => {
        try {
            bot.end();
        } catch (err) {}
    });
    process.exit();
});