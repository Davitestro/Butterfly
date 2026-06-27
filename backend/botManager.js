const mineflayer = require('mineflayer');
const { pathfinder } = require('mineflayer-pathfinder');
const fs = require('fs');
const path = require('path');

class BotManager {
    constructor() {
        this.bots = [];
        this.instances = {};
        this.botData = {};
        this.botTasks = {};
        this.broadcastCallback = null;
        this.saveCallback = null;
        this.wsClients = new Set();
        this.botsFile = path.join(__dirname, 'bots.json');
    }

    setCallbacks(broadcast, save) {
        this.broadcastCallback = broadcast;
        this.saveCallback = save;
    }

    addWsClient(ws) {
        this.wsClients.add(ws);
    }

    removeWsClient(ws) {
        this.wsClients.delete(ws);
    }

    _cleanMessage(obj) {
        if (obj === null || obj === undefined) return obj;
        if (typeof obj !== 'object') return obj;
        
        if (Array.isArray(obj)) {
            return obj.map(item => this._cleanMessage(item));
        }
        
        const cleaned = {};
        for (const [key, value] of Object.entries(obj)) {
            if (typeof value === 'function') continue;
            if (typeof value === 'object' && value !== null) {
                try {
                    const test = JSON.stringify(value);
                    if (test.length > 100000) continue;
                    cleaned[key] = this._cleanMessage(value);
                } catch (err) {
                    continue;
                }
            } else {
                cleaned[key] = value;
            }
        }
        return cleaned;
    }

    sendToAllClients(message) {
        try {
            const cleanMessage = this._cleanMessage(message);
            const data = JSON.stringify(cleanMessage);
            this.wsClients.forEach(client => {
                if (client && client.readyState === 1) {
                    try {
                        client.send(data);
                    } catch (err) {
                        console.error('Error sending to client:', err.message);
                    }
                }
            });
        } catch (err) {
            console.error('Error in sendToAllClients:', err.message);
        }
    }

    sendToClient(ws, message) {
        if (ws && ws.readyState === 1) {
            try {
                const cleanMessage = this._cleanMessage(message);
                ws.send(JSON.stringify(cleanMessage));
            } catch (err) {
                console.error('Error sending to client:', err.message);
            }
        }
    }

    loadBots() {
        try {
            if (fs.existsSync(this.botsFile)) {
                const data = fs.readFileSync(this.botsFile, 'utf8');
                const parsed = JSON.parse(data);
                this.bots = parsed;
                console.log(`✅ Loaded ${this.bots.length} bots from file`);
                return this.bots;
            } else {
                console.log('📁 No bots file found, creating new');
                this.bots = [];
                this.saveBots();
                return this.bots;
            }
        } catch (err) {
            console.error('❌ Error loading bots:', err);
            this.bots = [];
            return this.bots;
        }
    }

    saveBots() {
        try {
            fs.writeFileSync(this.botsFile, JSON.stringify(this.bots, null, 2));
            console.log(`💾 Saved ${this.bots.length} bots to file`);
            if (this.broadcastCallback) {
                this.broadcastCallback(this.bots);
            }
            return true;
        } catch (err) {
            console.error('❌ Error saving bots:', err);
            return false;
        }
    }

    getBots() {
        return this.bots;
    }

    getBot(id) {
        return this.bots.find(b => b.id === id);
    }

    getBotInstance(id) {
        return this.instances[id];
    }

    getBotData(id) {
        return this.botData[id] || {};
    }

    addBot(botData) {
        const bot = {
            id: Date.now().toString(),
            name: botData.name || 'Bot',
            server: botData.server || 'localhost',
            port: parseInt(botData.port) || 25565,
            username: botData.username || '',
            password: botData.password || '',
            status: 'stopped',
            createdAt: new Date().toISOString(),
            lastModified: new Date().toISOString()
        };
        this.bots.push(bot);
        this.saveBots();
        return bot;
    }

    updateBot(id, data) {
        const index = this.bots.findIndex(b => b.id === id);
        if (index === -1) return null;
        
        this.bots[index] = { 
            ...this.bots[index], 
            ...data,
            lastModified: new Date().toISOString()
        };
        this.saveBots();
        return this.bots[index];
    }

    deleteBot(id) {
        const index = this.bots.findIndex(b => b.id === id);
        if (index === -1) return false;
        
        if (this.instances[id]) {
            this.instances[id].end();
            delete this.instances[id];
            delete this.botData[id];
        }
        
        this.bots.splice(index, 1);
        this.saveBots();
        return true;
    }

    startBot(id, ws) {
        const botConfig = this.bots.find(b => b.id === id);
        if (!botConfig) {
            this.sendToClient(ws, {
                type: 'notification',
                title: 'Error',
                message: 'Bot not found',
                error: true
            });
            return false;
        }

        if (this.instances[id]) {
            this.sendToClient(ws, {
                type: 'notification',
                title: 'Error',
                message: 'Bot already running',
                error: true
            });
            return false;
        }

        this._createBot(botConfig, ws);
        return true;
    }

    stopBot(id) {
        if (this.instances[id]) {
            this.instances[id].end();
            return true;
        }
        return false;
    }

