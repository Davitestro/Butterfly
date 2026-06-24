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

class BotDialog : public QDialog {
public:
    BotDialog(QWidget* parent = nullptr, const BotData* existing = nullptr) : QDialog(parent) {
        setWindowTitle(existing ? "Edit Bot" : "Add Bot");
        resize(400, 300);
        
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
        form->addRow("Password:", passwordEdit);
        
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
        data.name = nameEdit->text();
        data.server = serverEdit->text();
        data.port = portSpin->value();
        data.username = usernameEdit->text();
        data.password = passwordEdit->text();
        return data;
    }
    
private:
    QLineEdit* nameEdit;
    QLineEdit* serverEdit;
    QSpinBox* portSpin;
    QLineEdit* usernameEdit;
    QLineEdit* passwordEdit;
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
    resize(500, 400);
    
    QWidget* central = new QWidget(this);
    setCentralWidget(central);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(central);
    
    // Bot list
    botList = new QListWidget(this);
    botList->setMinimumWidth(300);
    botList->setMinimumHeight(300);
    connect(botList, &QListWidget::itemDoubleClicked, this, &MainWindow::onBotDoubleClicked);
    mainLayout->addWidget(botList);
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    addButton = new QPushButton("➕ Add Bot", this);
    connect(addButton, &QPushButton::clicked, this, &MainWindow::onAddBot);
    buttonLayout->addWidget(addButton);
    
    deleteButton = new QPushButton("🗑 Delete", this);
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::onDeleteBot);
    buttonLayout->addWidget(deleteButton);
    
    startButton = new QPushButton("▶ Start", this);
    connect(startButton, &QPushButton::clicked, this, &MainWindow::onStartBot);
    buttonLayout->addWidget(startButton);
    
    stopButton = new QPushButton("⏹ Stop", this);
    connect(stopButton, &QPushButton::clicked, this, &MainWindow::onStopBot);
    buttonLayout->addWidget(stopButton);
    
    mainLayout->addLayout(buttonLayout);
    
    statusBar()->showMessage("Ready");
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
    // Use the single-parameter version
    QJsonObject empty;
    sendMessage("get_bots", empty);
}

void MainWindow::onWebSocketDisconnected() {
    statusBar()->showMessage("Disconnected from backend, reconnecting...");
    reconnectTimer->start();
}

void MainWindow::onWebSocketMessage(const QString& message) {
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (!doc.isObject()) return;
    
    QJsonObject obj = doc.object();
    QString type = obj["type"].toString();
    
    if (type == "bots_update") {
        updateBotList(obj["data"].toArray());
    }
}

void MainWindow::updateBotList(const QJsonArray& botsArray) {
    botList->clear();
    
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
        
        QString statusIcon;
        if (bot.status == "online") statusIcon = "🟢 ";
        else if (bot.status == "connecting") statusIcon = "🟡 ";
        else if (bot.status == "error") statusIcon = "🔴 ";
        else statusIcon = "⚪ ";
        
        QListWidgetItem* item = new QListWidgetItem(
            statusIcon + bot.name + " (" + bot.server + ":" + QString::number(bot.port) + ")"
        );
        item->setData(Qt::UserRole, bot.id);
        item->setData(Qt::UserRole + 1, bot.status);
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
    BotDialog dialog(this, existing);
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
        } else {
            sendMessage("add_bot", botData);
        }
    }
}

void MainWindow::onAddBot() {
    showBotDialog(nullptr);
}

void MainWindow::onDeleteBot() {
    QListWidgetItem* item = botList->currentItem();
    if (!item) {
        QMessageBox::warning(this, "No Selection", "Please select a bot to delete");
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
    }
}

void MainWindow::onStartBot() {
    QListWidgetItem* item = botList->currentItem();
    if (!item) {
        QMessageBox::warning(this, "No Selection", "Please select a bot to start");
        return;
    }
    
    QString botId = item->data(Qt::UserRole).toString();
    QString status = item->data(Qt::UserRole + 1).toString();
    
    if (status == "online" || status == "connecting") {
        QMessageBox::information(this, "Already Running", "This bot is already running");
        return;
    }
    
    QJsonObject data;
    data["id"] = botId;
    sendMessage("start_bot", data);
}

void MainWindow::onStopBot() {
    QListWidgetItem* item = botList->currentItem();
    if (!item) {
        QMessageBox::warning(this, "No Selection", "Please select a bot to stop");
        return;
    }
    
    QString botId = item->data(Qt::UserRole).toString();
    QJsonObject data;
    data["id"] = botId;
    sendMessage("stop_bot", data);
}

void MainWindow::onBotDoubleClicked(QListWidgetItem* item) {
    QString botId = item->data(Qt::UserRole).toString();
    
    // Find bot data from the list
    BotData dummy;
    dummy.id = botId;
    QString fullText = item->text();
    QString name = fullText.split(" (").first();
    // Remove status icon if present
    if (name.startsWith("🟢 ") || name.startsWith("🟡 ") || 
        name.startsWith("🔴 ") || name.startsWith("⚪ ")) {
        name = name.mid(2);
    }
    dummy.name = name;
    dummy.server = "localhost";
    dummy.port = 25565;
    dummy.username = "";
    dummy.password = "";
    dummy.status = "stopped";
    
    showBotDialog(&dummy);
}