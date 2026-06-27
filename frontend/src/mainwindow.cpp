#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QJsonDocument>
#include <QUrl>
#include <QStatusBar>
#include <QTableWidget>
#include <QHeaderView>
#include <QTextEdit>
#include <QTabWidget>
#include <QGroupBox>
#include <QProgressBar>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QScrollArea>
#include <QInputDialog>
#include <QJsonObject>
#include <QJsonArray>
#include <QComboBox>
#include <QCheckBox>

class BotDialog : public QDialog {
public:
    BotDialog(QWidget* parent = nullptr, const BotData* existing = nullptr, const QJsonObject* fullData = nullptr) : QDialog(parent) {
        setWindowTitle(existing ? "Edit Bot" : "Add Bot");
        resize(400, 400);
        
        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        QFormLayout* form = new QFormLayout();
        
        nameEdit = new QLineEdit(this);
        form->addRow("Bot Name:", nameEdit);
        
        serverEdit = new QLineEdit(this);
        form->addRow("Server:", serverEdit);
        
        portSpin = new QSpinBox(this);
        portSpin->setRange(1, 65535);
        portSpin->setValue(25565);
        form->addRow("Port:", portSpin);
        
        usernameEdit = new QLineEdit(this);
        form->addRow("Username:", usernameEdit);
        
        passwordEdit = new QLineEdit(this);
        passwordEdit->setEchoMode(QLineEdit::Password);
        passwordEdit->setPlaceholderText("Leave empty for offline mode");
        form->addRow("Password:", passwordEdit);
        
        if (existing) {
            infoLabel = new QLabel(this);
            infoLabel->setStyleSheet("color: gray; font-size: 10px;");
            if (fullData) {
                QString created = fullData->value("createdAt").toString();
                QString modified = fullData->value("lastModified").toString();
                if (!created.isEmpty()) {
                    infoLabel->setText("Created: " + created.left(16).replace("T", " ") + 
                                      "\nModified: " + modified.left(16).replace("T", " "));
                }
            }
            form->addRow("", infoLabel);
        }
        
        mainLayout->addLayout(form);
        
        QDialogButtonBox* buttons = new QDialogButtonBox(
            QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
            Qt::Horizontal,
            this
        );
        mainLayout->addWidget(buttons);
        
        connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
        
        if (existing) {
            currentData = *existing;
            nameEdit->setText(existing->name);
            serverEdit->setText(existing->server);
            portSpin->setValue(existing->port);
            usernameEdit->setText(existing->username);
            passwordEdit->setText(existing->password);
        }
    }
    
    BotData getData() const {
        BotData data = currentData;
        data.name = nameEdit->text().trimmed();
        data.server = serverEdit->text().trimmed();
        data.port = portSpin->value();
        data.username = usernameEdit->text().trimmed();
        data.password = passwordEdit->text();
        return data;
    }
    
private:
    QLineEdit* nameEdit;
    QLineEdit* serverEdit;
    QSpinBox* portSpin;
    QLineEdit* usernameEdit;
    QLineEdit* passwordEdit;
    QLabel* infoLabel;
    BotData currentData;
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setupUI();
    setupWebSocket();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupUI() {
    setWindowTitle("Minecraft Bot Manager");
    resize(1300, 750);
    
    QWidget* central = new QWidget(this);
    setCentralWidget(central);
    QHBoxLayout* mainLayout = new QHBoxLayout(central);
    
    // === LEFT PANEL - Bot List ===
    QWidget* leftPanel = new QWidget(this);
    leftPanel->setFixedWidth(280);
    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);
    
    QLabel* listLabel = new QLabel("🤖 Bots", this);
    listLabel->setStyleSheet("font-size: 14px; font-weight: bold;");
    leftLayout->addWidget(listLabel);
    
    botList = new QListWidget(this);
    botList->setMinimumHeight(400);
    connect(botList, &QListWidget::itemClicked, this, &MainWindow::onBotSelected);
    connect(botList, &QListWidget::itemDoubleClicked, this, &MainWindow::onBotDoubleClicked);
    leftLayout->addWidget(botList);
    
    QGridLayout* botButtons = new QGridLayout();
    addButton = new QPushButton("➕ Add", this);
    deleteButton = new QPushButton("🗑 Delete", this);
    editButton = new QPushButton("✏️ Edit", this);
    startButton = new QPushButton("▶ Start", this);
    stopButton = new QPushButton("⏹ Stop", this);
    
