.PHONY: help setup build run stop status clean rebuild logs

help:
	@echo "Minecraft Bot Manager Commands:"
	@echo ""
	@echo "  make setup   - Install dependencies"
	@echo "  make build   - Build the application"
	@echo "  make run     - Start the application (with logs)"
	@echo "  make stop    - Stop the application"
	@echo "  make status  - Check running processes"
	@echo "  make clean   - Clean build files (keeps bot data)"
	@echo "  make clean-all - Clean everything including bot data"
	@echo "  make rebuild - Clean and rebuild"
	@echo "  make logs    - Show backend logs"

setup:
	@echo "Installing dependencies..."
	@sudo apt-get update
	@sudo apt-get install -y nodejs npm cmake build-essential qt5-default qtbase5-dev qt5-qmake libqt5websockets5-dev
	@cd backend && npm install
	@echo "Setup complete!"

build:
	@echo "Building backend..."
	@cd backend && npm install
	@echo "Building frontend..."
	@mkdir -p frontend/build
	@cd frontend/build && cmake .. && make
	@cd frontend/build && patchelf --replace-needed libpthread.so.0 libpthread.so.0 MinecraftBotManager 2>/dev/null || true
	@echo "Build complete!"

run:
	@echo "========================================"
	@echo "🚀 Starting Minecraft Bot Manager"
	@echo "========================================"
	@echo ""
	@echo "Starting backend server..."
	@cd backend && nohup npm start > backend.log 2>&1 &
	@echo "⏳ Waiting for backend to start..."
	@sleep 3
	@echo "✅ Backend running on port 3000"
	@echo ""
	@echo "Starting frontend..."
	@./launch.sh &
	@echo "✅ Frontend started"
	@echo ""
	@echo "========================================"
	@echo "📊 Application is running!"
	@echo "========================================"
	@echo ""
	@echo "Showing backend logs (Ctrl+C to stop viewing):"
	@echo ""
	@tail -f backend/backend.log

stop:
	@echo "Stopping all processes..."
	@pkill -f "node server.js" || true
	@pkill -f MinecraftBotManager || true
	@sudo fuser -k 3000/tcp 2>/dev/null || true
	@echo "All processes stopped"

status:
	@echo "Checking running processes..."
	@echo ""
	@echo "Backend:"
	@ps aux | grep "node server.js" | grep -v grep || echo "  ❌ Not running"
	@echo ""
	@echo "Frontend:"
	@ps aux | grep MinecraftBotManager | grep -v grep || echo "  ❌ Not running"
	@echo ""
	@echo "Port 3000:"
	@sudo lsof -i :3000 || echo "  Port 3000 is free"
	@echo ""
	@echo "Saved bots:"
	@if [ -f backend/bots.json ]; then cat backend/bots.json | grep -c '"id"' || echo "  0 bots"; else echo "  No bots file"; fi

logs:
	@echo "Showing backend logs (Ctrl+C to exit):"
	@tail -f backend/backend.log

clean:
	@echo "Cleaning build files (keeping bot data)..."
	@rm -rf frontend/build
	@rm -rf backend/node_modules
	@rm -f backend/backend.log
	@rm -f backend/nohup.out
	@echo "Clean complete! Bot data preserved in backend/bots.json"

clean-all:
	@echo "Cleaning EVERYTHING (including bot data)..."
	@rm -rf frontend/build
	@rm -rf backend/node_modules
	@rm -f backend/bots.json
	@rm -f backend/backend.log
	@rm -f backend/nohup.out
	@echo "Complete clean done!"

rebuild: clean build