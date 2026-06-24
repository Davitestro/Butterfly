#!/bin/bash
# Completely remove Snap from the environment
unset LD_LIBRARY_PATH
unset SNAP
unset SNAP_ARCH
unset SNAP_COMMON
unset SNAP_DATA
unset SNAP_INSTANCE_KEY
unset SNAP_INSTANCE_NAME
unset SNAP_LIBRARY_PATH
unset SNAP_NAME
unset SNAP_REVISION
unset SNAP_USER_COMMON
unset SNAP_USER_DATA
unset SNAP_VERSION

# Set library path to system only
export LD_LIBRARY_PATH=/usr/lib/x86_64-linux-gnu:/lib/x86_64-linux-gnu

# Preload system libc and pthread
export LD_PRELOAD=/lib/x86_64-linux-gnu/libc.so.6:/lib/x86_64-linux-gnu/libpthread.so.0

# Run the application
cd /home/testro/opt/Minecraft_Bot/frontend/build
exec ./MinecraftBotManager "$@"