    connect(addButton, &QPushButton::clicked, this, &MainWindow::onAddBot);
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::onDeleteBot);
    connect(editButton, &QPushButton::clicked, this, &MainWindow::onEditBot);
    connect(startButton, &QPushButton::clicked, this, &MainWindow::onStartBot);
    connect(stopButton, &QPushButton::clicked, this, &MainWindow::onStopBot);
    
    botButtons->addWidget(addButton, 0, 0);
    botButtons->addWidget(deleteButton, 0, 1);
    botButtons->addWidget(editButton, 0, 2);
    botButtons->addWidget(startButton, 1, 0);
    botButtons->addWidget(stopButton, 1, 1);
    leftLayout->addLayout(botButtons);
    
    leftLayout->addStretch();
    mainLayout->addWidget(leftPanel);
    
    // === CENTER PANEL ===
    QWidget* centerPanel = new QWidget(this);
    QVBoxLayout* centerLayout = new QVBoxLayout(centerPanel);
    
    // Bot info header
    QFrame* headerFrame = new QFrame(this);
    headerFrame->setFrameStyle(QFrame::StyledPanel);
    QHBoxLayout* headerLayout = new QHBoxLayout(headerFrame);
    
    botNameLabel = new QLabel("No bot selected", this);
    botNameLabel->setStyleSheet("font-size: 16px; font-weight: bold;");
    headerLayout->addWidget(botNameLabel);
    
    botStatusLabel = new QLabel("⚪ Offline", this);
    botStatusLabel->setStyleSheet("font-size: 14px;");
    headerLayout->addWidget(botStatusLabel);
    headerLayout->addStretch();
    
    botServerLabel = new QLabel("", this);
    headerLayout->addWidget(botServerLabel);
    botUsernameLabel = new QLabel("", this);
    headerLayout->addWidget(botUsernameLabel);
    
    centerLayout->addWidget(headerFrame);
    
    // Task progress bar
    QHBoxLayout* taskLayout = new QHBoxLayout();
    taskStatusLabel = new QLabel("No active task", this);
    taskStatusLabel->setStyleSheet("font-size: 12px; color: gray;");
    taskLayout->addWidget(taskStatusLabel);
    taskProgressBar = new QProgressBar(this);
    taskProgressBar->setRange(0, 100);
    taskProgressBar->setValue(0);
    taskProgressBar->setFixedWidth(200);
    taskLayout->addWidget(taskProgressBar);
    taskLayout->addStretch();
    centerLayout->addLayout(taskLayout);
    
    // Tabs
    centralTabs = new QTabWidget(this);
    
    // === Tab 1: Bot Info ===
    infoWidget = new QWidget(this);
    QGridLayout* infoLayout = new QGridLayout(infoWidget);
    
    QLabel* healthLabel = new QLabel("❤️ Health:", this);
    healthBar = new QProgressBar(this);
    healthBar->setRange(0, 20);
    healthBar->setValue(20);
    healthBar->setStyleSheet("QProgressBar::chunk { background-color: #00ff00; }");
    infoLayout->addWidget(healthLabel, 0, 0);
    infoLayout->addWidget(healthBar, 0, 1);
    
    QLabel* foodLabel = new QLabel("🍖 Food:", this);
    foodBar = new QProgressBar(this);
    foodBar->setRange(0, 20);
    foodBar->setValue(20);
    foodBar->setStyleSheet("QProgressBar::chunk { background-color: #ff8800; }");
    infoLayout->addWidget(foodLabel, 1, 0);
    infoLayout->addWidget(foodBar, 1, 1);
    
    QLabel* expTextLabel = new QLabel("⭐ Experience:", this);
    expLabel = new QLabel("0", this);
    infoLayout->addWidget(expTextLabel, 2, 0);
    infoLayout->addWidget(expLabel, 2, 1);
    
    QLabel* levelTextLabel = new QLabel("📈 Level:", this);
    levelLabel = new QLabel("0", this);
    infoLayout->addWidget(levelTextLabel, 3, 0);
    infoLayout->addWidget(levelLabel, 3, 1);
    
    QLabel* coordsTextLabel = new QLabel("📍 Position:", this);
    coordsLabel = new QLabel("X: 0, Y: 0, Z: 0", this);
    infoLayout->addWidget(coordsTextLabel, 4, 0);
    infoLayout->addWidget(coordsLabel, 4, 1);
    
    QLabel* dimTextLabel = new QLabel("🌍 Dimension:", this);
    dimensionLabel = new QLabel("Overworld", this);
    infoLayout->addWidget(dimTextLabel, 5, 0);
    infoLayout->addWidget(dimensionLabel, 5, 1);
    
    QLabel* gmTextLabel = new QLabel("🎮 Gamemode:", this);
    gamemodeLabel = new QLabel("Survival", this);
    infoLayout->addWidget(gmTextLabel, 6, 0);
    infoLayout->addWidget(gamemodeLabel, 6, 1);
    
    infoLayout->setRowStretch(7, 1);
    centralTabs->addTab(infoWidget, "📊 Info");
    
    // === Tab 2: Inventory ===
    inventoryWidget = new QWidget(this);
    QVBoxLayout* invLayout = new QVBoxLayout(inventoryWidget);
    
    inventoryTable = new QTableWidget(this);
    inventoryTable->setColumnCount(3);
    inventoryTable->setHorizontalHeaderLabels({"Slot", "Item", "Count"});
    inventoryTable->horizontalHeader()->setStretchLastSection(true);
    inventoryTable->setAlternatingRowColors(true);
    invLayout->addWidget(inventoryTable);
    
    centralTabs->addTab(inventoryWidget, "🎒 Inventory");
    
    // === Tab 3: Tasks ===
    tasksWidget = new QWidget(this);
    QVBoxLayout* tasksLayout = new QVBoxLayout(tasksWidget);
    
    tasksTable = new QTableWidget(this);
    tasksTable->setColumnCount(4);
    tasksTable->setHorizontalHeaderLabels({"Task", "Status", "Progress", "Time"});
    tasksTable->horizontalHeader()->setStretchLastSection(true);
    tasksTable->setAlternatingRowColors(true);
    tasksLayout->addWidget(tasksTable);
    
    refreshTasksButton = new QPushButton("🔄 Refresh Tasks", this);
    connect(refreshTasksButton, &QPushButton::clicked, this, &MainWindow::onRefreshTasks);
    tasksLayout->addWidget(refreshTasksButton);
    
    centralTabs->addTab(tasksWidget, "📋 Tasks");
    
    // === Tab 4: Commands ===
    commandsWidget = new QWidget(this);
    QVBoxLayout* cmdLayout = new QVBoxLayout(commandsWidget);
    
    // Command output
    commandOutput = new QTextEdit(this);
    commandOutput->setReadOnly(true);
    commandOutput->setFont(QFont("Monospace", 10));
    commandOutput->setMinimumHeight(150);
    cmdLayout->addWidget(commandOutput);
    
    // Command grid
    QGridLayout* cmdGrid = new QGridLayout();
    
    // Row 1: Move To
    cmdGrid->addWidget(new QLabel("Move To:", this), 0, 0);
    moveToButton = new QPushButton("🎯 Move", this);
    connect(moveToButton, &QPushButton::clicked, this, &MainWindow::onMoveTo);
    cmdGrid->addWidget(moveToButton, 0, 1);
    
    // Row 2: Follow Player
    cmdGrid->addWidget(new QLabel("Follow:", this), 1, 0);
    followInput = new QLineEdit(this);
    followInput->setPlaceholderText("Player name");
    cmdGrid->addWidget(followInput, 1, 1);
    followButton = new QPushButton("👤 Follow", this);
    connect(followButton, &QPushButton::clicked, this, &MainWindow::onFollowPlayer);
    cmdGrid->addWidget(followButton, 1, 2);
    
    // Row 3: Guard
    cmdGrid->addWidget(new QLabel("Guard:", this), 2, 0);
    guardInput = new QLineEdit(this);
    guardInput->setPlaceholderText("Radius");
    guardInput->setText("10");
    cmdGrid->addWidget(guardInput, 2, 1);
    guardButton = new QPushButton("🛡️ Guard", this);
    connect(guardButton, &QPushButton::clicked, this, &MainWindow::onGuardArea);
    cmdGrid->addWidget(guardButton, 2, 2);
    
    // Row 4: Collect
    cmdGrid->addWidget(new QLabel("Collect:", this), 3, 0);
    collectInput = new QLineEdit(this);
    collectInput->setPlaceholderText("Resource name");
    cmdGrid->addWidget(collectInput, 3, 1);
    collectButton = new QPushButton("⛏️ Collect", this);
    connect(collectButton, &QPushButton::clicked, this, &MainWindow::onCollectResources);
    cmdGrid->addWidget(collectButton, 3, 2);
    
    // Row 5: Drop
    cmdGrid->addWidget(new QLabel("Drop:", this), 4, 0);
    dropInput = new QLineEdit(this);
    dropInput->setPlaceholderText("Item [count]");
    cmdGrid->addWidget(dropInput, 4, 1);
    dropButton = new QPushButton("🗑️ Drop", this);
    connect(dropButton, &QPushButton::clicked, this, &MainWindow::onDropItem);
    cmdGrid->addWidget(dropButton, 4, 2);
    
    // Row 6: Hunt Animals
    cmdGrid->addWidget(new QLabel("Hunt:", this), 5, 0);
    huntInput = new QLineEdit(this);
    huntInput->setPlaceholderText("Animal name");
    cmdGrid->addWidget(huntInput, 5, 1);
    huntAnimalsButton = new QPushButton("🐄 Hunt", this);
    connect(huntAnimalsButton, &QPushButton::clicked, this, &MainWindow::onHuntAnimals);
    cmdGrid->addWidget(huntAnimalsButton, 5, 2);
    
    // Row 7: Hunt Players
    cmdGrid->addWidget(new QLabel("Hunt Player:", this), 6, 0);
    huntPlayerInput = new QLineEdit(this);
    huntPlayerInput->setPlaceholderText("Player name (optional)");
    cmdGrid->addWidget(huntPlayerInput, 6, 1);
    huntPlayersButton = new QPushButton("⚔️ Hunt", this);
    connect(huntPlayersButton, &QPushButton::clicked, this, &MainWindow::onHuntPlayers);
    cmdGrid->addWidget(huntPlayersButton, 6, 2);
    
    // Row 8: Stop Action
    cmdGrid->addWidget(new QLabel("", this), 7, 0);
    stopActionButton = new QPushButton("🛑 STOP ALL ACTIONS", this);
    stopActionButton->setStyleSheet("background-color: #ff4444; color: white; font-weight: bold;");
    connect(stopActionButton, &QPushButton::clicked, this, &MainWindow::onStopAction);
    cmdGrid->addWidget(stopActionButton, 7, 1, 1, 2);
    
    cmdLayout->addLayout(cmdGrid);
    
    // Custom command
    QHBoxLayout* customCmdLayout = new QHBoxLayout();
    commandInput = new QLineEdit(this);
    commandInput->setPlaceholderText("Type custom command...");
    sendButton = new QPushButton("Send", this);
    
    connect(sendButton, &QPushButton::clicked, this, &MainWindow::onSendCommand);
    connect(commandInput, &QLineEdit::returnPressed, this, &MainWindow::onSendCommand);
    
    customCmdLayout->addWidget(commandInput);
    customCmdLayout->addWidget(sendButton);
    cmdLayout->addLayout(customCmdLayout);
    
    centralTabs->addTab(commandsWidget, "💬 Commands");
    
    centerLayout->addWidget(centralTabs);
    mainLayout->addWidget(centerPanel);
    
    mainLayout->setStretch(0, 0);
    mainLayout->setStretch(1, 1);
    
    statusBar()->showMessage("Ready");
    clearBotInfo();
}

