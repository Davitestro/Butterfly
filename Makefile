# Minecraft Bot Manager Makefile

.PHONY: help setup build run stop clean status rebuild

help:
	@echo "Minecraft Bot Manager Commands:"
	@echo ""
	@echo "  make setup   - Install dependencies"
	@echo "  make build   - Build the application"
	@echo "  make run     - Start the application"
	@echo "  make stop    - Stop the application"
	@echo "  make status  - Check running processes"
	@echo "  make clean   - Clean build files"
	@echo "  make rebuild - Clean and rebuild"

setup:
	@echo "Installing dependencies..."
	sudo apt-get update
	sudo apt-get install -y qt6-base-dev qt6-tools-dev qt6-tools-dev-tools qt6-websockets-dev libcurl4-openssl-dev build-essential cmake patchelf nodejs npm
	cd backend && npm install
	@echo "Setup complete!"

build:
	@echo "Building backend..."
	cd backend && npm install
	@echo "Building frontend..."
	mkdir -p frontend/build
	cd frontend/build && cmake .. && make
	cd frontend/build && patchelf --replace-needed libpthread.so.0 libpthread.so.0 MinecraftBotManager 2>/dev/null || true
	@echo "Build complete!"

run:
	@echo "Starting backend..."
	@cd backend && nohup npm start > backend.log 2>&1 &
	@sleep 3
	@echo "Starting frontend..."
	@./launch.sh &
	@echo "Application started!"
	@echo "Check backend.log for backend output"

stop:
	@echo "Stopping all processes..."
	pkill -f "node server.js" || true
	pkill -f "MinecraftBotManager" || true
	@echo "Stopped"

status:
	@if pgrep -f "node server.js" > /dev/null; then \
		echo "Backend: RUNNING"; \
	else \
		echo "Backend: STOPPED"; \
	fi
	@if pgrep -f "MinecraftBotManager" > /dev/null; then \
		echo "Frontend: RUNNING"; \
	else \
		echo "Frontend: STOPPED"; \
	fi

clean:
	@echo "Cleaning..."
	rm -rf frontend/build
	rm -rf backend/node_modules
	@echo "Clean complete!"

rebuild: clean build
