const { GoalBlock, GoalFollow } = require('mineflayer-pathfinder').goals;

function setupCommands(bot, botConfig, botManager) {
    // Store task data
    if (!botManager.botTasks) {
        botManager.botTasks = {};
    }
    if (!botManager.botTasks[botConfig.id]) {
        botManager.botTasks[botConfig.id] = {
            tasks: [],
            currentTask: null,
            completed: 0,
            total: 0,
            isRunning: false,
            progress: 0
        };
    }

    bot.on('chat', (username, message) => {
        if (username === bot.username) return;
        console.log(`[${botConfig.name}] <${username}> ${message}`);
    });

    bot.executeCommand = function(command) {
        return new Promise((resolve, reject) => {
            try {
                const parts = command.split(' ');
                const cmd = parts[0].toLowerCase();
                let response = '';
                const taskData = botManager.botTasks[botConfig.id];

                switch(cmd) {
                    case 'moveto':
                    case 'move':
                        if (parts.length >= 4) {
                            const x = parseFloat(parts[1]);
                            const y = parseFloat(parts[2]);
                            const z = parseFloat(parts[3]);
                            if (!isNaN(x) && !isNaN(y) && !isNaN(z)) {
                                if (bot.pathfinder && bot.pathfinder.setGoal) {
                                    try {
                                        const goal = new GoalBlock(Math.floor(x), Math.floor(y), Math.floor(z));
                                        bot.pathfinder.setGoal(goal);
                                        response = `Moving to ${x}, ${y}, ${z}`;
                                    } catch (err) {
                                        bot.chat(`/tp ${x} ${y} ${z}`);
                                        response = `Teleporting to ${x}, ${y}, ${z}`;
                                    }
                                } else {
                                    bot.chat(`/tp ${x} ${y} ${z}`);
                                    response = `Teleporting to ${x}, ${y}, ${z}`;
                                }
                            } else {
                                response = 'Invalid coordinates. Usage: moveto <x> <y> <z>';
                            }
                        } else {
                            response = 'Usage: moveto <x> <y> <z>';
                        }
                        break;

                    case 'follow':
                        const playerName = parts.slice(1).join(' ');
                        if (playerName) {
                            response = `Following ${playerName}`;
                            followPlayer(bot, playerName, botManager, botConfig);
                        } else {
                            response = 'Please specify a player name to follow';
                        }
                        break;

                    case 'guard':
                        const radius = parseInt(parts[1]) || 10;
                        response = `Guarding area with radius ${radius}`;
                        bot.chat(`/say Guarding area with radius ${radius}`);
                        break;

                    case 'collect':
                        const resource = parts.slice(1).join(' ');
                        if (resource) {
                            response = `Collecting ${resource}`;
                            bot.chat(`/say Collecting ${resource}`);
                            collectResources(bot, resource, botManager, botConfig);
                        } else {
                            response = 'Please specify a resource to collect';
                        }
                        break;

                    case 'drop':
                        if (parts.length >= 2) {
                            const itemName = parts[1];
                            const count = parts.length > 2 ? parseInt(parts[2]) : 999;
                            const item = bot.inventory.items().find(i => 
                                i.name.toLowerCase().includes(itemName.toLowerCase())
                            );
                            if (item) {
                                const dropCount = Math.min(count, item.count);
                                bot.toss(item.type, null, dropCount);
                                response = `Dropped ${dropCount} ${itemName}`;
                            } else {
                                response = `No ${itemName} found in inventory`;
                            }
                        } else {
                            response = 'Usage: drop <item> [count]';
                        }
                        break;

                    case 'hunt':
                        const animal = parts.slice(1).join(' ');
                        if (animal) {
                            response = `Hunting ${animal}`;
                            bot.chat(`/say Hunting ${animal}`);
                            huntAnimals(bot, animal, botManager, botConfig);
                        } else {
                            response = 'Please specify an animal to hunt';
                        }
                        break;

                    case 'hunt_player':
                    case 'huntplayer':
                        const player = parts.slice(1).join(' ') || 'any player';
                        response = `Hunting ${player}`;
                        bot.chat(`/say Hunting ${player}`);
                        break;

                    case 'stop':
                        response = 'Stopping all actions';
                        if (bot.pathfinder && bot.pathfinder.setGoal) {
                            bot.pathfinder.setGoal(null);
                        }
                        if (taskData) {
                            taskData.isRunning = false;
                            taskData.currentTask = null;
                            taskData.progress = 0;
                        }
                        bot.chat('/say Stopped all actions');
                        break;

                    case 'status':
                    case 'stats':
                        const data = botManager.botData[botConfig.id];
                        if (data) {
                            let taskStatus = '';
                            if (taskData && taskData.currentTask) {
                                const progress = taskData.progress || 0;
                                taskStatus = ` | Task: ${taskData.currentTask} (${progress}%)`;
                            }
                            response = `❤️ Health: ${data.health}/20 | 🍖 Food: ${data.food}/20 | ⭐ Level: ${data.level} | 📍 ${data.position.x}, ${data.position.y}, ${data.position.z}${taskStatus}`;
                        } else {
                            response = 'Status data not available';
                        }
                        break;

                    case 'tasks':
                        if (taskData && taskData.tasks.length > 0) {
                            let taskStr = '📋 Tasks:\n';
                            taskData.tasks.forEach((t, i) => {
                                const status = t.completed ? '✅' : '⏳';
                                const progress = t.progress || 0;
                                taskStr += `  ${i+1}. ${status} ${t.name} (${progress}%)\n`;
                            });
                            response = taskStr;
                        } else {
                            response = 'No tasks assigned';
                        }
                        break;

                    case 'help':
                        response = `Available commands:
moveto <x> <y> <z> - Move to position
follow <player> - Follow a player
guard <radius> - Guard area
collect <resource> - Collect resources
drop <item> [count] - Drop item
hunt <animal> - Hunt animals
hunt_player <name> - Hunt players
status - Show bot status
tasks - Show task list
stop - Stop all actions
help - Show this help`;
                        break;

                    default:
                        bot.chat(command);
                        response = `Command sent: ${command}`;
                }

                resolve(response);
            } catch (err) {
                reject(err);
            }
        });
    };
}

