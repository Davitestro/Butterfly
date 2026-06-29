const { GoalBlock, GoalFollow } = require('mineflayer-pathfinder').goals;

const HOSTILE_MOBS = new Set([
    'zombie', 'skeleton', 'creeper', 'spider', 'enderman', 'witch',
    'blaze', 'ghast', 'wither_skeleton', 'phantom', 'drowned',
    'husk', 'stray', 'pillager', 'vindicator', 'evoker', 'ravager',
    'warden', 'breeze', 'bogged'
]);

const DANGEROUS_PROJECTILES = new Set(['arrow', 'fireball', 'small_fireball', 'snowball']);

const RANGED_WEAPONS = new Set(['bow', 'crossbow', 'trident']);
const MELEE_WEAPONS = new Set([
    'netherite_sword', 'diamond_sword', 'iron_sword', 'stone_sword', 'wooden_sword', 'golden_sword',
    'netherite_axe', 'diamond_axe', 'iron_axe', 'stone_axe', 'wooden_axe', 'golden_axe'
]);

const FOOD_ITEMS = [
    'cooked_beef', 'cooked_porkchop', 'cooked_chicken', 'cooked_mutton',
    'cooked_salmon', 'cooked_cod', 'bread', 'baked_potato', 'golden_apple',
    'golden_carrot', 'apple', 'beef', 'porkchop', 'chicken', 'mutton',
    'salmon', 'cod', 'potato', 'carrot', 'melon_slice', 'sweet_berries'
];

class SurvivalAI {
    constructor(bot, botConfig, botManager) {
        this.bot = bot;
        this.botConfig = botConfig;
        this.botManager = botManager;
        this.enabled = true;
        this.userCommandActive = false;
        this.lastEatTime = 0;
        this.lastThreatCheck = 0;
        this.fleeing = false;
        this.combatTarget = null;
        this.combatInterval = null;
        this.survivalInterval = null;
        this.mapInterval = null;
        this.alertedPlayers = new Set();
    }

    start() {
        this.survivalInterval = setInterval(() => this._survivalTick(), 1000);
        this.mapInterval = setInterval(() => this._broadcastMap(), 3000);
        console.log(`[${this.botConfig.name}] Survival AI started`);
    }

    stop() {
        if (this.survivalInterval) clearInterval(this.survivalInterval);
        if (this.combatInterval) clearInterval(this.combatInterval);
        if (this.mapInterval) clearInterval(this.mapInterval);
        this.survivalInterval = null;
        this.combatInterval = null;
        this.mapInterval = null;
    }

    setUserCommandActive(active) {
        this.userCommandActive = active;
    }

    _survivalTick() {
        if (!this.enabled || !this.bot || !this.bot.entity) return;
        if (this.fleeing) return;

        const now = Date.now();

        // Auto-eat when hungry
        if (now - this.lastEatTime > 3000) {
            this._autoEat();
        }

        // Threat detection every 500ms
        if (now - this.lastThreatCheck > 500) {
            this.lastThreatCheck = now;
            this._checkThreats();
        }
    }

    _autoEat() {
        if (!this.bot || this.bot.food >= 18) return;
        if (this.userCommandActive && !this.fleeing) return;

        const inventory = this.bot.inventory.items();
        const foodItem = inventory.find(item => FOOD_ITEMS.includes(item.name));

        if (foodItem) {
            this.bot.equip(foodItem, 'hand').then(() => {
                return this.bot.consume();
            }).then(() => {
                this.lastEatTime = Date.now();
                this._notify('Auto-eat', `Ate ${foodItem.displayName || foodItem.name}`);
            }).catch(() => {});
        }
    }

    _checkThreats() {
        if (!this.bot || !this.bot.entity) return;
        const pos = this.bot.entity.position;

        // Find nearby hostile mobs
        const threats = Object.values(this.bot.entities).filter(e => {
            if (!e || !e.position || !e.name) return false;
            if (e.type !== 'mob') return false;
            if (!HOSTILE_MOBS.has(e.name)) return false;
            const dist = pos.distanceTo(e.position);
            return dist < 16;
        });

        if (threats.length === 0) {
            if (this.fleeing) {
                this.fleeing = false;
                this.bot.pathfinder.setGoal(null);
                this._notify('Safe', 'Threats cleared, resuming normal behavior');
            }
            return;
        }

        // If health is low, flee
        if (this.bot.health < 8 && threats.length > 0) {
            this._flee(threats[0]);
            return;
        }

        // If health is decent, fight back (only if user isn't commanding)
        if (!this.userCommandActive && this.bot.health >= 8) {
            this._autoDefend(threats);
        }
    }