    _createBot(botConfig, ws) {
        const isAternos = botConfig.server && botConfig.server.includes('aternos');
        const hasPassword = botConfig.password && botConfig.password.length > 0;
        const useOffline = !hasPassword || isAternos;
        const authMode = useOffline ? 'offline' : 'microsoft';

        console.log(`[${botConfig.name}] Starting with ${authMode} mode`);
        console.log(`[${botConfig.name}] Server: ${botConfig.server}:${botConfig.port}`);

        const botOptions = {
            host: botConfig.server || 'localhost',
            port: parseInt(botConfig.port) || 25565,
            username: botConfig.username || 'Steve',
            version: false,
            auth: authMode,
            connectTimeout: 30000,
            log: true
        };

        if (!useOffline && hasPassword) {
            botOptions.password = botConfig.password;
        }

        const bot = mineflayer.createBot(botOptions);
        this.instances[botConfig.id] = bot;
        this.botData[botConfig.id] = {
            health: 20,
            food: 20,
            experience: 0,
            level: 0,
            position: { x: 0, y: 0, z: 0 },
            dimension: 'Overworld',
            gamemode: 'Survival',
            inventory: [],
            lastUpdate: Date.now()
        };

        // Initialize tasks for this bot
        if (!this.botTasks) {
            this.botTasks = {};
        }
        this.botTasks[botConfig.id] = {
            tasks: [],
            currentTask: null,
            completed: 0,
            total: 0,
            isRunning: false,
            progress: 0
        };

        const botEntry = this.bots.find(b => b.id === botConfig.id);
        if (botEntry) {
            botEntry.status = 'connecting';
            this.saveBots();
        }

        try {
            bot.loadPlugin(pathfinder);
            console.log(`[${botConfig.name}] Pathfinder loaded`);
        } catch (err) {
            console.log(`[${botConfig.name}] Pathfinder already loaded`);
        }

        bot.once('spawn', () => {
            try {
                const mcData = require('minecraft-data')(bot.version);
                const { Movements } = require('mineflayer-pathfinder');
                const defaultMove = new Movements(bot, mcData);
                bot.pathfinder.setMovements(defaultMove);
                console.log(`[${botConfig.name}] Pathfinder configured`);
            } catch (err) {
                console.log(`[${botConfig.name}] Failed to configure pathfinder:`, err.message);
            }
        });

        const botEvents = require('./botEvents');
        botEvents.setupEvents(bot, botConfig, this);

        const botCommands = require('./botCommands');
        botCommands.setupCommands(bot, botConfig, this);

        this._startStatusUpdates(botConfig.id);

        this.sendToClient(ws, {
            type: 'notification',
            title: 'Bot Starting',
            message: `${botConfig.name} is connecting to server...`,
            error: false
        });
    }

    _startStatusUpdates(botId) {
        const updateInterval = setInterval(() => {
            const bot = this.instances[botId];
            if (!bot) {
                clearInterval(updateInterval);
                return;
            }

            const data = this.botData[botId];
            if (!data) return;

            // Update dimension
            if (bot.game) {
                data.dimension = bot.game.dimension || 'Overworld';
                data.gamemode = bot.game.gamemode || 'Survival';
            }

            // Update health
            if (bot.health !== undefined) {
                data.health = Math.round(bot.health * 10) / 10;
            }
            if (bot.food !== undefined) {
                data.food = bot.food;
            }
            if (bot.experience !== undefined) {
                data.experience = Math.round(bot.experience);
            }
            if (bot.level !== undefined) {
                data.level = bot.level;
            }

            // Update position
            if (bot.entity && bot.entity.position) {
                data.position = {
                    x: Math.round(bot.entity.position.x * 10) / 10,
                    y: Math.round(bot.entity.position.y * 10) / 10,
                    z: Math.round(bot.entity.position.z * 10) / 10
                };
            }

            // Update inventory
            if (bot.inventory) {
                data.inventory = bot.inventory.items().map(item => ({
                    name: item.name,
                    count: item.count,
                    slot: item.slot,
                    displayName: item.displayName || item.name
                }));
            }

            data.lastUpdate = Date.now();

            // Send stats update
            const cleanData = {
                health: data.health || 20,
                food: data.food || 20,
                experience: data.experience || 0,
                level: data.level || 0,
                position: data.position || { x: 0, y: 0, z: 0 },
                dimension: data.dimension || 'Overworld',
                gamemode: data.gamemode || 'Survival',
                inventory: (data.inventory || []).slice(0, 36).map(item => ({
                    name: item.name || 'unknown',
                    count: item.count || 0,
                    slot: item.slot || 0,
                    displayName: item.displayName || item.name || 'unknown'
                })),
                lastUpdate: data.lastUpdate
            };

            this.sendToAllClients({
                type: 'bot_stats',
                data: cleanData
            });

            if (cleanData.inventory && cleanData.inventory.length > 0) {
                this.sendToAllClients({
                    type: 'bot_inventory',
                    data: cleanData.inventory
                });
            }

            // Send task progress
            const taskInfo = this.botTasks && this.botTasks[botId];
            if (taskInfo && taskInfo.currentTask) {
                this.sendToAllClients({
                    type: 'task_progress',
                    data: {
                        currentTask: taskInfo.currentTask,
                        progress: taskInfo.progress || 0,
                        total: taskInfo.total || 0,
                        completed: taskInfo.completed || 0
                    }
                });
            }

        }, 1000);

        this.botData[botId].updateInterval = updateInterval;
    }

    cleanup() {
        console.log('🧹 Cleaning up bot manager...');
        Object.keys(this.instances).forEach(id => {
            if (this.instances[id]) {
                try {
                    console.log(`Stopping bot ${id}...`);
                    this.instances[id].end();
                } catch (e) {
                    console.error(`Error stopping bot ${id}:`, e.message);
                }
            }
            if (this.botData[id] && this.botData[id].updateInterval) {
                clearInterval(this.botData[id].updateInterval);
            }
        });
        this.instances = {};
        this.botData = {};
        this.botTasks = {};
        this.wsClients.clear();
        console.log('✅ Cleanup complete');
    }
}

module.exports = BotManager;