void MainWindow::showNotification(const QString& title, const QString& message, bool isError) {
    QMessageBox::Icon icon = isError ? QMessageBox::Critical : QMessageBox::Information;
    QMessageBox msgBox(icon, title, message, QMessageBox::Ok, this);
    msgBox.exec();
}

void MainWindow::clearBotInfo() {
    botNameLabel->setText("No bot selected");
    botStatusLabel->setText("⚪ Offline");
    botServerLabel->setText("");
    botUsernameLabel->setText("");
    healthBar->setValue(20);
    foodBar->setValue(20);
    expLabel->setText("0");
    levelLabel->setText("0");
    coordsLabel->setText("X: 0, Y: 0, Z: 0");
    dimensionLabel->setText("Overworld");
    gamemodeLabel->setText("Survival");
    inventoryTable->setRowCount(0);
    tasksTable->setRowCount(0);
    commandOutput->clear();
    taskStatusLabel->setText("No active task");
    taskProgressBar->setValue(0);
    currentBotId = "";
}

void MainWindow::updateBotInfo(const QJsonObject& botData) {
    if (!botData.isEmpty()) {
        botNameLabel->setText(botData["name"].toString());
        botServerLabel->setText("🌐 " + botData["server"].toString());
        botUsernameLabel->setText("👤 " + botData["username"].toString());
        
        QString status = botData["status"].toString();
        QString statusText;
        if (status == "online") statusText = "🟢 Online";
        else if (status == "connecting") statusText = "🟡 Connecting...";
        else if (status == "error") statusText = "🔴 Error";
        else statusText = "⚪ Offline";
        botStatusLabel->setText(statusText);
    }
}

