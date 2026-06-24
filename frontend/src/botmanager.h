#ifndef BOTMANAGER_H
#define BOTMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QWebSocket>

struct BotInfo {
    QString id;
    QString username;
    QString serverIp;
    int port;
    QString status;
    QString error;
};

class BotManager : public QObject
{
    Q_OBJECT

public:
    explicit BotManager(QObject *parent = nullptr);
    ~BotManager();

    void createBot(const QString& botId, const QString& username, 
                   const QString& password, const QString& serverIp, int port);
    void removeBot(const QString& botId);
    void getBotList();
    void sendCommand(const QString& botId, const QString& command);

signals:
    void botListUpdated(const QList<BotInfo>& bots);
    void botMessageReceived(const QString& botId, const QString& message);
    void botStatusUpdated(const QString& botId, const QString& status);

private slots:
    void onNetworkReply(QNetworkReply* reply);
    void onWebSocketMessage(const QString& message);
    void onWebSocketConnected();
    void onWebSocketDisconnected();

private:
    void connectWebSocket();
    void parseBotList(const QJsonObject& data);
    void setupWebSocketHandlers();
    
    QNetworkAccessManager *networkManager;
    QWebSocket *webSocket;
    QString backendUrl = "http://localhost:3000";
    QString wsUrl = "ws://localhost:3000";
};

#endif // BOTMANAGER_H