    _flee(threat) {
        if (this.fleeing) return;
        this.fleeing = true;
        this._notify('Fleeing', `Low health (${Math.round(this.bot.health)}), running from ${threat.name}!`);

        // Run away from threat
        const botPos = this.bot.entity.position;
        const threatPos = threat.position;
        const dx = botPos.x - threatPos.x;
        const dz = botPos.z - threatPos.z;
        const dist = Math.sqrt(dx * dx + dz * dz) || 1;
        const fleeX = Math.floor(botPos.x + (dx / dist) * 30);
        const fleeZ = Math.floor(botPos.z + (dz / dist) * 30);

        if (this.bot.pathfinder) {
            this.bot.pathfinder.setGoal(new GoalBlock(fleeX, Math.floor(botPos.y), fleeZ));
        }

        // Auto-eat while fleeing if possible
        setTimeout(() => {
            this._autoEat();
        }, 1000);

        // Stop fleeing after 10 seconds
        setTimeout(() => {
            this.fleeing = false;
        }, 10000);
    }

    _autoDefend(threats) {
        // Pick closest threat
        const pos = this.bot.entity.position;
        const closest = threats.reduce((a, b) => {
            const da = pos.distanceTo(a.position);
            const db = pos.distanceTo(b.position);
            return da < db ? a : b;
        });

        const dist = pos.distanceTo(closest.position);

        // Equip best weapon
        this._equipBestWeapon(dist < 4);

        if (dist < 4) {
            // Melee range
            this.bot.attack(closest);
        } else if (dist < 16) {
            // Try to close distance
            if (this.bot.pathfinder) {
                this.bot.pathfinder.setGoal(new GoalBlock(
                    Math.floor(closest.position.x),
                    Math.floor(closest.position.y),
                    Math.floor(closest.position.z)
                ));
            }
        }
    }

    _equipBestWeapon(isMelee) {
        const inventory = this.bot.inventory.items();

        if (isMelee) {
            for (const weapon of ['netherite_sword', 'diamond_sword', 'iron_sword', 'stone_sword',
                                   'netherite_axe', 'diamond_axe', 'iron_axe']) {
                const item = inventory.find(i => i.name === weapon);
                if (item) {
                    this.bot.equip(item, 'hand').catch(() => {});
                    return;
                }
            }
        } else {
            // Check for bow + arrows
            const bow = inventory.find(i => i.name === 'bow');
            const arrows = inventory.find(i => i.name === 'arrow');
            if (bow && arrows) {
                this.bot.equip(bow, 'hand').catch(() => {});
                return;
            }
            // Fallback to melee weapons
            for (const weapon of ['netherite_sword', 'diamond_sword', 'iron_sword']) {
                const item = inventory.find(i => i.name === weapon);
                if (item) {
                    this.bot.equip(item, 'hand').catch(() => {});
                    return;
                }
            }
        }
    }

    // ---- PVP SYSTEM ----

    engagePlayer(targetName) {
        if (this.combatInterval) clearInterval(this.combatInterval);

        this.combatTarget = targetName;
        this.userCommandActive = true;

        this.combatInterval = setInterval(() => {
            this._pvpTick();
        }, 300);
    }

    disengage() {
        if (this.combatInterval) clearInterval(this.combatInterval);
        this.combatInterval = null;
        this.combatTarget = null;
        this.userCommandActive = false;
        if (this.bot.pathfinder) this.bot.pathfinder.setGoal(null);
    }

    _pvpTick() {
        if (!this.bot || !this.bot.entity) return;

        // Find target player
        let targetEntity = null;
        if (this.combatTarget === 'any') {
            // Find nearest player that isn't us
            const players = Object.values(this.bot.players).filter(p =>
                p.entity && p.username !== this.bot.username
            );
            if (players.length > 0) {
                const pos = this.bot.entity.position;
                players.sort((a, b) => pos.distanceTo(a.entity.position) - pos.distanceTo(b.entity.position));
                targetEntity = players[0].entity;
            }
        } else {
            const player = this.bot.players[this.combatTarget];
            if (player && player.entity) {
                targetEntity = player.entity;
            }
        }

        if (!targetEntity) {
            this._notify('PvP', 'Target lost or not found');
            this.disengage();
            return;
        }

        const pos = this.bot.entity.position;
        const dist = pos.distanceTo(targetEntity.position);
        const health = this.bot.health;

        // Flee if critically low
        if (health < 6) {
            this._pvpFlee(targetEntity);
            return;
        }

        // Tactical decisions
        if (dist > 20) {
            // Too far, close distance
            this._equipBestWeapon(false);
            this._moveTo(targetEntity.position);
        } else if (dist > 4) {
            // Medium range - try ranged attack
            const hasRanged = this._tryEquipBow();
            if (hasRanged) {
                this.bot.lookAt(targetEntity.position.offset(0, 1.6, 0));
                this.bot.activateItem();
                setTimeout(() => this.bot.deactivateItem(), 800);
            } else {
                this._equipBestWeapon(false);
                this._moveTo(targetEntity.position);
            }
        } else if (dist <= 4) {
            // Close combat - hit and strafe
            this._equipBestWeapon(true);
            this._combatStrafe(targetEntity);
            this.bot.attack(targetEntity);
        }
    }