void MainWindow::updateBotStats(const QJsonObject& stats) {
    if (stats.isEmpty()) return;
    
    double health = stats["health"].toDouble(20.0);
    int food = stats["food"].toInt(20);
    int exp = stats["experience"].toInt(0);
    int level = stats["level"].toInt(0);
    QJsonObject pos = stats["position"].toObject();
    double x = pos["x"].toDouble(0);
    double y = pos["y"].toDouble(0);
    double z = pos["z"].toDouble(0);
    
    healthBar->setValue((int)health);
    foodBar->setValue(food);
    expLabel->setText(QString::number(exp));
    levelLabel->setText(QString::number(level));
    coordsLabel->setText(QString("X: %1, Y: %2, Z: %3").arg(x, 0, 'f', 1).arg(y, 0, 'f', 1).arg(z, 0, 'f', 1));
    
    if (stats.contains("dimension")) {
        dimensionLabel->setText(stats["dimension"].toString());
    }
    if (stats.contains("gamemode")) {
        gamemodeLabel->setText(stats["gamemode"].toString());
    }
    
    // Update task progress if available
    if (stats.contains("task")) {
        QJsonObject task = stats["task"].toObject();
        QString taskName = task["name"].toString();
        int progress = task["progress"].toInt(0);
        taskStatusLabel->setText("📋 " + taskName);
        taskProgressBar->setValue(progress);
    }
}

