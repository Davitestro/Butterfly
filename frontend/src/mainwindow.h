#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QPushButton>
#include <QWebSocket>
#include <QTimer>
#include <QJsonObject>
#include <QJsonArray>
#include <QLabel>
#include <QTableWidget>
#include <QTextEdit>
#include <QTabWidget>
#include <QGroupBox>
#include <QProgressBar>
#include <QMap>

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
    void onEditBot();
    void onDeleteBot();
    void onStartBot();
    void onStopBot();
    void onBotDoubleClicked(QListWidgetItem* item);
    void onBotSelected(QListWidgetItem* item);
    
    void onWebSocketConnected();
    void onWebSocketDisconnected();
    void onWebSocketMessage(const QString& message);
    void reconnectWebSocket();
    
    void onSendCommand();
    void onMoveTo();
    void onGuardArea();
    void onCollectResources();
    void onDropItem();
    void onHuntAnimals();
    void onHuntPlayers();

private:
    Ui::MainWindow *ui;
    QWebSocket* webSocket;
    QTimer* reconnectTimer;
    
    QListWidget* botList;
    QTabWidget* centralTabs;
    QWidget* infoWidget;
    QWidget* inventoryWidget;
    QWidget* commandsWidget;
    
    QLabel* botNameLabel;
    QLabel* botStatusLabel;
    QLabel* botServerLabel;
    QLabel* botUsernameLabel;
    QProgressBar* healthBar;
    QProgressBar* foodBar;
    QLabel* expLabel;
    QLabel* levelLabel;
    QLabel* coordsLabel;
    QLabel* dimensionLabel;
    QLabel* gamemodeLabel;
    
    QTableWidget* inventoryTable;
    
    QTextEdit* commandOutput;
    QLineEdit* commandInput;
    QPushButton* sendButton;
    QPushButton* moveToButton;
    QPushButton* guardButton;
    QPushButton* collectButton;
    QPushButton* dropButton;
    QPushButton* huntAnimalsButton;
    QPushButton* huntPlayersButton;
    
    QPushButton* addButton;
    QPushButton* deleteButton;
    QPushButton* startButton;
    QPushButton* stopButton;
    QPushButton* editButton;
    
    QString currentBotId;
    QMap<QString, QJsonObject> botDataCache; // Cache full bot data for editing
    
    void setupUI();
    void setupWebSocket();
    void updateBotList(const QJsonArray& botsArray);
    void updateBotInfo(const QJsonObject& botData);
    void updateBotStats(const QJsonObject& stats);
    void updateInventory(const QJsonArray& inventory);
    void showNotification(const QString& title, const QString& message, bool isError = false);
    void sendMessage(const QString& type, const QJsonObject& data = QJsonObject());
    void sendMessage(const QString& type);
    void showBotDialog(const BotData* existing = nullptr);
    void clearBotInfo();
    void addCommandToOutput(const QString& command, const QString& response);
};

#endif // MAINWINDOW_H