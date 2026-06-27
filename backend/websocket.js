const WebSocket = require('ws');

class WebSocketHandler {
    constructor(botManager) {
        this.botManager = botManager;
        this.wss = null;
        this.clients = new Set();
    }

    setup(server) {
        this.wss = new WebSocket.Server({ server });

        this.wss.on('connection', (ws) => {
            console.log('WebSocket client connected');
            this.clients.add(ws);
            this.botManager.addWsClient(ws);

            try {
                const bots = this.botManager.getBots();
                console.log(`Sending ${bots.length} bots to client`);
                ws.send(JSON.stringify({
                    type: 'bots_update',
                    data: bots
                }));
            } catch (err) {
                console.error('Error sending initial data:', err.message);
            }

            ws.on('message', (message) => {
                try {
                    const data = JSON.parse(message);
                    console.log(`Received: ${data.type}`);
                    this.handleMessage(ws, data);
                } catch (err) {
                    console.error('WebSocket message error:', err.message);
                }
            });

            ws.on('close', () => {
                console.log('WebSocket client disconnected');
                this.clients.delete(ws);
                this.botManager.removeWsClient(ws);
            });

            ws.on('error', (err) => {
                console.error('WebSocket error:', err.message);
            });
        });
    }

    handleMessage(ws, data) {
        try {
            switch(data.type) {
                case 'get_bots':
                    const bots = this.botManager.getBots();
                    console.log(`Sending ${bots.length} bots to client (get_bots)`);
                    ws.send(JSON.stringify({
                        type: 'bots_update',
                        data: bots
                    }));
                    break;

                case 'add_bot':
                    const newBot = this.botManager.addBot(data);
                    ws.send(JSON.stringify({
                        type: 'bot_info',
                        data: newBot
                    }));
                    this.broadcastBots(this.botManager.getBots());
                    break;

                case 'update_bot':
                    const updated = this.botManager.updateBot(data.id, data);
                    if (updated) {
                        ws.send(JSON.stringify({
                            type: 'bot_info',
                            data: updated
                        }));
                        this.broadcastBots(this.botManager.getBots());
                    }
                    break;

                case 'delete_bot':
                    const deleted = this.botManager.deleteBot(data.id);
                    if (deleted) {
                        ws.send(JSON.stringify({
                            type: 'notification',
                            title: 'Bot Deleted',
                            message: 'Bot removed successfully',
                            error: false
                        }));
                        this.broadcastBots(this.botManager.getBots());
                    }
                    break;

                case 'start_bot':
                    this.botManager.startBot(data.id, ws);
                    break;

                case 'stop_bot':
                    const stopped = this.botManager.stopBot(data.id);
                    if (stopped) {
                        ws.send(JSON.stringify({
                            type: 'notification',
                            title: 'Bot Stopped',
                            message: 'Bot stopped successfully',
                            error: false
                        }));
                    }
                    break;

                case 'get_bot_info':
                    const botInfo = this.botManager.getBot(data.id);
                    if (botInfo) {
                        ws.send(JSON.stringify({
                            type: 'bot_info',
                            data: botInfo
                        }));

                        const botData = this.botManager.getBotData(data.id);
                        if (botData && Object.keys(botData).length > 0) {
                            ws.send(JSON.stringify({
                                type: 'bot_stats',
                                data: botData
                            }));

                            if (botData.inventory && botData.inventory.length > 0) {
                                ws.send(JSON.stringify({
                                    type: 'bot_inventory',
                                    data: botData.inventory
                                }));
                            }
                        }

                        // Send tasks
                        const taskData = this.botManager.botTasks && this.botManager.botTasks[data.id];
                        if (taskData) {
                            ws.send(JSON.stringify({
                                type: 'bot_tasks',
                                data: taskData.tasks || []
                            }));
                        }
                    }
                    break;

                case 'bot_command':
                    const botInstance = this.botManager.getBotInstance(data.id);
                    if (!botInstance) {
                        ws.send(JSON.stringify({
                            type: 'notification',
                            title: 'Error',
                            message: 'Bot is not connected. Start the bot first.',
                            error: true
                        }));
                        return;
                    }

                    if (typeof botInstance.executeCommand === 'function') {
                        botInstance.executeCommand(data.command)
                            .then(response => {
                                ws.send(JSON.stringify({
                                    type: 'bot_command_response',
                                    command: data.command,
                                    response: response
                                }));
                            })
                            .catch(err => {
                                ws.send(JSON.stringify({
                                    type: 'notification',
                                    title: 'Command Error',
                                    message: `Failed to execute command: ${err.message}`,
                                    error: true
                                }));
                            });
                    } else {
                        botInstance.chat(data.command);
                        ws.send(JSON.stringify({
                            type: 'bot_command_response',
                            command: data.command,
                            response: `Command sent: ${data.command}`
                        }));
                    }
                    break;

                case 'get_tasks':
                    const taskData2 = this.botManager.botTasks && this.botManager.botTasks[data.id];
                    if (taskData2) {
                        ws.send(JSON.stringify({
                            type: 'bot_tasks',
                            data: taskData2.tasks || []
                        }));
                    }
                    break;
            }
        } catch (err) {
            console.error('Error in handleMessage:', err.message);
            try {
                ws.send(JSON.stringify({
                    type: 'notification',
                    title: 'Error',
                    message: `Failed to process request: ${err.message}`,
                    error: true
                }));
            } catch (e) {}
        }
    }

    broadcastBots(bots) {
        try {
            const message = JSON.stringify({
                type: 'bots_update',
                data: bots || this.botManager.getBots()
            });

            console.log(`Broadcasting ${bots ? bots.length : this.botManager.getBots().length} bots to ${this.clients.size} clients`);
            
            this.clients.forEach(client => {
                if (client && client.readyState === WebSocket.OPEN) {
                    try {
                        client.send(message);
                    } catch (err) {
                        console.error('Error broadcasting to client:', err.message);
                    }
                }
            });
        } catch (err) {
            console.error('Error in broadcastBots:', err.message);
        }
    }

    broadcastNotification(title, message, isError) {
        try {
            const notification = JSON.stringify({
                type: 'notification',
                title: title,
                message: message,
                error: isError
            });

            this.clients.forEach(client => {
                if (client && client.readyState === WebSocket.OPEN) {
                    try {
                        client.send(notification);
                    } catch (err) {
                        console.error('Error broadcasting notification:', err.message);
                    }
                }
            });
        } catch (err) {
            console.error('Error in broadcastNotification:', err.message);
        }
    }
}

module.exports = WebSocketHandler;