void MainWindow::updateInventory(const QJsonArray& inventory) {
    inventoryTable->setRowCount(inventory.size());
    
    for (int i = 0; i < inventory.size(); ++i) {
        QJsonObject item = inventory[i].toObject();
        
        QTableWidgetItem* slotItem = new QTableWidgetItem(QString::number(item["slot"].toInt()));
        inventoryTable->setItem(i, 0, slotItem);
        
        QTableWidgetItem* nameItem = new QTableWidgetItem(item["name"].toString());
        inventoryTable->setItem(i, 1, nameItem);
        
        QTableWidgetItem* countItem = new QTableWidgetItem(QString::number(item["count"].toInt()));
        inventoryTable->setItem(i, 2, countItem);
    }
}

void MainWindow::updateTasks(const QJsonArray& tasks) {
    tasksTable->setRowCount(tasks.size());
    
    for (int i = 0; i < tasks.size(); ++i) {
        QJsonObject task = tasks[i].toObject();
        
        QTableWidgetItem* nameItem = new QTableWidgetItem(task["name"].toString());
        tasksTable->setItem(i, 0, nameItem);
        
        bool completed = task["completed"].toBool(false);
        QTableWidgetItem* statusItem = new QTableWidgetItem(completed ? "✅ Done" : "⏳ In Progress");
        tasksTable->setItem(i, 1, statusItem);
        
        int progress = task["progress"].toInt(0);
        QTableWidgetItem* progressItem = new QTableWidgetItem(QString::number(progress) + "%");
        tasksTable->setItem(i, 2, progressItem);
        
        qint64 started = task["started"].toVariant().toLongLong();
        QString timeStr = QDateTime::fromMSecsSinceEpoch(started).toString("hh:mm:ss");
        if (completed) {
            qint64 completedAt = task["completedAt"].toVariant().toLongLong();
            timeStr += " - " + QDateTime::fromMSecsSinceEpoch(completedAt).toString("hh:mm:ss");
        }
        QTableWidgetItem* timeItem = new QTableWidgetItem(timeStr);
        tasksTable->setItem(i, 3, timeItem);
    }
}

void MainWindow::updateTaskProgress(const QJsonObject& taskData) {
    QString taskName = taskData["currentTask"].toString();
    int progress = taskData["progress"].toInt(0);
    int total = taskData["total"].toInt(0);
    
    if (!taskName.isEmpty()) {
        taskStatusLabel->setText("📋 " + taskName);
        taskProgressBar->setValue(progress);
    } else {
        taskStatusLabel->setText("No active task");
        taskProgressBar->setValue(0);
    }
}

void MainWindow::addCommandToOutput(const QString& command, const QString& response) {
    commandOutput->append("> " + command);
    commandOutput->append("  " + response);
    commandOutput->append("");
}

void MainWindow::setupWebSocket() {
    webSocket = new QWebSocket();
    reconnectTimer = new QTimer(this);
    reconnectTimer->setInterval(3000);
    connect(reconnectTimer, &QTimer::timeout, this, &MainWindow::reconnectWebSocket);
    
    connect(webSocket, &QWebSocket::connected, this, &MainWindow::onWebSocketConnected);
    connect(webSocket, &QWebSocket::disconnected, this, &MainWindow::onWebSocketDisconnected);
    connect(webSocket, &QWebSocket::textMessageReceived, this, &MainWindow::onWebSocketMessage);
    
    reconnectWebSocket();
}

