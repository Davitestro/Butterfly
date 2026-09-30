# 🦋 Butterfly

**Run and manage your Minecraft bots from one simple desktop app.**

Butterfly gives you a control panel for Minecraft bots. Instead of juggling scripts and terminal windows, you use a native desktop window to create your bots, keep them organized, and start or stop everything with a single command.

---

## ✨ What You Can Do With Butterfly

- **Manage many bots in one place.** Each bot gets its own saved entry, so you can keep a whole fleet organized instead of one-off scripts.
- **Never lose your setup.** Your bots are saved automatically and are still there the next time you open the app.
- **Use a real desktop app.** The interface is a native window (built with Qt), not a browser tab or a command line.
- **Start and stop everything with one command.** `make run` launches the backend and the app together, and `make stop` shuts it all down cleanly.
- **Watch what's happening.** Live backend logs are always available, so you can see what your bots are doing and spot problems quickly.
- **Check on things at a glance.** `make status` tells you what is running, whether the port is free, and how many bots you have saved.
- **Run it your way.** Use it directly on your machine, or run everything in Docker.
- **Start fresh or keep your data.** Clean up build files without touching your bots, or wipe everything for a full reset.

## 🧭 How It Works

Butterfly has two parts that work together:

1. **The backend** runs quietly in the background, keeps your bots' information, and does the heavy lifting.
2. **The desktop app** is what you see and click. It talks to the backend live, so what you do in the window happens right away.

Because the two are separate, the interface stays light and responsive while the backend handles the work.

## 🚀 Getting Started

You need a Debian/Ubuntu-based Linux system. Then:

```bash
git clone https://github.com/Davitestro/Butterfly.git
cd Butterfly

make setup    # installs everything you need
make build    # builds the app
make run      # starts the backend and opens the app
```

When you're done:

```bash
make stop
```

### Handy Commands

| Command          | What it does                                      |
| ---------------- | ------------------------------------------------- |
| `make run`       | Start everything and show live logs               |
| `make stop`      | Stop everything and free the port                 |
| `make status`    | See what's running and how many bots are saved    |
| `make logs`      | Follow the backend logs                           |
| `make clean`     | Remove build files, **keep your bots**            |
| `make clean-all` | Remove everything, **including your bots**        |
| `make rebuild`   | Clean and build again                             |

### Prefer Docker?

```bash
docker-compose up --build
```

The desktop window is shown through your display (X11). If it doesn't appear, run `xhost +local:docker` first.

## 📁 What's Inside

```
Butterfly/
├── backend/     # The server that runs and stores your bots
├── frontend/    # The desktop app (Qt / C++)
├── Makefile     # Easy setup, build, run and stop commands
├── launch.sh    # Opens the desktop app with a clean environment
└── Dockerfile.* / docker-compose.yml   # Docker support
```

## ⚠️ Good to Know

- `launch.sh` currently points to a fixed folder (`/home/testro/opt/Minecraft_Bot/frontend/build`). Change it to wherever you cloned the project.
- On newer Ubuntu versions (22.04+), `make setup` may fail on the `qt5-default` package. Install `qtbase5-dev` and the other Qt5 packages by hand if that happens.
- Your bots live in `backend/bots.json`. Back it up if they matter to you.

## 📜 License and Credit

Butterfly is released under the **MIT License**, see the [LICENSE](LICENSE) file.

In plain words: you are free to use, change and share this project, **but you must keep the original copyright notice and license text with it**. That means anyone who copies or builds on Butterfly has to credit **Davitestro** as the original author.

Copyright © 2026 Davitestro
