#!/bin/bash
# Remove Snap from library path
unset LD_LIBRARY_PATH
export LD_LIBRARY_PATH=/usr/lib/x86_64-linux-gnu:/lib/x86_64-linux-gnu

# Run the application
cd /home/testro/opt/Minecraft_Bot/frontend/build
./MinecraftBotManager