// Follow player function
function followPlayer(bot, playerName, botManager, botConfig) {
    const taskData = botManager.botTasks[botConfig.id];
    taskData.currentTask = `Following ${playerName}`;
    taskData.isRunning = true;
    taskData.completed = 0;
    taskData.total = 0;
    taskData.progress = 0;

    taskData.tasks.push({
        name: `Follow ${playerName}`,
        completed: false,
        started: Date.now(),
        progress: 0,
        total: 0
    });

    const targetPlayer = bot.players[playerName];
    if (!targetPlayer || !targetPlayer.entity) {
        botManager.sendToAllClients({
            type: 'bot_command_response',
            command: `follow ${playerName}`,
            response: `Player ${playerName} not found`
        });
        taskData.isRunning = false;
        taskData.currentTask = null;
        return;
    }

    const followInterval = setInterval(() => {
        if (!taskData.isRunning) {
            clearInterval(followInterval);
            return;
        }

        const player = bot.players[playerName];
        if (!player || !player.entity) {
            botManager.sendToAllClients({
                type: 'bot_command_response',
                command: `follow ${playerName}`,
                response: `Lost ${playerName}`
            });
            clearInterval(followInterval);
            taskData.isRunning = false;
            taskData.currentTask = null;
            taskData.progress = 0;
            return;
        }

        if (bot.pathfinder && bot.pathfinder.setGoal) {
            try {
                const goal = new GoalFollow(player.entity, 2);
                bot.pathfinder.setGoal(goal);
                taskData.progress = 50; // Following is in progress
            } catch (err) {
                console.error('Follow error:', err.message);
            }
        }
    }, 1000);

    taskData.followInterval = followInterval;
    taskData.progress = 10;

    botManager.sendToAllClients({
        type: 'bot_command_response',
        command: `follow ${playerName}`,
        response: `Following ${playerName}`
    });
}

// Collect resources with progress
async function collectResources(bot, resource, botManager, botConfig) {
    const taskData = botManager.botTasks[botConfig.id];
    taskData.currentTask = `Collecting ${resource}`;
    taskData.isRunning = true;
    taskData.progress = 0;

    taskData.tasks.push({
        name: `Collect ${resource}`,
        completed: false,
        started: Date.now(),
        progress: 0,
        total: 0
    });

    try {
        const blocks = bot.findBlocks({
            matching: (block) => {
                return block.name && block.name.toLowerCase().includes(resource.toLowerCase());
            },
            maxDistance: 32,
            count: 50
        });

        if (blocks.length === 0) {
            botManager.sendToAllClients({
                type: 'bot_command_response',
                command: `collect ${resource}`,
                response: `❌ No ${resource} found nearby. Do you want to provide it? (Place in a chest or use /give)`
            });
            taskData.isRunning = false;
            taskData.currentTask = null;
            taskData.progress = 0;
            return;
        }

        const totalBlocks = Math.min(blocks.length, 10);
        taskData.total = totalBlocks;
        taskData.completed = 0;
        taskData.tasks[taskData.tasks.length - 1].total = totalBlocks;

        botManager.sendToAllClients({
            type: 'bot_command_response',
            command: `collect ${resource}`,
            response: `Found ${blocks.length} ${resource} blocks, mining ${totalBlocks}...`
        });

        let mined = 0;
        for (const blockPos of blocks) {
            if (!taskData.isRunning) break;
            if (mined >= 10) break;

            const block = bot.blockAt(blockPos);
            if (block) {
                try {
                    if (bot.pathfinder && bot.pathfinder.setGoal) {
                        try {
                            const goal = new GoalBlock(blockPos.x, blockPos.y, blockPos.z);
                            bot.pathfinder.setGoal(goal);
                            await new Promise(resolve => setTimeout(resolve, 2000));
                        } catch (err) {
                            console.error('Pathfinder error:', err.message);
                        }
                    }
                    await bot.dig(block);
                    mined++;
                    taskData.completed = mined;
                    taskData.progress = Math.round((mined / totalBlocks) * 100);
                    taskData.tasks[taskData.tasks.length - 1].progress = taskData.progress;
                    
                    botManager.sendToAllClients({
                        type: 'bot_command_response',
                        command: `collect ${resource}`,
                        response: `⛏️ Mined ${block.name} (${taskData.progress}%)`
                    });
                } catch (err) {
                    console.error(`[${bot.username}] Mining error:`, err.message);
                }
            }
        }

        taskData.tasks[taskData.tasks.length - 1].completed = true;
        taskData.tasks[taskData.tasks.length - 1].completedAt = Date.now();
        taskData.currentTask = null;
        taskData.isRunning = false;
        taskData.progress = 100;

        botManager.sendToAllClients({
            type: 'bot_command_response',
            command: `collect ${resource}`,
            response: `✅ Finished collecting ${resource}. Mined ${mined} blocks.`
        });

    } catch (err) {
        console.error(`[${bot.username}] Collect error:`, err.message);
        taskData.isRunning = false;
        taskData.currentTask = null;
        taskData.progress = 0;
        botManager.sendToAllClients({
            type: 'bot_command_response',
            command: `collect ${resource}`,
            response: `❌ Error: ${err.message}`
        });
    }
}

