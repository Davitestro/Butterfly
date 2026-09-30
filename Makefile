SHELL := /bin/sh

.DEFAULT_GOAL := help

COMPOSE ?= docker compose
COMPOSE_FILE ?= docker-compose.yml
PROJECT ?= butterfly
SERVICE ?=
TAIL ?= 100
EXEC_SERVICE ?= backend
CMD ?= sh

DC := $(COMPOSE) -p $(PROJECT) -f $(COMPOSE_FILE)

.PHONY: all help setup check build up up-build run start stop down restart rebuild \
	logs status ps config shell exec clean clean-all docker-check docker-build \
	docker-up docker-up-build docker-start docker-stop docker-down docker-restart \
	docker-rebuild docker-logs docker-status docker-ps docker-config docker-shell \
	docker-exec docker-clean docker-clean-all

all: build

help:
	@printf '%s\n' 'Butterfly Docker commands:'
	@printf '%s\n' '' \
		'  make setup          Check that Docker and Compose are available' \
		'  make build          Build all Docker images' \
		'  make up             Start the stack in the background' \
		'  make up-build       Build images and start the stack' \
		'  make run            Alias for up-build' \
		'  make start          Start already-created containers' \
		'  make stop           Stop containers without removing them' \
		'  make down           Stop and remove the stack' \
		'  make restart        Restart the stack' \
		'  make rebuild        Rebuild images and recreate the stack' \
		'  make logs           Follow logs (SERVICE=backend|frontend)' \
		'  make status         Show container status and health' \
		'  make config         Validate and print the Compose configuration' \
		'  make shell          Open a shell (EXEC_SERVICE=backend|frontend)' \
		'  make exec CMD=...   Run a command in a service container' \
		'  make clean          Remove containers and local images' \
		'  make clean-all      Also remove Compose volumes' \
		'' \
		'Options:' \
		'  SERVICE=backend|frontend  Limit build/start/log commands to one service' \
		'  PROJECT=name              Use a different Compose project name' \
		'  TAIL=100                 Number of log lines to show before following'

setup: docker-check

check: docker-check

docker-check:
	@command -v docker >/dev/null 2>&1 || { printf '%s\n' 'Docker is required but was not found.' >&2; exit 1; }
	@docker compose version >/dev/null 2>&1 || { printf '%s\n' 'Docker Compose v2 is required but was not found.' >&2; exit 1; }
	@printf '%s\n' 'Docker and Docker Compose are available.'

build: docker-build

docker-build: docker-check
	@$(DC) build $(SERVICE)

up: docker-up

docker-up: docker-check
	@$(DC) up -d --remove-orphans $(SERVICE)

up-build: docker-up-build

docker-up-build: docker-check
	@$(DC) up -d --build --remove-orphans $(SERVICE)

run: up-build

start: docker-start

docker-start: docker-check
	@$(DC) start $(SERVICE)

stop: docker-stop

docker-stop: docker-check
	@$(DC) stop $(SERVICE)

down: docker-down

docker-down: docker-check
	@$(DC) down --remove-orphans

restart: docker-restart

docker-restart: docker-check
	@$(DC) restart $(SERVICE)

rebuild: docker-rebuild

docker-rebuild: docker-check
	@$(DC) up -d --build --force-recreate --remove-orphans $(SERVICE)

logs: docker-logs

docker-logs: docker-check
	@$(DC) logs -f --tail=$(TAIL) $(SERVICE)

status: docker-status

docker-status: docker-check
	@$(DC) ps --all

ps: docker-ps

docker-ps: docker-status

config: docker-config

docker-config: docker-check
	@$(DC) config

shell: docker-shell

docker-shell: docker-check
	@$(DC) exec $(EXEC_SERVICE) sh

exec: docker-exec

docker-exec: docker-check
	@$(DC) exec $(EXEC_SERVICE) $(CMD)

clean: docker-clean

docker-clean: docker-check
	@$(DC) down --remove-orphans --rmi all

clean-all: docker-clean-all

docker-clean-all: docker-check
	@$(DC) down --remove-orphans --rmi all --volumes