void MainWindow::reconnectWebSocket() {
    statusBar()->showMessage("Connecting to backend...");
    webSocket->open(QUrl("ws://localhost:3000"));
}

void MainWindow::onWebSocketConnected() {
    reconnectTimer->stop();
    statusBar()->showMessage("Connected to backend");
    QJsonObject empty;
    sendMessage("get_bots", empty);
}

void MainWindow::onWebSocketDisconnected() {
    statusBar()->showMessage("Disconnected from backend, reconnecting...");
    reconnectTimer->start();
    showNotification("Connection Lost", "Backend connection lost. Reconnecting...", true);
}

void MainWindow::onWebSocketMessage(const QString& message) {
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (!doc.isObject()) return;
    
    QJsonObject obj = doc.object();
    QString type = obj["type"].toString();
    
    if (type == "bots_update") {
        updateBotList(obj["data"].toArray());
    } else if (type == "bot_info") {
        updateBotInfo(obj["data"].toObject());
    } else if (type == "bot_stats") {
        updateBotStats(obj["data"].toObject());
    } else if (type == "bot_inventory") {
        updateInventory(obj["data"].toArray());
    } else if (type == "bot_tasks") {
        updateTasks(obj["data"].toArray());
    } else if (type == "task_progress") {
        updateTaskProgress(obj["data"].toObject());
    } else if (type == "bot_command_response") {
        QString command = obj["command"].toString();
        QString response = obj["response"].toString();
        addCommandToOutput(command, response);
    } else if (type == "notification") {
        QString title = obj["title"].toString();
        QString msg = obj["message"].toString();
        bool isError = obj["error"].toBool(false);
        showNotification(title, msg, isError);
    }
}

void MainWindow::updateBotList(const QJsonArray& botsArray) {
    botList->clear();
    botDataCache.clear();
    
    for (const QJsonValue& value : botsArray) {
        QJsonObject botObj = value.toObject();
        BotData bot;
        bot.id = botObj["id"].toString();
        bot.name = botObj["name"].toString();
        bot.server = botObj["server"].toString();
        bot.port = botObj["port"].toInt();
        bot.username = botObj["username"].toString();
        bot.password = botObj["password"].toString();
        bot.status = botObj["status"].toString();
        
        botDataCache[bot.id] = botObj;
        
        QString statusIcon;
        if (bot.status == "online") statusIcon = "🟢 ";
        else if (bot.status == "connecting") statusIcon = "🟡 ";
        else if (bot.status == "error") statusIcon = "🔴 ";
        else statusIcon = "⚪ ";
        
        QListWidgetItem* item = new QListWidgetItem(
            statusIcon + bot.name
        );
        item->setData(Qt::UserRole, bot.id);
        item->setData(Qt::UserRole + 1, bot.status);
        item->setToolTip("Server: " + bot.server + ":" + QString::number(bot.port) + 
                        "\nUsername: " + bot.username +
                        "\nStatus: " + bot.status);
        botList->addItem(item);
    }
    
    statusBar()->showMessage(QString("Loaded %1 bots").arg(botsArray.size()));
}

void MainWindow::sendMessage(const QString& type, const QJsonObject& data) {
    QJsonObject message;
    message["type"] = type;
    for (auto it = data.begin(); it != data.end(); ++it) {
        message[it.key()] = it.value();
    }
    webSocket->sendTextMessage(QJsonDocument(message).toJson());
}

void MainWindow::sendMessage(const QString& type) {
    QJsonObject message;
    message["type"] = type;
    webSocket->sendTextMessage(QJsonDocument(message).toJson());
}

void MainWindow::showBotDialog(const BotData* existing) {
    QJsonObject fullData;
    if (existing && botDataCache.contains(existing->id)) {
        fullData = botDataCache[existing->id];
    }
    
    BotDialog dialog(this, existing, existing ? &fullData : nullptr);
    if (dialog.exec() == QDialog::Accepted) {
        BotData data = dialog.getData();
        QJsonObject botData;
        botData["name"] = data.name;
        botData["server"] = data.server;
        botData["port"] = data.port;
        botData["username"] = data.username;
        botData["password"] = data.password;
        
        if (existing) {
            botData["id"] = existing->id;
            sendMessage("update_bot", botData);
            showNotification("Bot Updated", "Bot settings saved successfully!", false);
        } else {
            sendMessage("add_bot", botData);
            showNotification("Bot Added", "New bot created successfully!", false);
        }
    }
}

void MainWindow::onAddBot() {
    showBotDialog(nullptr);
}

