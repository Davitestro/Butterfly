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
#include <QFrame>
#include <QScrollArea>
#include <QInputDialog>
#include <QJsonObject>
#include <QJsonArray>
#include <QComboBox>
#include <QCheckBox>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QTimer>
#include <QEasingCurve>
#include <QPainter>
#include <QApplication>
#include <QProcessEnvironment>

class BotDialog : public QDialog {
public:
    BotDialog(QWidget* parent = nullptr, const BotData* existing = nullptr, const QJsonObject* fullData = nullptr) : QDialog(parent) {
        setWindowTitle(existing ? "Edit Bot" : "Add Bot");
        resize(420, 380);
        setObjectName("botDialog");
        
        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        mainLayout->setSpacing(16);
        mainLayout->setContentsMargins(24, 24, 24, 24);
        
        QLabel* titleLabel = new QLabel(existing ? "Edit Bot Configuration" : "Create New Bot", this);
        titleLabel->setObjectName("dialogTitle");
        mainLayout->addWidget(titleLabel);
        
        QFormLayout* form = new QFormLayout();
        form->setSpacing(12);
        form->setLabelAlignment(Qt::AlignRight);
        
        nameEdit = new QLineEdit(this);
        nameEdit->setObjectName("formInput");
        nameEdit->setPlaceholderText("My Bot");
        form->addRow("Bot Name:", nameEdit);
        
        serverEdit = new QLineEdit(this);
        serverEdit->setObjectName("formInput");
        serverEdit->setPlaceholderText("play.example.com");
        form->addRow("Server:", serverEdit);
        
        portSpin = new QSpinBox(this);
        portSpin->setObjectName("formInput");
        portSpin->setRange(1, 65535);
        portSpin->setValue(25565);
        form->addRow("Port:", portSpin);
        
        usernameEdit = new QLineEdit(this);
        usernameEdit->setObjectName("formInput");
        usernameEdit->setPlaceholderText("Steve");
        form->addRow("Username:", usernameEdit);
        
        passwordEdit = new QLineEdit(this);
        passwordEdit->setObjectName("formInput");
        passwordEdit->setEchoMode(QLineEdit::Password);
        passwordEdit->setPlaceholderText("Leave empty for offline mode");
        form->addRow("Password:", passwordEdit);
        
        if (existing) {
            infoLabel = new QLabel(this);
            infoLabel->setObjectName("infoLabelSmall");
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
        mainLayout->addStretch();
        
        QDialogButtonBox* buttons = new QDialogButtonBox(
            QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
            Qt::Horizontal,
            this
        );
        buttons->setObjectName("dialogButtons");
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
    , statusGlow(nullptr)
    , statusPulseTimer(nullptr)
    , botOnline(false)
{
    ui->setupUi(this);
    setupUI();
    setupAnimations();
    setupWebSocket();
}

MainWindow::~MainWindow()
{
    delete ui;
}

QFrame* MainWindow_createCard(QWidget* parent) {
    QFrame* card = new QFrame(parent);
    card->setObjectName("card");
    return card;
}

void MainWindow::setupUI() {
    setWindowTitle("Minecraft Bot Manager");
    resize(1400, 800);
    setMinimumSize(1100, 650);
    
    QWidget* central = new QWidget(this);
    central->setObjectName("centralWidget");
    setCentralWidget(central);
    QHBoxLayout* mainLayout = new QHBoxLayout(central);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    
    // ==================== LEFT SIDEBAR ====================
    QWidget* sidebar = new QWidget(this);
    sidebar->setObjectName("sidebar");
    sidebar->setFixedWidth(300);
    
    QVBoxLayout* sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setSpacing(0);
    sidebarLayout->setContentsMargins(0, 0, 0, 0);
    
    // --- Sidebar Header ---
    QWidget* sidebarHeader = new QWidget(this);
    sidebarHeader->setObjectName("sidebarHeader");
    sidebarHeader->setFixedHeight(80);
    QVBoxLayout* headerLay = new QVBoxLayout(sidebarHeader);
    headerLay->setContentsMargins(20, 16, 20, 16);
    
    QLabel* appTitle = new QLabel("MC Bot Manager", this);
    appTitle->setObjectName("appTitle");
    headerLay->addWidget(appTitle);
    
    QLabel* appSubtitle = new QLabel("Control your bots", this);
    appSubtitle->setObjectName("appSubtitle");
    headerLay->addWidget(appSubtitle);
    
    sidebarLayout->addWidget(sidebarHeader);
    
    // --- Bots Section Label ---
    QWidget* botsHeader = new QWidget(this);
    botsHeader->setObjectName("sectionHeader");
    botsHeader->setFixedHeight(40);
    QHBoxLayout* botsHLay = new QHBoxLayout(botsHeader);
    botsHLay->setContentsMargins(20, 0, 20, 0);
    
    QLabel* botsLabel = new QLabel("BOTS", this);
    botsLabel->setObjectName("sectionLabel");
    botsHLay->addWidget(botsLabel);
    botsHLay->addStretch();
    
    QLabel* botCountLabel = new QLabel("0", this);
    botCountLabel->setObjectName("badge");
    botsHLay->addWidget(botCountLabel);
    
    sidebarLayout->addWidget(botsHeader);
    
    // --- Bot List ---
    botList = new QListWidget(this);
    botList->setObjectName("botList");
    botList->setMinimumHeight(300);
    connect(botList, &QListWidget::itemClicked, this, &MainWindow::onBotSelected);
    connect(botList, &QListWidget::itemDoubleClicked, this, &MainWindow::onBotDoubleClicked);
    sidebarLayout->addWidget(botList, 1);
    
    // --- Action Buttons ---
    QWidget* actionsPanel = new QWidget(this);
    actionsPanel->setObjectName("actionsPanel");
    QVBoxLayout* actionsLayout = new QVBoxLayout(actionsPanel);
    actionsLayout->setSpacing(6);
    actionsLayout->setContentsMargins(16, 12, 16, 16);
    
    addButton = new QPushButton("Add Bot", this);
    addButton->setObjectName("addButton");
    addButton->setCursor(Qt::PointingHandCursor);
    addButton->setMinimumHeight(36);
    actionsLayout->addWidget(addButton);
    
    QHBoxLayout* editDelLayout = new QHBoxLayout();
    editDelLayout->setSpacing(6);
    editButton = new QPushButton("Edit", this);
    editButton->setObjectName("editButton");
    editButton->setCursor(Qt::PointingHandCursor);
    editButton->setMinimumHeight(34);
    deleteButton = new QPushButton("Delete", this);
    deleteButton->setObjectName("deleteButton");
    deleteButton->setCursor(Qt::PointingHandCursor);
    deleteButton->setMinimumHeight(34);
    editDelLayout->addWidget(editButton);
    editDelLayout->addWidget(deleteButton);
    actionsLayout->addLayout(editDelLayout);
    
    QHBoxLayout* startStopLayout = new QHBoxLayout();
    startStopLayout->setSpacing(6);
    startButton = new QPushButton("Start", this);
    startButton->setObjectName("startButton");
    startButton->setCursor(Qt::PointingHandCursor);
    startButton->setMinimumHeight(34);
    stopButton = new QPushButton("Stop", this);
    stopButton->setObjectName("stopButton");
    stopButton->setCursor(Qt::PointingHandCursor);
    stopButton->setMinimumHeight(34);
    startStopLayout->addWidget(startButton);
    startStopLayout->addWidget(stopButton);
    actionsLayout->addLayout(startStopLayout);
    
    sidebarLayout->addWidget(actionsPanel);
    
    connect(addButton, &QPushButton::clicked, this, &MainWindow::onAddBot);
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::onDeleteBot);
    connect(editButton, &QPushButton::clicked, this, &MainWindow::onEditBot);
    connect(startButton, &QPushButton::clicked, this, &MainWindow::onStartBot);
    connect(stopButton, &QPushButton::clicked, this, &MainWindow::onStopBot);
    
    mainLayout->addWidget(sidebar);
    
    // ==================== CENTER CONTENT ====================
    QWidget* centerPanel = new QWidget(this);
    centerPanel->setObjectName("centerPanel");
    QVBoxLayout* centerLayout = new QVBoxLayout(centerPanel);
    centerLayout->setSpacing(0);
    centerLayout->setContentsMargins(0, 0, 0, 0);
    
    // --- Top Header Bar ---
    QFrame* headerFrame = new QFrame(this);
    headerFrame->setObjectName("headerFrame");
    headerFrame->setFixedHeight(70);
    QHBoxLayout* headerLayout = new QHBoxLayout(headerFrame);
    headerLayout->setContentsMargins(24, 12, 24, 12);
    headerLayout->setSpacing(16);
    
    // Status glow indicator
    statusGlow = new QLabel(this);
    statusGlow->setObjectName("statusGlow");
    statusGlow->setFixedSize(12, 12);
    headerLayout->addWidget(statusGlow);
    
    botNameLabel = new QLabel("No bot selected", this);
    botNameLabel->setObjectName("botNameLabel");
    headerLayout->addWidget(botNameLabel);
    
    botStatusLabel = new QLabel("Offline", this);
    botStatusLabel->setObjectName("botStatusLabel");
    headerLayout->addWidget(botStatusLabel);
    
    headerLayout->addStretch();
    
    // Task progress in header
    QWidget* taskInfoWidget = new QWidget(this);
    taskInfoWidget->setObjectName("taskInfoWidget");
    QHBoxLayout* taskInfoLayout = new QHBoxLayout(taskInfoWidget);
    taskInfoLayout->setContentsMargins(0, 0, 0, 0);
    taskInfoLayout->setSpacing(10);
    
    taskStatusLabel = new QLabel("No active task", this);
    taskStatusLabel->setObjectName("taskStatusLabel");
    taskInfoLayout->addWidget(taskStatusLabel);
    
    taskProgressBar = new QProgressBar(this);
    taskProgressBar->setObjectName("taskProgressBar");
    taskProgressBar->setRange(0, 100);
    taskProgressBar->setValue(0);
    taskProgressBar->setFixedWidth(160);
    taskProgressBar->setFixedHeight(8);
    taskInfoLayout->addWidget(taskProgressBar);
    
    headerLayout->addWidget(taskInfoWidget);
    
    // Server & username tags
    botServerLabel = new QLabel("", this);
    botServerLabel->setObjectName("infoTag");
    headerLayout->addWidget(botServerLabel);
    botUsernameLabel = new QLabel("", this);
    botUsernameLabel->setObjectName("infoTag");
    headerLayout->addWidget(botUsernameLabel);
    
    centerLayout->addWidget(headerFrame);
    
    // --- Tabs Area ---
    QWidget* tabsContainer = new QWidget(this);
    tabsContainer->setObjectName("tabsContainer");
    QVBoxLayout* tabsContainerLayout = new QVBoxLayout(tabsContainer);
    tabsContainerLayout->setContentsMargins(20, 20, 20, 20);
    tabsContainerLayout->setSpacing(0);
    
    centralTabs = new QTabWidget(this);
    centralTabs->setObjectName("centralTabs");
    
    // === Tab 1: Bot Info ===
    infoWidget = new QWidget(this);
    infoWidget->setObjectName("infoTab");
    QVBoxLayout* infoVLayout = new QVBoxLayout(infoWidget);
    infoVLayout->setSpacing(16);
    infoVLayout->setContentsMargins(8, 12, 8, 8);
    
    // Stats cards row
    QWidget* statsRow = new QWidget(this);
    statsRow->setObjectName("statsRow");
    QHBoxLayout* statsLayout = new QHBoxLayout(statsRow);
    statsLayout->setSpacing(12);
    statsLayout->setContentsMargins(0, 0, 0, 0);
    
    // Health Card
    QFrame* healthCard = MainWindow_createCard(this);
    healthCard->setObjectName("healthCard");
    QVBoxLayout* hcLay = new QVBoxLayout(healthCard);
    hcLay->setContentsMargins(16, 12, 16, 12);
    hcLay->setSpacing(6);
    QLabel* healthIcon = new QLabel("HP", this);
    healthIcon->setObjectName("statIcon");
    hcLay->addWidget(healthIcon);
    healthBar = new QProgressBar(this);
    healthBar->setObjectName("healthBar");
    healthBar->setRange(0, 20);
    healthBar->setValue(20);
    healthBar->setFixedHeight(6);
    healthBar->setTextVisible(false);
    hcLay->addWidget(healthBar);
    healthValueLabel = new QLabel("-- / 20", this);
    healthValueLabel->setObjectName("statValue");
    hcLay->addWidget(healthValueLabel);
    statsLayout->addWidget(healthCard);
    
    // Food Card
    QFrame* foodCard = MainWindow_createCard(this);
    foodCard->setObjectName("foodCard");
    QVBoxLayout* fcLay = new QVBoxLayout(foodCard);
    fcLay->setContentsMargins(16, 12, 16, 12);
    fcLay->setSpacing(6);
    QLabel* foodIcon = new QLabel("FOOD", this);
    foodIcon->setObjectName("statIcon");
    fcLay->addWidget(foodIcon);
    foodBar = new QProgressBar(this);
    foodBar->setObjectName("foodBar");
    foodBar->setRange(0, 20);
    foodBar->setValue(20);
    foodBar->setFixedHeight(6);
    foodBar->setTextVisible(false);
    fcLay->addWidget(foodBar);
    foodValueLabel = new QLabel("-- / 20", this);
    foodValueLabel->setObjectName("statValue");
    fcLay->addWidget(foodValueLabel);
    statsLayout->addWidget(foodCard);
    
    // XP Card
    QFrame* xpCard = MainWindow_createCard(this);
    xpCard->setObjectName("xpCard");
    QVBoxLayout* xcLay = new QVBoxLayout(xpCard);
    xcLay->setContentsMargins(16, 12, 16, 12);
    xcLay->setSpacing(6);
    QLabel* xpIcon = new QLabel("XP", this);
    xpIcon->setObjectName("statIcon");
    xcLay->addWidget(xpIcon);
    expLabel = new QLabel("0", this);
    expLabel->setObjectName("statBigValue");
    xcLay->addWidget(expLabel);
    QLabel* xpSub = new QLabel("Experience", this);
    xpSub->setObjectName("statSubtext");
    xcLay->addWidget(xpSub);
    statsLayout->addWidget(xpCard);
    
    // Level Card
    QFrame* levelCard = MainWindow_createCard(this);
    levelCard->setObjectName("levelCard");
    QVBoxLayout* lcLay = new QVBoxLayout(levelCard);
    lcLay->setContentsMargins(16, 12, 16, 12);
    lcLay->setSpacing(6);
    QLabel* lvlIcon = new QLabel("LVL", this);
    lvlIcon->setObjectName("statIcon");
    lcLay->addWidget(lvlIcon);
    levelLabel = new QLabel("0", this);
    levelLabel->setObjectName("statBigValue");
    lcLay->addWidget(levelLabel);
    QLabel* lvlSub = new QLabel("Level", this);
    lvlSub->setObjectName("statSubtext");
    lcLay->addWidget(lvlSub);
    statsLayout->addWidget(levelCard);
    
    infoVLayout->addWidget(statsRow);
    
    // === Minimap ===
    QWidget* mapSection = new QWidget(this);
    mapSection->setObjectName("mapSection");
    QHBoxLayout* mapLayout = new QHBoxLayout(mapSection);
    mapLayout->setContentsMargins(0, 0, 0, 0);
    mapLayout->setSpacing(16);
    
    QFrame* mapCard = MainWindow_createCard(this);
    mapCard->setObjectName("mapCard");
    QVBoxLayout* mapCardLayout = new QVBoxLayout(mapCard);
    mapCardLayout->setContentsMargins(12, 12, 12, 12);
    mapCardLayout->setSpacing(8);
    
    QLabel* mapTitle = new QLabel("AREA MAP", this);
    mapTitle->setObjectName("mapTitle");
    mapCardLayout->addWidget(mapTitle);
    
    // Minimap with overlay
    QWidget* mapContainer = new QWidget(this);
    mapContainer->setObjectName("mapContainer");
    mapContainer->setFixedSize(280, 280);
    QVBoxLayout* mapContainerLayout = new QVBoxLayout(mapContainer);
    mapContainerLayout->setContentsMargins(0, 0, 0, 0);
    
    minimap = new MinimapWidget(mapContainer);
    minimap->setFixedSize(280, 280);
    mapContainerLayout->addWidget(minimap);
    
    // No-data overlay
    noDataOverlay = new QWidget(mapContainer);
    noDataOverlay->setObjectName("noDataOverlay");
    noDataOverlay->setFixedSize(280, 280);
    noDataOverlay->setVisible(true);
    QVBoxLayout* ndLayout = new QVBoxLayout(noDataOverlay);
    ndLayout->setAlignment(Qt::AlignCenter);
    noDataLabel = new QLabel("Bot Offline", this);
    noDataLabel->setObjectName("noDataLabel");
    ndLayout->addWidget(noDataLabel);
    QLabel* ndSub = new QLabel("Start a bot to see map data", this);
    ndSub->setObjectName("noDataSubLabel");
    ndLayout->addWidget(ndSub);
    
    mapCardLayout->addWidget(mapContainer);
    
    // Map legend
    QHBoxLayout* legendLayout = new QHBoxLayout();
    legendLayout->setSpacing(12);
    auto addLegendItem = [&](const QString& color, const QString& label) {
        QLabel* dot = new QLabel(this);
        dot->setFixedSize(10, 10);
        dot->setStyleSheet("background: " + color + "; border-radius: 5px;");
        QLabel* lbl = new QLabel(label, this);
        lbl->setObjectName("legendLabel");
        legendLayout->addWidget(dot);
        legendLayout->addWidget(lbl);
    };
    addLegendItem("#4ade80", "You");
    addLegendItem("#f87171", "Hostile");
    addLegendItem("#60a5fa", "Player");
    addLegendItem("#a78bfa", "Mob");
    legendLayout->addStretch();
    mapCardLayout->addLayout(legendLayout);
    
    mapLayout->addWidget(mapCard);
    
    // Details section (right side of map row)
    QFrame* detailsCard = MainWindow_createCard(this);
    detailsCard->setObjectName("detailsCard");
    QGridLayout* detailsGrid = new QGridLayout(detailsCard);
    detailsGrid->setContentsMargins(20, 16, 20, 16);
    detailsGrid->setSpacing(12);
    
    auto addDetailRow = [&](int row, const QString& icon, const QString& label, QWidget* valueWidget) {
        QLabel* iconLbl = new QLabel(icon, this);
        iconLbl->setObjectName("detailIcon");
        detailsGrid->addWidget(iconLbl, row, 0);
        
        QLabel* nameLbl = new QLabel(label, this);
        nameLbl->setObjectName("detailLabel");
        detailsGrid->addWidget(nameLbl, row, 1);
        
        valueWidget->setObjectName("detailValue");
        detailsGrid->addWidget(valueWidget, row, 2);
    };
    
    coordsLabel = new QLabel("X: 0, Y: 0, Z: 0", this);
    addDetailRow(0, "POS", "Position", coordsLabel);
    
    dimensionLabel = new QLabel("Overworld", this);
    addDetailRow(1, "DIM", "Dimension", dimensionLabel);
    
    gamemodeLabel = new QLabel("Survival", this);
    addDetailRow(2, "GM", "Gamemode", gamemodeLabel);
    
    detailsGrid->setColumnStretch(2, 1);
    
    mapLayout->addWidget(detailsCard, 1);
    
    infoVLayout->addWidget(mapSection);
    infoVLayout->addStretch();
    
    centralTabs->addTab(infoWidget, "Info");
    
    // === Tab 2: Inventory ===
    inventoryWidget = new QWidget(this);
    inventoryWidget->setObjectName("inventoryTab");
    QVBoxLayout* invLayout = new QVBoxLayout(inventoryWidget);
    invLayout->setContentsMargins(8, 12, 8, 8);
    
    inventoryTable = new QTableWidget(this);
    inventoryTable->setObjectName("inventoryTable");
    inventoryTable->setColumnCount(3);
    inventoryTable->setHorizontalHeaderLabels({"Slot", "Item", "Count"});
    inventoryTable->horizontalHeader()->setStretchLastSection(true);
    inventoryTable->horizontalHeader()->setObjectName("tableHeader");
    inventoryTable->setAlternatingRowColors(true);
    inventoryTable->verticalHeader()->setVisible(false);
    inventoryTable->setShowGrid(false);
    inventoryTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    inventoryTable->setSelectionMode(QAbstractItemView::SingleSelection);
    invLayout->addWidget(inventoryTable);
    
    centralTabs->addTab(inventoryWidget, "Inventory");
    
    // === Tab 3: Tasks ===
    tasksWidget = new QWidget(this);
    tasksWidget->setObjectName("tasksTab");
    QVBoxLayout* tasksLayout = new QVBoxLayout(tasksWidget);
    tasksLayout->setContentsMargins(8, 12, 8, 8);
    tasksLayout->setSpacing(12);
    
    tasksTable = new QTableWidget(this);
    tasksTable->setObjectName("tasksTable");
    tasksTable->setColumnCount(4);
    tasksTable->setHorizontalHeaderLabels({"Task", "Status", "Progress", "Time"});
    tasksTable->horizontalHeader()->setStretchLastSection(true);
    tasksTable->horizontalHeader()->setObjectName("tableHeader");
    tasksTable->setAlternatingRowColors(true);
    tasksTable->verticalHeader()->setVisible(false);
    tasksTable->setShowGrid(false);
    tasksTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    tasksLayout->addWidget(tasksTable);
    
    refreshTasksButton = new QPushButton("Refresh Tasks", this);
    refreshTasksButton->setObjectName("refreshButton");
    refreshTasksButton->setCursor(Qt::PointingHandCursor);
    refreshTasksButton->setFixedWidth(160);
    connect(refreshTasksButton, &QPushButton::clicked, this, &MainWindow::onRefreshTasks);
    tasksLayout->addWidget(refreshTasksButton, 0, Qt::AlignRight);
    
    centralTabs->addTab(tasksWidget, "Tasks");
    
    // === Tab 4: Commands ===
    commandsWidget = new QWidget(this);
    commandsWidget->setObjectName("commandsTab");
    QVBoxLayout* cmdLayout = new QVBoxLayout(commandsWidget);
    cmdLayout->setContentsMargins(8, 12, 8, 8);
    cmdLayout->setSpacing(12);
    
    // Command output
    commandOutput = new QTextEdit(this);
    commandOutput->setObjectName("commandOutput");
    commandOutput->setReadOnly(true);
    commandOutput->setFont(QFont("JetBrains Mono", 10));
    commandOutput->setMinimumHeight(180);
    cmdLayout->addWidget(commandOutput);
    
    // Command grid in a card
    QFrame* cmdCard = MainWindow_createCard(this);
    cmdCard->setObjectName("commandCard");
    QGridLayout* cmdGrid = new QGridLayout(cmdCard);
    cmdGrid->setContentsMargins(16, 16, 16, 16);
    cmdGrid->setSpacing(10);
    
    auto makeCmdRow = [&](int row, const QString& label, const QString& placeholder, 
                          const QString& btnText, const QString& btnObjName,
                          QPushButton*& outBtn, QLineEdit*& outInput, bool hasInput = true) {
        QLabel* lbl = new QLabel(label, this);
        lbl->setObjectName("cmdLabel");
        cmdGrid->addWidget(lbl, row, 0);
        
        if (hasInput) {
            outInput = new QLineEdit(this);
            outInput->setObjectName("cmdInput");
            outInput->setPlaceholderText(placeholder);
            cmdGrid->addWidget(outInput, row, 1);
        }
        
        outBtn = new QPushButton(btnText, this);
        outBtn->setObjectName(btnObjName);
        outBtn->setCursor(Qt::PointingHandCursor);
        outBtn->setMinimumHeight(32);
        cmdGrid->addWidget(outBtn, row, hasInput ? 2 : 1);
    };
    
    moveToButton = nullptr;
    followInput = nullptr;
    guardInput = nullptr;
    collectInput = nullptr;
    dropInput = nullptr;
    huntInput = nullptr;
    huntPlayerInput = nullptr;
    
    makeCmdRow(0, "Move To:", "", "Move", "cmdMoveButton", moveToButton, followInput, false);
    connect(moveToButton, &QPushButton::clicked, this, &MainWindow::onMoveTo);
    
    makeCmdRow(1, "Follow:", "Player name", "Follow", "cmdFollowButton", followButton, followInput);
    connect(followButton, &QPushButton::clicked, this, &MainWindow::onFollowPlayer);
    
    makeCmdRow(2, "Guard:", "Radius", "Guard", "cmdGuardButton", guardButton, guardInput);
    guardInput->setText("10");
    connect(guardButton, &QPushButton::clicked, this, &MainWindow::onGuardArea);
    
    makeCmdRow(3, "Collect:", "Resource name", "Collect", "cmdCollectButton", collectButton, collectInput);
    connect(collectButton, &QPushButton::clicked, this, &MainWindow::onCollectResources);
    
    makeCmdRow(4, "Drop:", "Item [count]", "Drop", "cmdDropButton", dropButton, dropInput);
    connect(dropButton, &QPushButton::clicked, this, &MainWindow::onDropItem);
    
    makeCmdRow(5, "Hunt:", "Animal name", "Hunt", "cmdHuntButton", huntAnimalsButton, huntInput);
    connect(huntAnimalsButton, &QPushButton::clicked, this, &MainWindow::onHuntAnimals);
    
    makeCmdRow(6, "Hunt Player:", "Player name (optional)", "Hunt", "cmdHuntPlayerButton", huntPlayersButton, huntPlayerInput);
    connect(huntPlayersButton, &QPushButton::clicked, this, &MainWindow::onHuntPlayers);
    
    stopActionButton = new QPushButton("STOP ALL ACTIONS", this);
    stopActionButton->setObjectName("stopActionButton");
    stopActionButton->setCursor(Qt::PointingHandCursor);
    stopActionButton->setMinimumHeight(36);
    connect(stopActionButton, &QPushButton::clicked, this, &MainWindow::onStopAction);
    cmdGrid->addWidget(stopActionButton, 7, 0, 1, 3);
    
    cmdLayout->addWidget(cmdCard);
    
    // Custom command bar
    QFrame* customCmdFrame = new QFrame(this);
    customCmdFrame->setObjectName("customCmdFrame");
    QHBoxLayout* customCmdLayout = new QHBoxLayout(customCmdFrame);
    customCmdLayout->setContentsMargins(12, 8, 8, 8);
    customCmdLayout->setSpacing(8);
    
    QLabel* cmdPrefix = new QLabel(">", this);
    cmdPrefix->setObjectName("cmdPrefix");
    customCmdLayout->addWidget(cmdPrefix);
    
    commandInput = new QLineEdit(this);
    commandInput->setObjectName("commandInput");
    commandInput->setPlaceholderText("Type custom command...");
    customCmdLayout->addWidget(commandInput, 1);
    
    sendButton = new QPushButton("Send", this);
    sendButton->setObjectName("sendButton");
    sendButton->setCursor(Qt::PointingHandCursor);
    sendButton->setFixedSize(70, 34);
    
    connect(sendButton, &QPushButton::clicked, this, &MainWindow::onSendCommand);
    connect(commandInput, &QLineEdit::returnPressed, this, &MainWindow::onSendCommand);
    
    customCmdLayout->addWidget(sendButton);
    cmdLayout->addWidget(customCmdFrame);
    
    centralTabs->addTab(commandsWidget, "Commands");
    
    // Tab switch animation
    connect(centralTabs, &QTabWidget::currentChanged, this, &MainWindow::animateTabSwitch);
    
    tabsContainerLayout->addWidget(centralTabs);
    centerLayout->addWidget(tabsContainer, 1);
    
    mainLayout->addWidget(centerPanel, 1);
    
    statusBar()->setObjectName("mainStatusBar");
    statusBar()->showMessage("Ready");
    clearBotInfo();
}

void MainWindow::setupAnimations() {
    // Status glow pulse via timer (avoids QGraphicsOpacityEffect which breaks QSS hover)
    statusPulseTimer = new QTimer(this);
    statusPulseTimer->setInterval(1200);
    connect(statusPulseTimer, &QTimer::timeout, this, [this]() {
        static bool bright = false;
        bright = !bright;
        if (bright) {
            statusGlow->setStyleSheet("background: #4ade80; border-radius: 6px;");
        } else {
            statusGlow->setStyleSheet("background: #22c55e; border-radius: 6px; opacity: 0.6;");
        }
    });
    statusPulseTimer->start();
}

void MainWindow::animateTabSwitch(int index) {
    Q_UNUSED(index);
    // No-op: QGraphicsOpacityEffect causes black hover artifacts with QSS.
    // Tab switch is already instant via QTabWidget's built-in behavior.
}

void MainWindow::pulseStatusIndicator() {
    // Handled by statusPulseTimer in setupAnimations()
}

void MainWindow::showNotification(const QString& title, const QString& message, bool isError) {
    QMessageBox msgBox(this);
    msgBox.setObjectName("notificationBox");
    msgBox.setIcon(isError ? QMessageBox::Critical : QMessageBox::Information);
    msgBox.setWindowTitle(title);
    msgBox.setText(message);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();
}

void MainWindow::clearBotInfo() {
    botOnline = false;
    setBotOnlineState(false);
    
    botNameLabel->setText("No bot selected");
    botStatusLabel->setText("Offline");
    botStatusLabel->setProperty("status", "offline");
    statusGlow->setStyleSheet("background: #64748b; border-radius: 6px;");
    botServerLabel->setText("");
    botServerLabel->hide();
    botUsernameLabel->setText("");
    botUsernameLabel->hide();
    
    healthBar->setValue(0);
    foodBar->setValue(0);
    healthValueLabel->setText("-- / 20");
    foodValueLabel->setText("-- / 20");
    expLabel->setText("--");
    levelLabel->setText("--");
    coordsLabel->setText("X: --, Y: --, Z: --");
    dimensionLabel->setText("--");
    gamemodeLabel->setText("--");
    
    inventoryTable->setRowCount(0);
    tasksTable->setRowCount(0);
    commandOutput->clear();
    taskStatusLabel->setText("No active task");
    taskProgressBar->setValue(0);
    currentBotId = "";
    
    MapData emptyMap;
    minimap->setMapData(emptyMap);
    minimap->setNoData(true);
    noDataOverlay->setVisible(true);
}

void MainWindow::updateBotInfo(const QJsonObject& botData) {
    if (!botData.isEmpty()) {
        botNameLabel->setText(botData["name"].toString());
        botServerLabel->setText(botData["server"].toString());
        botServerLabel->show();
        botUsernameLabel->setText(botData["username"].toString());
        botUsernameLabel->show();
        
        QString status = botData["status"].toString();
        QString statusText;
        QString glowColor;
        QString statusProp;
        bool isOnline = false;
        
        if (status == "online") {
            statusText = "Online";
            glowColor = "background: #4ade80; border-radius: 6px;";
            statusProp = "online";
            isOnline = true;
        } else if (status == "connecting") {
            statusText = "Connecting...";
            glowColor = "background: #facc15; border-radius: 6px;";
            statusProp = "connecting";
        } else if (status == "error") {
            statusText = "Error";
            glowColor = "background: #f87171; border-radius: 6px;";
            statusProp = "error";
        } else {
            statusText = "Offline";
            glowColor = "background: #64748b; border-radius: 6px;";
            statusProp = "offline";
        }
        
        botStatusLabel->setText(statusText);
        botStatusLabel->setProperty("status", statusProp);
        statusGlow->setStyleSheet(glowColor);
        style()->unpolish(botStatusLabel);
        style()->polish(botStatusLabel);
        
        setBotOnlineState(isOnline);
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
    
    QPropertyAnimation* healthAnim = new QPropertyAnimation(healthBar, "value", this);
    healthAnim->setDuration(400);
    healthAnim->setStartValue(healthBar->value());
    healthAnim->setEndValue((int)health);
    healthAnim->setEasingCurve(QEasingCurve::OutCubic);
    connect(healthAnim, &QPropertyAnimation::finished, healthAnim, &QObject::deleteLater);
    healthAnim->start();
    
    QPropertyAnimation* foodAnim = new QPropertyAnimation(foodBar, "value", this);
    foodAnim->setDuration(400);
    foodAnim->setStartValue(foodBar->value());
    foodAnim->setEndValue(food);
    foodAnim->setEasingCurve(QEasingCurve::OutCubic);
    connect(foodAnim, &QPropertyAnimation::finished, foodAnim, &QObject::deleteLater);
    foodAnim->start();
    
    healthValueLabel->setText(QString("%1 / 20").arg(health, 0, 'f', 1));
    foodValueLabel->setText(QString("%1 / 20").arg(food));
    expLabel->setText(QString::number(exp));
    levelLabel->setText(QString::number(level));
    coordsLabel->setText(QString("X: %1, Y: %2, Z: %3").arg(x, 0, 'f', 1).arg(y, 0, 'f', 1).arg(z, 0, 'f', 1));
    
    if (stats.contains("dimension")) {
        dimensionLabel->setText(stats["dimension"].toString());
    }
    if (stats.contains("gamemode")) {
        gamemodeLabel->setText(stats["gamemode"].toString());
    }
    
    if (stats.contains("task")) {
        QJsonObject task = stats["task"].toObject();
        QString taskName = task["name"].toString();
        int progress = task["progress"].toInt(0);
        taskStatusLabel->setText(taskName);
        
        QPropertyAnimation* taskAnim = new QPropertyAnimation(taskProgressBar, "value", this);
        taskAnim->setDuration(500);
        taskAnim->setStartValue(taskProgressBar->value());
        taskAnim->setEndValue(progress);
        taskAnim->setEasingCurve(QEasingCurve::OutCubic);
        connect(taskAnim, &QPropertyAnimation::finished, taskAnim, &QObject::deleteLater);
        taskAnim->start();
    }
}

void MainWindow::updateInventory(const QJsonArray& inventory) {
    inventoryTable->setRowCount(inventory.size());
    
    for (int i = 0; i < inventory.size(); ++i) {
        QJsonObject item = inventory[i].toObject();
        
        QTableWidgetItem* slotItem = new QTableWidgetItem(QString::number(item["slot"].toInt()));
        slotItem->setTextAlignment(Qt::AlignCenter);
        inventoryTable->setItem(i, 0, slotItem);
        
        QTableWidgetItem* nameItem = new QTableWidgetItem(item["name"].toString());
        inventoryTable->setItem(i, 1, nameItem);
        
        QTableWidgetItem* countItem = new QTableWidgetItem(QString::number(item["count"].toInt()));
        countItem->setTextAlignment(Qt::AlignCenter);
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
        QTableWidgetItem* statusItem = new QTableWidgetItem(completed ? "Done" : "In Progress");
        statusItem->setTextAlignment(Qt::AlignCenter);
        tasksTable->setItem(i, 1, statusItem);
        
        int progress = task["progress"].toInt(0);
        QTableWidgetItem* progressItem = new QTableWidgetItem(QString::number(progress) + "%");
        progressItem->setTextAlignment(Qt::AlignCenter);
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
    
    if (!taskName.isEmpty()) {
        taskStatusLabel->setText(taskName);
        QPropertyAnimation* anim = new QPropertyAnimation(taskProgressBar, "value", this);
        anim->setDuration(500);
        anim->setStartValue(taskProgressBar->value());
        anim->setEndValue(progress);
        anim->setEasingCurve(QEasingCurve::OutCubic);
        connect(anim, &QPropertyAnimation::finished, anim, &QObject::deleteLater);
        anim->start();
    } else {
        taskStatusLabel->setText("No active task");
        taskProgressBar->setValue(0);
    }
}

void MainWindow::addCommandToOutput(const QString& command, const QString& response) {
    commandOutput->append("<span style='color: #64ffda;'>&gt; " + command + "</span>");
    commandOutput->append("<span style='color: #a8b2d1;'>" + response + "</span>");
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
    const QString wsUrl = QProcessEnvironment::systemEnvironment().value(
        "BUTTERFLY_WS_URL", "ws://localhost:3000");
    webSocket->open(QUrl(wsUrl));
}

void MainWindow::onWebSocketConnected() {
    reconnectTimer->stop();
    statusBar()->showMessage("Connected");
    QJsonObject empty;
    sendMessage("get_bots", empty);
}

void MainWindow::onWebSocketDisconnected() {
    statusBar()->showMessage("Disconnected, reconnecting...");
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
    } else if (type == "bot_map") {
        updateMap(obj["data"].toObject());
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
        
        QString statusDot;
        if (bot.status == "online") statusDot = QString::fromUtf8("\xF0\x9F\x9F\xA2 ");
        else if (bot.status == "connecting") statusDot = QString::fromUtf8("\xF0\x9F\x9F\xA1 ");
        else if (bot.status == "error") statusDot = QString::fromUtf8("\xF0\x9F\x94\xB4 ");
        else statusDot = QString::fromUtf8("\xE2\x9A\xAA ");
        
        QListWidgetItem* item = new QListWidgetItem(statusDot + bot.name);
        item->setData(Qt::UserRole, bot.id);
        item->setData(Qt::UserRole + 1, bot.status);
        item->setSizeHint(QSize(0, 44));
        item->setToolTip("Server: " + bot.server + ":" + QString::number(bot.port) + 
                        "\nUsername: " + bot.username +
                        "\nStatus: " + bot.status);
        botList->addItem(item);
    }
    
    statusBar()->showMessage(QString::number(botsArray.size()) + " bots loaded");
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
    // Strip status emoji prefix
    for (const QString& prefix : {QString::fromUtf8("\xF0\x9F\x9F\xA2 "), QString::fromUtf8("\xF0\x9F\x9F\xA1 "),
                                   QString::fromUtf8("\xF0\x9F\x94\xB4 "), QString::fromUtf8("\xE2\x9A\xAA ")}) {
        if (dummy.name.startsWith(prefix)) {
            dummy.name = dummy.name.mid(prefix.length());
            break;
        }
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

void MainWindow::setBotOnlineState(bool online) {
    botOnline = online;
    noDataOverlay->setVisible(!online);
    minimap->setNoData(!online);
    
    if (!online) {
        healthBar->setValue(0);
        foodBar->setValue(0);
        healthValueLabel->setText("-- / 20");
        foodValueLabel->setText("-- / 20");
    }
}

void MainWindow::updateMap(const QJsonObject& mapObj) {
    if (mapObj.isEmpty()) return;
    
    MapData data;
    QJsonObject center = mapObj["center"].toObject();
    data.centerX = center["x"].toInt();
    data.centerY = center["y"].toInt();
    data.centerZ = center["z"].toInt();
    
    QJsonObject blocks = mapObj["blocks"].toObject();
    for (auto it = blocks.begin(); it != blocks.end(); ++it) {
        data.blocks[it.key()] = it.value().toString();
    }
    
    QJsonArray entities = mapObj["entities"].toArray();
    for (const QJsonValue& e : entities) {
        QJsonObject eo = e.toObject();
        MapEntity ent;
        ent.name = eo["name"].toString();
        ent.x = eo["x"].toInt();
        ent.y = eo["y"].toInt();
        ent.z = eo["z"].toInt();
        ent.hostile = eo["hostile"].toBool();
        ent.distance = eo["distance"].toInt();
        data.entities.append(ent);
    }
    
    QJsonArray players = mapObj["players"].toArray();
    for (const QJsonValue& p : players) {
        QJsonObject po = p.toObject();
        MapPlayer pl;
        pl.name = po["name"].toString();
        pl.x = po["x"].toInt();
        pl.y = po["y"].toInt();
        pl.z = po["z"].toInt();
        pl.distance = po["distance"].toInt();
        data.players.append(pl);
    }
    
    minimap->setMapData(data);
    minimap->setNoData(false);
    minimap->update();
}

// ==================== MinimapWidget ====================

MinimapWidget::MinimapWidget(QWidget* parent)
    : QWidget(parent), noDataState(true)
{
    setObjectName("minimap");
}

void MinimapWidget::setMapData(const MapData& data) {
    mapData = data;
    update();
}

void MinimapWidget::setNoData(bool noData) {
    noDataState = noData;
    update();
}

QColor MinimapWidget::getBlockColor(const QString& blockName) {
    if (blockName.contains("stone") || blockName.contains("cobble") || blockName.contains("deepslate"))
        return QColor(80, 80, 80);
    if (blockName.contains("dirt") || blockName.contains("grass"))
        return QColor(100, 70, 40);
    if (blockName.contains("water"))
        return QColor(40, 100, 200);
    if (blockName.contains("lava"))
        return QColor(220, 100, 20);
    if (blockName.contains("sand"))
        return QColor(200, 190, 130);
    if (blockName.contains("wood") || blockName.contains("log") || blockName.contains("plank"))
        return QColor(140, 100, 50);
    if (blockName.contains("leaves"))
        return QColor(50, 130, 50);
    if (blockName.contains("iron"))
        return QColor(180, 160, 140);
    if (blockName.contains("gold"))
        return QColor(240, 210, 60);
    if (blockName.contains("diamond"))
        return QColor(80, 220, 240);
    if (blockName.contains("coal"))
        return QColor(40, 40, 40);
    if (blockName.contains("ore"))
        return QColor(120, 100, 80);
    if (blockName.contains("air"))
        return QColor(20, 20, 35);
    if (blockName.contains("bedrock"))
        return QColor(30, 30, 30);
    if (blockName.contains("snow"))
        return QColor(230, 235, 240);
    if (blockName.contains("ice"))
        return QColor(140, 180, 240);
    if (blockName.contains("netherrack"))
        return QColor(100, 30, 30);
    if (blockName.contains("end_stone"))
        return QColor(200, 200, 150);
    return QColor(60, 60, 70);
}

void MinimapWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    QRect rect = this->rect();
    
    // Background
    painter.fillRect(rect, QColor(12, 12, 22));
    
    if (noDataState) {
        painter.setPen(QColor(100, 116, 139));
        painter.setFont(QFont("Segoe UI", 11));
        painter.drawText(rect, Qt::AlignCenter, "No map data");
        return;
    }
    
    const int mapRadius = 24;
    const int gridSize = mapRadius * 2 + 1;
    int cellSize = qMin(rect.width(), rect.height()) / gridSize;
    int offsetX = rect.x() + (rect.width() - cellSize * gridSize) / 2;
    int offsetY = rect.y() + (rect.height() - cellSize * gridSize) / 2;
    
    // Draw grid lines
    drawGrid(painter, QRect(offsetX, offsetY, cellSize * gridSize, cellSize * gridSize));
    
    // Draw blocks
    drawBlocks(painter, QRect(offsetX, offsetY, cellSize * gridSize, cellSize * gridSize), cellSize);
    
    // Draw entities
    drawEntities(painter, QRect(offsetX, offsetY, cellSize * gridSize, cellSize * gridSize), cellSize);
    
    // Draw bot center
    drawBot(painter, QRect(offsetX, offsetY, cellSize * gridSize, cellSize * gridSize));
}

void MinimapWidget::drawGrid(QPainter& painter, const QRect& rect) {
    painter.setPen(QPen(QColor(30, 30, 50, 60), 1));
    int step = rect.width() / 12;
    for (int x = rect.x(); x <= rect.right(); x += step) {
        painter.drawLine(x, rect.y(), x, rect.bottom());
    }
    for (int y = rect.y(); y <= rect.bottom(); y += step) {
        painter.drawLine(rect.x(), y, rect.right(), y);
    }
}

void MinimapWidget::drawBlocks(QPainter& painter, const QRect& rect, int cellSize) {
    int mapRadius = 24;
    int gridSize = mapRadius * 2 + 1;
    
    for (auto it = mapData.blocks.begin(); it != mapData.blocks.end(); ++it) {
        QStringList parts = it.key().split(",");
        if (parts.size() < 3) continue;
        
        int gx = parts[0].toInt();
        int gz = parts[1].toInt();
        int gy = parts[2].toInt();
        
        // Only draw surface layer (skip deep underground)
        if (gy < 4) continue;
        
        int px = rect.x() + (gx * cellSize);
        int py = rect.y() + (gz * cellSize);
        
        QColor color = getBlockColor(it.value());
        painter.fillRect(px, py, cellSize, cellSize, color);
    }
}

void MinimapWidget::drawEntities(QPainter& painter, const QRect& rect, int cellSize) {
    int mapRadius = 24;
    int centerX = rect.x() + rect.width() / 2;
    int centerY = rect.y() + rect.height() / 2;
    
    // Draw mobs
    for (const MapEntity& ent : mapData.entities) {
        int dx = ent.x - mapData.centerX;
        int dz = ent.z - mapData.centerZ;
        if (qAbs(dx) > mapRadius || qAbs(dz) > mapRadius) continue;
        
        int px = centerX + dx * cellSize;
        int py = centerY + dz * cellSize;
        
        QColor color = ent.hostile ? QColor(248, 113, 113) : QColor(167, 139, 250);
        painter.setPen(Qt::NoPen);
        painter.setBrush(color);
        painter.drawEllipse(QPoint(px + cellSize / 2, py + cellSize / 2), 3, 3);
    }
    
    // Draw players
    for (const MapPlayer& pl : mapData.players) {
        int dx = pl.x - mapData.centerX;
        int dz = pl.z - mapData.centerZ;
        if (qAbs(dx) > mapRadius || qAbs(dz) > mapRadius) continue;
        
        int px = centerX + dx * cellSize;
        int py = centerY + dz * cellSize;
        
        painter.setPen(QPen(QColor(96, 165, 250), 2));
        painter.setBrush(QColor(96, 165, 250, 180));
        painter.drawRect(px - 2, py - 2, cellSize + 4, cellSize + 4);
        
        // Player name
        painter.setPen(QColor(200, 220, 255));
        painter.setFont(QFont("Segoe UI", 7));
        painter.drawText(QRect(px - 20, py - 14, 60, 12), Qt::AlignCenter, pl.name);
    }
}

void MinimapWidget::drawBot(QPainter& painter, const QRect& rect) {
    int centerX = rect.x() + rect.width() / 2;
    int centerY = rect.y() + rect.height() / 2;
    
    // Green dot for bot
    painter.setPen(QPen(QColor(74, 222, 128), 2));
    painter.setBrush(QColor(74, 222, 128, 200));
    painter.drawEllipse(QPoint(centerX, centerY), 5, 5);
    
    // Direction indicator
    painter.setPen(QPen(QColor(74, 222, 128, 150), 1));
    painter.drawLine(centerX, centerY, centerX, centerY - 10);
}
