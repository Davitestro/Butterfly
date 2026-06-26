const { GoalBlock } = require('mineflayer-pathfinder').goals;

function setupCommands(bot, botConfig, botManager) {
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

                switch(cmd) {
                    case 'moveto':
                    case 'move':
                        if (parts.length >= 4) {
                            const x = parseFloat(parts[1]);
                            const y = parseFloat(parts[2]);
                            const z = parseFloat(parts[3]);
                            if (!isNaN(x) && !isNaN(y) && !isNaN(z)) {
                                // Используем pathfinder с GoalBlock
                                if (bot.pathfinder && bot.pathfinder.setGoal) {
                                    try {
                                        const goal = new GoalBlock(Math.floor(x), Math.floor(y), Math.floor(z));
                                        bot.pathfinder.setGoal(goal);
                                        response = `Moving to ${x}, ${y}, ${z} using pathfinder`;
                                    } catch (err) {
                                        console.error('Pathfinder error:', err.message);
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
                            collectResources(bot, resource, botManager);
                        } else {
                            response = 'Please specify a resource to collect';
                        }
                        break;

                    case 'drop':
                        const dropItem = parts.slice(1).join(' ');
                        if (dropItem) {
                            const item = bot.inventory.items().find(i => 
                                i.name.toLowerCase().includes(dropItem.toLowerCase())
                            );
                            if (item) {
                                bot.toss(item.type, null, item.count);
                                response = `Dropped ${dropItem} (${item.count})`;
                            } else {
                                response = `No ${dropItem} found in inventory`;
                            }
                        } else {
                            response = 'Please specify an item to drop';
                        }
                        break;

                    case 'hunt':
                        const animal = parts.slice(1).join(' ');
                        if (animal) {
                            response = `Hunting ${animal}`;
                            bot.chat(`/say Hunting ${animal}`);
                            huntAnimals(bot, animal, botManager);
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
                        response = 'Stopping current action';
                        if (bot.pathfinder && bot.pathfinder.setGoal) {
                            bot.pathfinder.setGoal(null);
                        }
                        bot.chat('/say Stopped current action');
                        break;

                    case 'status':
                    case 'stats':
                        const data = botManager.botData[botConfig.id];
                        if (data) {
                            response = `Health: ${data.health}/20 | Food: ${data.food}/20 | Level: ${data.level} | Position: ${data.position.x}, ${data.position.y}, ${data.position.z}`;
                        } else {
                            response = 'Status data not available';
                        }
                        break;

                    case 'help':
                        response = `Available commands:
moveto <x> <y> <z> - Move to position
guard <radius> - Guard area
collect <resource> - Collect resources
drop <item> - Drop item
hunt <animal> - Hunt animals
hunt_player <name> - Hunt players
status - Show bot status
stop - Stop current action
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

async function collectResources(bot, resource, botManager) {
    try {
        const blocks = bot.findBlocks({
            matching: (block) => {
                return block.name && block.name.toLowerCase().includes(resource.toLowerCase());
            },
            maxDistance: 32,
            count: 10
        });

        if (blocks.length === 0) {
            botManager.sendToAllClients({
                type: 'bot_command_response',
                command: `collect ${resource}`,
                response: `No ${resource} found nearby`
            });
            return;
        }

        botManager.sendToAllClients({
            type: 'bot_command_response',
            command: `collect ${resource}`,
            response: `Found ${blocks.length} ${resource} blocks, mining...`
        });

        for (const blockPos of blocks) {
            const block = bot.blockAt(blockPos);
            if (block) {
                try {
                    // Move to block first using GoalBlock
                    if (bot.pathfinder && bot.pathfinder.setGoal) {
                        try {
                            const { GoalBlock } = require('mineflayer-pathfinder').goals;
                            const goal = new GoalBlock(blockPos.x, blockPos.y, blockPos.z);
                            bot.pathfinder.setGoal(goal);
                            await new Promise(resolve => setTimeout(resolve, 2000));
                        } catch (err) {
                            console.error('Pathfinder error:', err.message);
                        }
                    }
                    await bot.dig(block);
                    botManager.sendToAllClients({
                        type: 'bot_command_response',
                        command: `collect ${resource}`,
                        response: `Mined ${block.name}`
                    });
                } catch (err) {
                    console.error(`[${bot.username}] Mining error:`, err.message);
                }
            }
        }
    } catch (err) {
        console.error(`[${bot.username}] Collect error:`, err.message);
    }
}

async function huntAnimals(bot, animal, botManager) {
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
                response: `No ${animal} found nearby`
            });
            return;
        }

        botManager.sendToAllClients({
            type: 'bot_command_response',
            command: `hunt ${animal}`,
            response: `Found ${entities.length} ${animal}(s), attacking...`
        });

        for (const entity of entities) {
            try {
                // Move to entity first
                if (bot.pathfinder && bot.pathfinder.setGoal && entity.position) {
                    try {
                        const { GoalBlock } = require('mineflayer-pathfinder').goals;
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
                await new Promise(resolve => setTimeout(resolve, 500));
                botManager.sendToAllClients({
                    type: 'bot_command_response',
                    command: `hunt ${animal}`,
                    response: `Attacked ${entity.name}`
                });
            } catch (err) {
                console.error(`[${bot.username}] Attack error:`, err.message);
            }
        }
    } catch (err) {
        console.error(`[${bot.username}] Hunt error:`, err.message);
    }
}

module.exports = { setupCommands };