void MainWindow::onEditBot() {
    QListWidgetItem* item = botList->currentItem();
    if (!item) {
        showNotification("No Selection", "Please select a bot to edit", true);
        return;
    }
    
    QString botId = item->data(Qt::UserRole).toString();
    BotData dummy;
    dummy.id = botId;
    dummy.name = item->text();
    if (dummy.name.startsWith("🟢 ") || dummy.name.startsWith("🟡 ") || 
        dummy.name.startsWith("🔴 ") || dummy.name.startsWith("⚪ ")) {
        dummy.name = dummy.name.mid(2);
    }
    dummy.server = "localhost";
    dummy.port = 25565;
    dummy.username = "";
    dummy.password = "";
    dummy.status = "stopped";
    
    if (botDataCache.contains(botId)) {
        QJsonObject cached = botDataCache[botId];
        dummy.server = cached["server"].toString("localhost");
        dummy.port = cached["port"].toInt(25565);
        dummy.username = cached["username"].toString("");
        dummy.password = cached["password"].toString("");
        dummy.status = cached["status"].toString("stopped");
    }
    
    showBotDialog(&dummy);
}

void MainWindow::onDeleteBot() {
    QListWidgetItem* item = botList->currentItem();
    if (!item) {
        showNotification("No Selection", "Please select a bot to delete", true);
        return;
    }
    
    QString botId = item->data(Qt::UserRole).toString();
    QString botName = item->text();
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Confirm Delete",
        "Are you sure you want to delete " + botName + "?",
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        QJsonObject data;
        data["id"] = botId;
        sendMessage("delete_bot", data);
        botDataCache.remove(botId);
        clearBotInfo();
        showNotification("Bot Deleted", "Bot removed successfully!", false);
    }
}

void MainWindow::onStartBot() {
    QListWidgetItem* item = botList->currentItem();
    if (!item) {
        showNotification("No Selection", "Please select a bot to start", true);
        return;
    }
    
    QString botId = item->data(Qt::UserRole).toString();
    QString status = item->data(Qt::UserRole + 1).toString();
    
    if (status == "online" || status == "connecting") {
        showNotification("Already Running", "This bot is already running", true);
        return;
    }
    
    QJsonObject data;
    data["id"] = botId;
    sendMessage("start_bot", data);
    showNotification("Starting Bot", "Bot is connecting to server...", false);
}

void MainWindow::onStopBot() {
    QListWidgetItem* item = botList->currentItem();
    if (!item) {
        showNotification("No Selection", "Please select a bot to stop", true);
        return;
    }
    
    QString botId = item->data(Qt::UserRole).toString();
    QJsonObject data;
    data["id"] = botId;
    sendMessage("stop_bot", data);
    showNotification("Stopping Bot", "Bot is disconnecting...", false);
}

void MainWindow::onBotSelected(QListWidgetItem* item) {
    if (!item) return;
    
    QString botId = item->data(Qt::UserRole).toString();
    currentBotId = botId;
    
    QJsonObject data;
    data["id"] = botId;
    sendMessage("get_bot_info", data);
}

void MainWindow::onBotDoubleClicked(QListWidgetItem* item) {
    onEditBot();
}

void MainWindow::onSendCommand() {
    if (currentBotId.isEmpty()) {
        showNotification("No Bot Selected", "Please select a bot first", true);
        return;
    }
    
    QString command = commandInput->text().trimmed();
    if (command.isEmpty()) return;
    
    QJsonObject data;
    data["id"] = currentBotId;
    data["command"] = command;
    sendMessage("bot_command", data);
    
    commandInput->clear();
}

void MainWindow::onMoveTo() {
    if (currentBotId.isEmpty()) {
        showNotification("No Bot Selected", "Please select a bot first", true);
        return;
    }
    
    bool ok;
    double x = QInputDialog::getDouble(this, "Move To", "Enter X coordinate:", 0, -10000, 10000, 1, &ok);
    if (!ok) return;
    double y = QInputDialog::getDouble(this, "Move To", "Enter Y coordinate:", 64, -10000, 10000, 1, &ok);
    if (!ok) return;
    double z = QInputDialog::getDouble(this, "Move To", "Enter Z coordinate:", 0, -10000, 10000, 1, &ok);
    if (!ok) return;
    
    QJsonObject data;
    data["id"] = currentBotId;
    data["command"] = QString("moveto %1 %2 %3").arg(x).arg(y).arg(z);
    sendMessage("bot_command", data);
    
    addCommandToOutput(QString("moveto %1 %2 %3").arg(x).arg(y).arg(z), "Moving to target position...");
}