    _combatStrafe(targetEntity) {
        // Strafe around target to dodge hits
        const pos = this.bot.entity.position;
        const targetPos = targetEntity.position;
        const angle = Math.atan2(pos.z - targetPos.z, pos.x - targetPos.x);
        const strafeAngle = angle + (Math.random() > 0.5 ? Math.PI / 2 : -Math.PI / 2);
        const strafeDist = 3;
        const strafeX = Math.floor(targetPos.x + Math.cos(strafeAngle) * strafeDist);
        const strafeZ = Math.floor(targetPos.z + Math.sin(strafeAngle) * strafeDist);

        if (this.bot.pathfinder) {
            this.bot.pathfinder.setGoal(new GoalBlock(strafeX, Math.floor(pos.y), strafeZ));
        }
    }

    _pvpFlee(targetEntity) {
        const pos = this.bot.entity.position;
        const targetPos = targetEntity.position;
        const dx = pos.x - targetPos.x;
        const dz = pos.z - targetPos.z;
        const dist = Math.sqrt(dx * dx + dz * dz) || 1;
        const fleeX = Math.floor(pos.x + (dx / dist) * 25);
        const fleeZ = Math.floor(pos.z + (dz / dist) * 25);

        if (this.bot.pathfinder) {
            this.bot.pathfinder.setGoal(new GoalBlock(fleeX, Math.floor(pos.y), fleeZ));
        }

        // Eat while fleeing
        this._autoEat();
    }

    _tryEquipBow() {
        const inventory = this.bot.inventory.items();
        const bow = inventory.find(i => i.name === 'bow');
        const arrows = inventory.find(i => i.name === 'arrow');
        if (bow && arrows) {
            this.bot.equip(bow, 'hand').catch(() => {});
            return true;
        }
        return false;
    }

    _moveTo(position) {
        if (this.bot.pathfinder) {
            this.bot.pathfinder.setGoal(new GoalBlock(
                Math.floor(position.x),
                Math.floor(position.y),
                Math.floor(position.z)
            ));
        }
    }

    // ---- MAP DATA ----

    _broadcastMap() {
        if (!this.bot || !this.bot.entity) return;

        const pos = this.bot.entity.position;
        const mapData = this._scanArea(pos);

        this.botManager.sendToAllClients({
            type: 'bot_map',
            data: {
                botId: this.botConfig.id,
                center: { x: Math.floor(pos.x), y: Math.floor(pos.y), z: Math.floor(pos.z) },
                blocks: mapData.blocks,
                entities: mapData.entities,
                players: mapData.players
            }
        });
    }

    _scanArea(centerPos) {
        const radius = 24;
        const blocks = {};
        const entities = [];
        const players = [];

        // Scan blocks in a grid pattern (sample every 2 blocks for performance)
        for (let dx = -radius; dx <= radius; dx += 2) {
            for (let dz = -radius; dz <= radius; dz += 2) {
                for (let dy = -4; dy <= 4; dy += 2) {
                    try {
                        const bx = Math.floor(centerPos.x) + dx;
                        const by = Math.floor(centerPos.y) + dy;
                        const bz = Math.floor(centerPos.z) + dz;
                        const block = this.bot.blockAt(this.bot.entity.position.offset(dx, dy, dz));
                        if (block && block.name && block.name !== 'air') {
                            const key = `${dx + radius},${dz + radius},${dy + 4}`;
                            blocks[key] = block.name;
                        }
                    } catch (e) {}
                }
            }
        }

        // Scan entities
        for (const entity of Object.values(this.bot.entities)) {
            if (!entity || !entity.position) continue;
            const dist = centerPos.distanceTo(entity.position);
            if (dist > radius) continue;

            if (entity.type === 'player' && entity.username !== this.bot.username) {
                players.push({
                    name: entity.username || 'unknown',
                    x: Math.round(entity.position.x),
                    y: Math.round(entity.position.y),
                    z: Math.round(entity.position.z),
                    distance: Math.round(dist)
                });
            } else if (entity.type === 'mob') {
                entities.push({
                    name: entity.name || 'unknown',
                    x: Math.round(entity.position.x),
                    y: Math.round(entity.position.y),
                    z: Math.round(entity.position.z),
                    hostile: HOSTILE_MOBS.has(entity.name),
                    distance: Math.round(dist)
                });
            }
        }

        return { blocks, entities, players };
    }

    _notify(title, message) {
        this.botManager.sendToAllClients({
            type: 'notification',
            title: title,
            message: `[${this.botConfig.name}] ${message}`,
            error: false
        });
    }
}

module.exports = { SurvivalAI, HOSTILE_MOBS, FOOD_ITEMS };
