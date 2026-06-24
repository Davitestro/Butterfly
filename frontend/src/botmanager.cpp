#include "botmanager.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QTimer>
#include <QDebug>

BotManager::BotManager(QObject *parent)
    : QObject(parent)
{
    networkManager = new QNetworkAccessManager(this);
    connect(networkManager, &QNetworkAccessManager::finished, 
            this, &BotManager::onNetworkReply);
    
    webSocket = new QWebSocket();
    connectWebSocket();
    setupWebSocketHandlers();
}

BotManager::~BotManager()
{
    if (webSocket) {
        webSocket->close();
        delete webSocket;
    }
}

void BotManager::connectWebSocket()
{
    QUrl url(wsUrl);
    webSocket->open(url);
}

void BotManager::setupWebSocketHandlers()
{
    connect(webSocket, &QWebSocket::connected, 
            this, &BotManager::onWebSocketConnected);
    connect(webSocket, &QWebSocket::disconnected, 
            this, &BotManager::onWebSocketDisconnected);
    connect(webSocket, &QWebSocket::textMessageReceived, 
            this, &BotManager::onWebSocketMessage);
}

void BotManager::onWebSocketConnected()
{
    qDebug() << "WebSocket connected to backend";
}

void BotManager::onWebSocketDisconnected()
{
    qDebug() << "WebSocket disconnected, reconnecting...";
    QTimer::singleShot(3000, this, &BotManager::connectWebSocket);
}

void BotManager::onWebSocketMessage(const QString& message)
{
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (!doc.isObject()) return;
    
    QJsonObject obj = doc.object();
    QString type = obj["type"].toString();
    
    if (type == "botMessage") {
        QString botId = obj["botId"].toString();
        QString msg = obj["message"].toString();
        emit botMessageReceived(botId, msg);
    } else if (type == "botStatusUpdate") {
        QString botId = obj["botId"].toString();
        QString status = obj["status"].toString();
        emit botStatusUpdated(botId, status);
    }
}

void BotManager::createBot(const QString& botId, const QString& username, 
                          const QString& password, const QString& serverIp, int port)
{
    QJsonObject request;
    request["botId"] = botId;
    request["username"] = username;
    request["password"] = password;
    request["serverIp"] = serverIp;
    request["port"] = port;
    
    QNetworkRequest req(QUrl(backendUrl + "/api/bots/create"));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QJsonDocument doc(request);
    networkManager->post(req, doc.toJson());
}

void BotManager::removeBot(const QString& botId)
{
    QNetworkRequest req(QUrl(backendUrl + "/api/bots/" + botId));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    networkManager->deleteResource(req);
}

void BotManager::getBotList()
{
    QNetworkRequest req(QUrl(backendUrl + "/api/bots"));
    networkManager->get(req);
}

void BotManager::sendCommand(const QString& botId, const QString& command)
{
    QJsonObject request;
    request["command"] = command;
    
    QNetworkRequest req(QUrl(backendUrl + "/api/bots/" + botId + "/command"));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QJsonDocument doc(request);
    networkManager->post(req, doc.toJson());
}

void BotManager::onNetworkReply(QNetworkReply* reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Network error:" << reply->errorString();
        reply->deleteLater();
        return;
    }
    
    QString url = reply->url().toString();
    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    
    if (url.contains("/api/bots") && !url.contains("/command")) {
        parseBotList(doc.object());
    }
    
    reply->deleteLater();
}

void BotManager::parseBotList(const QJsonObject& data)
{
    QList<BotInfo> botList;
    QJsonArray bots = data["bots"].toArray();
    
    for (const QJsonValue& value : bots) {
        QJsonObject obj = value.toObject();
        BotInfo info;
        info.id = obj["id"].toString();
        info.username = obj["username"].toString();
        info.serverIp = obj["serverIp"].toString();
        info.port = obj["port"].toInt();
        info.status = obj["status"].toString();
        info.error = obj["error"].toString();
        botList.append(info);
    }
    
    emit botListUpdated(botList);
}
