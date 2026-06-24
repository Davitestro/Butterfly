const mineflayer = require('mineflayer');

class BotController {
    constructor(io) {
        this.bots = new Map();
        this.io = io;
    }

    createBot(botId, username, password, serverIp, port = 25565) {
        if (this.bots.has(botId)) {
            throw new Error(`Bot with ID ${botId} already exists`);
        }

        const bot = mineflayer.createBot({
            host: serverIp,
            port: port,
            username: username,
            password: password,
            auth: password ? 'microsoft' : 'offline',
            version: false
        });

        this.setupBotEvents(bot, botId);
        this.bots.set(botId, { bot, username, serverIp, port, status: 'connecting' });

        return bot;
    }

    setupBotEvents(bot, botId) {
        bot.on('login', () => {
            console.log(`Bot ${botId} logged in`);
            this.updateBotStatus(botId, 'online');
        });

        bot.on('error', (err) => {
            console.error(`Bot ${botId} error:`, err);
            this.updateBotStatus(botId, 'error', err.message);
        });

        bot.on('end', () => {
            console.log(`Bot ${botId} disconnected`);
            this.updateBotStatus(botId, 'offline');
        });

        bot.on('message', (message) => {
            this.io.emit('botMessage', { botId, message: message.toString() });
        });

        bot.on('health', () => {
            this.io.emit('botHealth', { 
                botId, 
                health: bot.health, 
                food: bot.food 
            });
        });
    }

    updateBotStatus(botId, status, error = null) {
        if (this.bots.has(botId)) {
            const botData = this.bots.get(botId);
            botData.status = status;
            if (error) botData.error = error;
            this.io.emit('botStatusUpdate', { botId, status, error });
        }
    }

    removeBot(botId) {
        if (!this.bots.has(botId)) {
            throw new Error(`Bot ${botId} not found`);
        }
        const { bot } = this.bots.get(botId);
        bot.end();
        this.bots.delete(botId);
        this.io.emit('botRemoved', { botId });
    }

    getAllBots() {
        const result = [];
        for (const [id, data] of this.bots) {
            result.push({
                id,
                username: data.username,
                serverIp: data.serverIp,
                port: data.port,
                status: data.status,
                error: data.error || null
            });
        }
        return result;
    }

    executeCommand(botId, command) {
        if (!this.bots.has(botId)) {
            throw new Error(`Bot ${botId} not found`);
        }
        const { bot } = this.bots.get(botId);
        bot.chat(command);
    }
}

module.exports = BotController;