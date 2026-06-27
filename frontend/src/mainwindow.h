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
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>

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

struct BotTask {
    QString name;
    bool completed;
    qint64 started;
    qint64 completedAt;
    int total;
    int progress;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Bot management
    void onAddBot();
    void onEditBot();
    void onDeleteBot();
    void onStartBot();
    void onStopBot();
    void onBotDoubleClicked(QListWidgetItem* item);
    void onBotSelected(QListWidgetItem* item);
    
    // WebSocket
    void onWebSocketConnected();
    void onWebSocketDisconnected();
    void onWebSocketMessage(const QString& message);
    void reconnectWebSocket();
    
    // Bot commands
    void onSendCommand();
    void onMoveTo();
    void onFollowPlayer();
    void onGuardArea();
    void onCollectResources();
    void onDropItem();
    void onHuntAnimals();
    void onHuntPlayers();
    void onStopAction();
    void onRefreshTasks();

private:
    Ui::MainWindow *ui;
    QWebSocket* webSocket;
    QTimer* reconnectTimer;
    
    // UI Components
    QListWidget* botList;
    QTabWidget* centralTabs;
    QWidget* infoWidget;
    QWidget* inventoryWidget;
    QWidget* tasksWidget;
    QWidget* commandsWidget;
    
    // Bot info
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
    QLabel* taskStatusLabel;
    QProgressBar* taskProgressBar;
    
    // Inventory
    QTableWidget* inventoryTable;
    
    // Tasks
    QTableWidget* tasksTable;
    QPushButton* refreshTasksButton;
    
    // Commands
    QTextEdit* commandOutput;
    QLineEdit* commandInput;
    QPushButton* sendButton;
    QPushButton* moveToButton;
    QPushButton* followButton;
    QLineEdit* followInput;
    QPushButton* guardButton;
    QLineEdit* guardInput;
    QPushButton* collectButton;
    QLineEdit* collectInput;
    QPushButton* dropButton;
    QLineEdit* dropInput;
    QPushButton* huntAnimalsButton;
    QLineEdit* huntInput;
    QPushButton* huntPlayersButton;
    QLineEdit* huntPlayerInput;
    QPushButton* stopActionButton;
    
    // Side panel buttons
    QPushButton* addButton;
    QPushButton* deleteButton;
    QPushButton* startButton;
    QPushButton* stopButton;
    QPushButton* editButton;
    
    // State
    QString currentBotId;
    QMap<QString, QJsonObject> botDataCache;
    
    void setupUI();
    void setupWebSocket();
    void updateBotList(const QJsonArray& botsArray);
    void updateBotInfo(const QJsonObject& botData);
    void updateBotStats(const QJsonObject& stats);
    void updateInventory(const QJsonArray& inventory);
    void updateTasks(const QJsonArray& tasks);
    void updateTaskProgress(const QJsonObject& taskData);
    void showNotification(const QString& title, const QString& message, bool isError = false);
    void sendMessage(const QString& type, const QJsonObject& data = QJsonObject());
    void sendMessage(const QString& type);
    void showBotDialog(const BotData* existing = nullptr);
    void clearBotInfo();
    void addCommandToOutput(const QString& command, const QString& response);
};

#endif // MAINWINDOW_H