// Hunt animals with progress
async function huntAnimals(bot, animal, botManager, botConfig) {
    const taskData = botManager.botTasks[botConfig.id];
    taskData.currentTask = `Hunting ${animal}`;
    taskData.isRunning = true;
    taskData.progress = 0;

    taskData.tasks.push({
        name: `Hunt ${animal}`,
        completed: false,
        started: Date.now(),
        progress: 0,
        total: 0
    });

    try {
        const entities = Object.values(bot.entities).filter(e => 
            e.type === 'mob' && 
            e.name && 
            e.name.toLowerCase().includes(animal.toLowerCase())
        );

        if (entities.length === 0) {
            botManager.sendToAllClients({
                type: 'bot_command_response',
                command: `hunt ${animal}`,
                response: `❌ No ${animal} found nearby`
            });
            taskData.isRunning = false;
            taskData.currentTask = null;
            taskData.progress = 0;
            return;
        }

        const totalEntities = Math.min(entities.length, 10);
        taskData.total = totalEntities;
        taskData.completed = 0;
        taskData.tasks[taskData.tasks.length - 1].total = totalEntities;

        botManager.sendToAllClients({
            type: 'bot_command_response',
            command: `hunt ${animal}`,
            response: `Found ${entities.length} ${animal}(s), attacking ${totalEntities}...`
        });

        let killed = 0;
        for (const entity of entities) {
            if (!taskData.isRunning) break;
            if (killed >= 10) break;

            try {
                if (bot.pathfinder && bot.pathfinder.setGoal && entity.position) {
                    try {
                        const goal = new GoalBlock(
                            Math.floor(entity.position.x),
                            Math.floor(entity.position.y),
                            Math.floor(entity.position.z)
                        );
                        bot.pathfinder.setGoal(goal);
                        await new Promise(resolve => setTimeout(resolve, 1500));
                    } catch (err) {
                        console.error('Pathfinder error:', err.message);
                    }
                }
                bot.attack(entity);
                killed++;
                taskData.completed = killed;
                taskData.progress = Math.round((killed / totalEntities) * 100);
                taskData.tasks[taskData.tasks.length - 1].progress = taskData.progress;
                
                botManager.sendToAllClients({
                    type: 'bot_command_response',
                    command: `hunt ${animal}`,
                    response: `⚔️ Attacked ${entity.name} (${taskData.progress}%)`
                });
                await new Promise(resolve => setTimeout(resolve, 500));
            } catch (err) {
                console.error(`[${bot.username}] Attack error:`, err.message);
            }
        }

        taskData.tasks[taskData.tasks.length - 1].completed = true;
        taskData.tasks[taskData.tasks.length - 1].completedAt = Date.now();
        taskData.currentTask = null;
        taskData.isRunning = false;
        taskData.progress = 100;

        botManager.sendToAllClients({
            type: 'bot_command_response',
            command: `hunt ${animal}`,
            response: `✅ Finished hunting. Killed ${killed} ${animal}(s).`
        });

    } catch (err) {
        console.error(`[${bot.username}] Hunt error:`, err.message);
        taskData.isRunning = false;
        taskData.currentTask = null;
        taskData.progress = 0;
        botManager.sendToAllClients({
            type: 'bot_command_response',
            command: `hunt ${animal}`,
            response: `❌ Error: ${err.message}`
        });
    }
}

module.exports = { setupCommands };