void MainWindow::onFollowPlayer() {
    if (currentBotId.isEmpty()) {
        showNotification("No Bot Selected", "Please select a bot first", true);
        return;
    }
    
    QString playerName = followInput->text().trimmed();
    if (playerName.isEmpty()) {
        showNotification("No Player", "Please enter a player name to follow", true);
        return;
    }
    
    QJsonObject data;
    data["id"] = currentBotId;
    data["command"] = QString("follow %1").arg(playerName);
    sendMessage("bot_command", data);
    
    addCommandToOutput(QString("follow %1").arg(playerName), "Following " + playerName + "...");
    followInput->clear();
}

void MainWindow::onGuardArea() {
    if (currentBotId.isEmpty()) {
        showNotification("No Bot Selected", "Please select a bot first", true);
        return;
    }
    
    int radius = guardInput->text().toInt();
    if (radius <= 0) radius = 10;
    
    QJsonObject data;
    data["id"] = currentBotId;
    data["command"] = QString("guard %1").arg(radius);
    sendMessage("bot_command", data);
    
    addCommandToOutput(QString("guard %1").arg(radius), "Now guarding area with radius " + QString::number(radius));
}

void MainWindow::onCollectResources() {
    if (currentBotId.isEmpty()) {
        showNotification("No Bot Selected", "Please select a bot first", true);
        return;
    }
    
    QString resource = collectInput->text().trimmed();
    if (resource.isEmpty()) {
        showNotification("No Resource", "Please enter a resource to collect", true);
        return;
    }
    
    QJsonObject data;
    data["id"] = currentBotId;
    data["command"] = QString("collect %1").arg(resource);
    sendMessage("bot_command", data);
    
    addCommandToOutput(QString("collect %1").arg(resource), "Searching for " + resource + "...");
    collectInput->clear();
}

void MainWindow::onDropItem() {
    if (currentBotId.isEmpty()) {
        showNotification("No Bot Selected", "Please select a bot first", true);
        return;
    }
    
    QString input = dropInput->text().trimmed();
    if (input.isEmpty()) {
        showNotification("No Item", "Please enter item to drop [optional: count]", true);
        return;
    }
    
    QJsonObject data;
    data["id"] = currentBotId;
    data["command"] = QString("drop %1").arg(input);
    sendMessage("bot_command", data);
    
    addCommandToOutput(QString("drop %1").arg(input), "Dropping item...");
    dropInput->clear();
}

void MainWindow::onHuntAnimals() {
    if (currentBotId.isEmpty()) {
        showNotification("No Bot Selected", "Please select a bot first", true);
        return;
    }
    
    QString animal = huntInput->text().trimmed();
    if (animal.isEmpty()) {
        showNotification("No Animal", "Please enter an animal to hunt", true);
        return;
    }
    
    QJsonObject data;
    data["id"] = currentBotId;
    data["command"] = QString("hunt %1").arg(animal);
    sendMessage("bot_command", data);
    
    addCommandToOutput(QString("hunt %1").arg(animal), "Hunting " + animal + "...");
    huntInput->clear();
}

void MainWindow::onHuntPlayers() {
    if (currentBotId.isEmpty()) {
        showNotification("No Bot Selected", "Please select a bot first", true);
        return;
    }
    
    QString player = huntPlayerInput->text().trimmed();
    
    QJsonObject data;
    data["id"] = currentBotId;
    if (player.isEmpty()) {
        data["command"] = "hunt_player";
        addCommandToOutput("hunt_player", "Searching for any player...");
    } else {
        data["command"] = QString("hunt_player %1").arg(player);
        addCommandToOutput(QString("hunt_player %1").arg(player), "Hunting " + player + "...");
    }
    sendMessage("bot_command", data);
    huntPlayerInput->clear();
}

void MainWindow::onStopAction() {
    if (currentBotId.isEmpty()) {
        showNotification("No Bot Selected", "Please select a bot first", true);
        return;
    }
    
    QJsonObject data;
    data["id"] = currentBotId;
    data["command"] = "stop";
    sendMessage("bot_command", data);
    
    addCommandToOutput("stop", "Stopping all actions...");
    taskStatusLabel->setText("No active task");
    taskProgressBar->setValue(0);
}

void MainWindow::onRefreshTasks() {
    if (currentBotId.isEmpty()) {
        showNotification("No Bot Selected", "Please select a bot first", true);
        return;
    }
    
    QJsonObject data;
    data["id"] = currentBotId;
    data["command"] = "tasks";
    sendMessage("bot_command", data);
}