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
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QScrollArea>
#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QVector>

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

struct MapEntity {
    QString name;
    int x, y, z;
    bool hostile;
    int distance;
};

struct MapPlayer {
    QString name;
    int x, y, z;
    int distance;
};

struct MapData {
    int centerX, centerY, centerZ;
    QMap<QString, QString> blocks;
    QVector<MapEntity> entities;
    QVector<MapPlayer> players;
};

class MinimapWidget : public QWidget {
    Q_OBJECT
public:
    explicit MinimapWidget(QWidget* parent = nullptr);
    void setMapData(const MapData& data);
    void setNoData(bool noData);
    QSize sizeHint() const override { return QSize(280, 280); }
    QSize minimumSizeHint() const override { return QSize(200, 200); }

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    MapData mapData;
    bool noDataState;
    void drawGrid(QPainter& painter, const QRect& rect);
    void drawBlocks(QPainter& painter, const QRect& rect, int cellSize);
    void drawEntities(QPainter& painter, const QRect& rect, int cellSize);
    void drawBot(QPainter& painter, const QRect& rect);
    QColor getBlockColor(const QString& blockName);
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
    
    QListWidget* botList;
    QTabWidget* centralTabs;
    QWidget* infoWidget;
    QWidget* inventoryWidget;
    QWidget* tasksWidget;
    QWidget* commandsWidget;
    
    QLabel* botNameLabel;
    QLabel* botStatusLabel;
    QLabel* botServerLabel;
    QLabel* botUsernameLabel;
    QProgressBar* healthBar;
    QProgressBar* foodBar;
    QLabel* healthValueLabel;
    QLabel* foodValueLabel;
    QLabel* expLabel;
    QLabel* levelLabel;
    QLabel* coordsLabel;
    QLabel* dimensionLabel;
    QLabel* gamemodeLabel;
    QLabel* taskStatusLabel;
    QProgressBar* taskProgressBar;
    
    MinimapWidget* minimap;
    QWidget* noDataOverlay;
    QLabel* noDataLabel;
    
    QTableWidget* inventoryTable;
    QTableWidget* tasksTable;
    QPushButton* refreshTasksButton;
    
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
    
    QPushButton* addButton;
    QPushButton* deleteButton;
    QPushButton* startButton;
    QPushButton* stopButton;
    QPushButton* editButton;
    
    QLabel* statusGlow;
    QTimer* statusPulseTimer;
    
    bool botOnline;
    
    void setupAnimations();
    void animateTabSwitch(int index);
    void pulseStatusIndicator();
    void setBotOnlineState(bool online);
    
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
    void updateMap(const QJsonObject& mapData);
    void showNotification(const QString& title, const QString& message, bool isError = false);
    void sendMessage(const QString& type, const QJsonObject& data = QJsonObject());
    void sendMessage(const QString& type);
    void showBotDialog(const BotData* existing = nullptr);
    void clearBotInfo();
    void addCommandToOutput(const QString& command, const QString& response);
};

#endif // MAINWINDOW_H
