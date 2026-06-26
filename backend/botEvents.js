function setupEvents(bot, botConfig, botManager) {
    bot.on('login', () => {
        console.log(`✅ [${botConfig.name}] Connected successfully!`);
        
        const botEntry = botManager.bots.find(b => b.id === botConfig.id);
        if (botEntry) {
            botEntry.status = 'online';
            if (botManager.saveCallback) botManager.saveCallback(botManager.bots);
            if (botManager.broadcastCallback) botManager.broadcastCallback(botManager.bots);
        }

        botManager.sendToAllClients({
            type: 'notification',
            title: 'Bot Connected',
            message: `${botConfig.name} is now online!`,
            error: false
        });

        setTimeout(() => {
            if (botManager.botData[botConfig.id]) {
                const data = botManager.botData[botConfig.id];
                botManager.sendToAllClients({
                    type: 'bot_stats',
                    data: data
                });
            }
        }, 1000);
    });

    bot.on('health', () => {
        const data = botManager.botData[botConfig.id];
        if (data) {
            data.health = Math.round(bot.health * 10) / 10;
            data.food = bot.food;
            data.lastUpdate = Date.now();
        }
    });

    bot.on('experience', () => {
        const data = botManager.botData[botConfig.id];
        if (data) {
            data.experience = Math.round(bot.experience);
            data.level = bot.level;
            data.lastUpdate = Date.now();
        }
    });

    bot.on('move', () => {
        const data = botManager.botData[botConfig.id];
        if (data && bot.entity && bot.entity.position) {
            data.position = {
                x: Math.round(bot.entity.position.x * 10) / 10,
                y: Math.round(bot.entity.position.y * 10) / 10,
                z: Math.round(bot.entity.position.z * 10) / 10
            };
            data.lastUpdate = Date.now();
        }
    });

    bot.on('inventory', () => {
        const data = botManager.botData[botConfig.id];
        if (data && bot.inventory) {
            data.inventory = bot.inventory.items().map(item => ({
                name: item.name,
                count: item.count,
                slot: item.slot,
                displayName: item.displayName || item.name
            }));
            data.lastUpdate = Date.now();

            botManager.sendToAllClients({
                type: 'bot_inventory',
                data: data.inventory
            });
        }
    });

    bot.on('spawn', () => {
        console.log(`[${botConfig.name}] Spawned in the world`);
        
        setTimeout(() => {
            const data = botManager.botData[botConfig.id];
            if (data && bot.inventory) {
                data.inventory = bot.inventory.items().map(item => ({
                    name: item.name,
                    count: item.count,
                    slot: item.slot,
                    displayName: item.displayName || item.name
                }));
                botManager.sendToAllClients({
                    type: 'bot_inventory',
                    data: data.inventory
                });
            }
        }, 1000);
    });

    bot.on('error', (err) => {
        console.error(`❌ [${botConfig.name}] Error:`, err.message);
        
        let errorMessage = err.message;
        if (err.message.includes('ENOTFOUND')) {
            errorMessage = `Server "${botConfig.server}" is not reachable. Make sure it's online.`;
        } else if (err.message.includes('ECONNRESET')) {
            errorMessage = 'Connection was reset. Server may be restarting.';
        } else if (err.message.includes('authentication')) {
            errorMessage = 'Authentication failed. Check username/password or use offline mode.';
        } else if (err.message.includes('timeout')) {
            errorMessage = 'Connection timed out. Server may be slow or unreachable.';
        }

        botManager.sendToAllClients({
            type: 'notification',
            title: 'Bot Error',
            message: `${botConfig.name}: ${errorMessage}`,
            error: true
        });

        const botEntry = botManager.bots.find(b => b.id === botConfig.id);
        if (botEntry) {
            botEntry.status = 'error';
            if (botManager.saveCallback) botManager.saveCallback(botManager.bots);
            if (botManager.broadcastCallback) botManager.broadcastCallback(botManager.bots);
        }
    });

    bot.on('end', (reason) => {
        console.log(`[${botConfig.name}] Disconnected: ${reason || 'Unknown reason'}`);
        
        const botEntry = botManager.bots.find(b => b.id === botConfig.id);
        if (botEntry) {
            botEntry.status = 'stopped';
            if (botManager.saveCallback) botManager.saveCallback(botManager.bots);
            if (botManager.broadcastCallback) botManager.broadcastCallback(botManager.bots);
        }

        if (botManager.botData[botConfig.id]?.updateInterval) {
            clearInterval(botManager.botData[botConfig.id].updateInterval);
        }

        delete botManager.instances[botConfig.id];

        botManager.sendToAllClients({
            type: 'notification',
            title: 'Bot Disconnected',
            message: `${botConfig.name} has disconnected.`,
            error: true
        });
    });

    bot.on('message', (message) => {
        console.log(`[${botConfig.name}] Chat: ${message.toString()}`);
    });

    bot.on('kicked', (reason) => {
        console.log(`[${botConfig.name}] Kicked: ${reason}`);
        botManager.sendToAllClients({
            type: 'notification',
            title: 'Bot Kicked',
            message: `${botConfig.name} was kicked: ${reason}`,
            error: true
        });
    });
}

module.exports = { setupEvents };