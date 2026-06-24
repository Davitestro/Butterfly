#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QPushButton>
#include <QWebSocket>
#include <QTimer>
#include <QJsonObject>
#include <QJsonArray>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

struct BotData {
    QString id;
    QString name;
    QString server;
    int port;
    QString username;
    QString password;
    QString status;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onAddBot();
    void onDeleteBot();
    void onStartBot();
    void onStopBot();
    void onBotDoubleClicked(QListWidgetItem* item);
    void onWebSocketConnected();
    void onWebSocketDisconnected();
    void onWebSocketMessage(const QString& message);
    void reconnectWebSocket();

private:
    Ui::MainWindow *ui;
    QWebSocket* webSocket;
    QTimer* reconnectTimer;
    QListWidget* botList;
    QPushButton* addButton;
    QPushButton* deleteButton;
    QPushButton* startButton;
    QPushButton* stopButton;
    
    void setupUI();
    void setupWebSocket();
    void updateBotList(const QJsonArray& botsArray);
    void sendMessage(const QString& type, const QJsonObject& data = QJsonObject());
    void sendMessage(const QString& type);
    void showBotDialog(const BotData* existing = nullptr);
};

#endif // MAINWINDOW_H