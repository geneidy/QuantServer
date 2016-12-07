#define ORG_NAME "Quanticks"
#define DOM_NAME "quanticks.com"
#define APP_NAME "Quanticks MDI Server"
#define VERSION_NUM "0.0.1"

#include "QtxGui.h"
#include "../Include/Settings.h"

#include <QtDebug>
#include <QApplication>
#include <QPlainTextEdit> //TODO remove when adding logwidget; for testing purposes now!!
#include <QSettings>
#include <QStatusBar>
#include <QInputDialog>
#include <QToolButton>
#include <QToolBar>
#include <QDockWidget>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QLayout>

//#include <QProcess>

QtxGui::QtxGui(QWidget* parent)
    : QMainWindow(parent)
{    
    ui.setupUi(this);
    
    QCoreApplication::setOrganizationName(ORG_NAME);
    QCoreApplication::setOrganizationDomain(DOM_NAME);
    QCoreApplication::setApplicationName(APP_NAME);
    QCoreApplication::setApplicationVersion(VERSION_NUM);
    
    setWindowTitle(APP_NAME);
    loadSettings();
    
    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(shutDown()));
    //connect(ui.action_Quit, SIGNAL(triggered()), qApp, SLOT(quit()));
    
    /*
    connectionDialog = new ConnectionDialog();
    connect(ui.actionConnect, SIGNAL(triggered()), connectionDialog, SLOT(show())); 
    */
    
   //connect(ui.actionConfigure_QuantServer, SIGNAL(triggered()), this, SLOT(onActionConfigManager()));
    //connect(ui.actionConfigure_QuantServer, SIGNAL(triggered()), configDialog, SLOT(show()));
    
    
   // connect(ui.action_About, SIGNAL(triggered()), this, SLOT(about()));
    
    createActions();
    createMenu();
    createToolBars();
    statusMessage("Ready", 2000);
    createDockWindows();
//     createStatusBar();
//     
//     //statusBar()->showMessage("Ready", 2000);
}
/////////////////////////////////////////////////////////////////////
void QtxGui::shutDown() {
    this->hide();
    save();
    QCoreApplication::quit();
}
/////////////////////////////////////////////////////////////////////
/* TODO IMPLEMENT THIS!!!
void QtxGui::initGUI() {
    QMenuBar *menuBar = new QMenuBar();
    setMenuBar(menuBar);
    QMenu *menu = new QMenu("&Help");
    
    QAction* a = new QAction("&About", this);
    a->setIconVisibleInMenu(true);
    connect(a, SIGNAL(activated()), this, SLOT(about()));
    menu->addAction(a);
    menuBar->addMenu(menu);
    
    statusBar()->showMessage("Ready", 2000);
    
}*/
/////////////////////////////////////////////////////////////////////
void QtxGui::loadSettings() {
    QSettings settings(ORG_NAME, APP_NAME);
    
    restoreGeometry(settings.value("main_window_geometry").toByteArray());
    
    restoreState(settings.value("main_window_state").toByteArray());
    
    // restore the size of the app
    QSize sz = settings.value("main_window_size", QSize(500, 300)).toSize();
    resize(sz);
    
    // restore the position of the app
    QPoint p = settings.value("main_window_pos", QPoint(0, 0)).toPoint();
    move(p);
}
/////////////////////////////////////////////////////////////////////
void QtxGui::save() {
    QSettings settings(ORG_NAME, APP_NAME);
    
    settings.setValue("main_window_geometry", saveGeometry());
    settings.setValue("main_window_state", saveState());
    settings.setValue("main_window_size", size());
    settings.setValue("main_window_pos", pos());
    //settings.setValue("Connect", saveState());
}
/////////////////////////////////////////////////////////////////////
void QtxGui::statusMessage(QString d, int milsec) {
    // update the status bar with a new message from somewhere
    statusBar()->showMessage(d, milsec);
    wakeup();
}
/////////////////////////////////////////////////////////////////////
void QtxGui::wakeup() {
    // force app to process the event que to keep from blocking
    qApp->processEvents();
}
/////////////////////////////////////////////////////////////////////
void QtxGui::aboutQtxMDI() {
    QMessageBox::information(this, "Quanticks", " Market Data Infrastructure (MDI) Server");
}
/////////////////////////////////////////////////////////////////////
void QtxGui::aboutQuanticks() {
    QMessageBox::information(this, "Quanticks", " WE THE BEST");
}
/////////////////////////////////////////////////////////////////////
void QtxGui::createActions() {
    
    //TODO QIcon Fallback 
    actionConfig = new QAction(QIcon::fromTheme(QString::fromUtf8("preferences-system-network")), "Con&fig", this);
    connect(actionConfig, SIGNAL(triggered(bool)), this, SLOT(onActionConfig()));
    
    actionConnect = new QAction(QIcon::fromTheme(QString::fromUtf8("network-wired")), "&Connect", this);
    connect(actionConnect, SIGNAL(triggered()), this, SLOT(onActionConnect()));
    
    actionExit =  new QAction(QIcon::fromTheme(QString::fromUtf8("application-exit")), "E&xit", this);
    actionExit->setShortcut(QKeySequence::Quit);
    connect(actionExit, SIGNAL(triggered()), this, SLOT(shutDown()));
    
    actionAboutQtxMDI = new QAction(QIcon::fromTheme(QString::fromUtf8("help-about")), "About Quant MDI", this);
    connect(actionAboutQtxMDI, SIGNAL(triggered()), this, SLOT(aboutQtxMDI()));
    
    actionAboutQuanticks = new QAction(QIcon::fromTheme(QString::fromUtf8("help-about")), "About Quanticks", this);
    connect(actionAboutQuanticks, SIGNAL(triggered()), this, SLOT(aboutQuanticks()));
    
    actionPlayFeed = new QAction(QIcon::fromTheme(QString::fromUtf8("media-playback-start")), "Play Feed", this);
    connect(actionPlayFeed, SIGNAL(triggered()), this, SLOT(onActionPlayFeed()));
    
    actionPauseFeed = new QAction(QIcon::fromTheme(QString::fromUtf8("media-playback-pause")), "Pause Feed", this);
    connect(actionPauseFeed, SIGNAL(triggered()), this, SLOT(onActionPauseFeed()));
    
    actionStopFeed = new QAction(QIcon::fromTheme(QString::fromUtf8("media-playback-stop")), "Stop Feed", this);
    connect(actionStopFeed, SIGNAL(triggered()), this, SLOT(onActionStopFeed()));
    
}
/////////////////////////////////////////////////////////////////////
void QtxGui::createMenu() {
    menuFile = menuBar()->addMenu("&File");
    menuFile->addAction(actionConnect);
    menuFile->addSeparator();
    menuFile->addAction(actionExit);
    
    menuSettings = menuBar()->addMenu("&Settings");
    menuSettings->addAction(actionConfig);
    menuSettings->addSeparator();
    
    menuView = menuBar()->addMenu("&View");
    
    menuHelp = menuBar()->addMenu("Help");
    menuHelp->addSeparator();
    menuHelp->addAction(actionAboutQtxMDI);
    menuHelp->addAction(actionAboutQuanticks);
    
    
}
/////////////////////////////////////////////////////////////////////
void QtxGui::createToolBars() {
    connectToolBar = addToolBar("Connect");
    connectToolBar->addAction(actionConnect);
    connectToolBar->addSeparator();
    connectToolBar->addAction(actionPlayFeed);
    connectToolBar->addAction(actionPauseFeed);
    connectToolBar->addAction(actionStopFeed);
    connectToolBar->addSeparator();
    connectToolBar->setObjectName("ConnectToolBar");
}
/////////////////////////////////////////////////////////////////////
void QtxGui::createDockWindows() {
    QDockWidget* dock0 = new QDockWidget("Log", this);
    //dock0->setAllowedAreas(Qt::BottomDockWidgetArea);
    logWindow = new QPlainTextEdit(dock0);
    logWindow->setReadOnly(true);
    dock0->setWidget(logWindow);
    addDockWidget(Qt::BottomDockWidgetArea, dock0);
    menuView->addAction(dock0->toggleViewAction());
    dock0->setObjectName("Log");
    
    //TODO Set default size
    QDockWidget* dock1 = new QDockWidget("System Performance Indicator", this);
    //dock1->setAllowedAreas(Qt::RightDockWidgetArea);
    perfWidget = new PerfWidget(dock1);
    dock1->setWidget(perfWidget);
    addDockWidget(Qt::RightDockWidgetArea, dock1);
    menuView->addAction(dock1->toggleViewAction());
    dock1->setObjectName("SysPerfIndicator");
}
/////////////////////////////////////////////////////////////////////
void QtxGui::onActionConfig() {
    configDialog = new ConfigDialog(this);
    //configDialog->setWindowFlags(configDialog->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    //configDialog->setParent(this);
    configDialog->show();
    
}

void QtxGui::onActionConnect() {
    connDialog = new ConnDialog(this);
    connDialog->show();
}
/////////////////////////////////////////////////////////////////////
void QtxGui::onActionPlayFeed() {
    
}
/////////////////////////////////////////////////////////////////////
void QtxGui::onActionPauseFeed() {
    
}
/////////////////////////////////////////////////////////////////////
void QtxGui::onActionStopFeed() {
    
}
/////////////////////////////////////////////////////////////////////

// void QtxGui::createStatusBar() {
//     statusMessage("Ready");
//     //statusBar()->showMessage("Ready", 2000);
// }

// void QtxGui::onExit() {
//     shutDown();
// }













// #include "QuantServer.h"
// #include <QVBoxLayout>
// #include <QHBoxLayout>
// 
// StartStopBtnWidget::StartStopBtnWidget(QWidget *parent)
//     : QWidget(parent) {
//         
//     QVBoxLayout *vbox = new QVBoxLayout(this);
//     QHBoxLayout *hbox = new QHBoxLayout();
//     
//     startBtn = new QPushButton("Start", this);
//     stopBtn = new QPushButton("Stop", this);
//     pauseBtn = new QPushButton("Paused", this);
//     
//     hbox->addWidget(startBtn, 1, Qt::AlignRight);
//     hbox->addWidget(stopBtn, 0);
//     hbox->addWidget(pauseBtn, 0);
//     
//     vbox->addStretch(1);
//     vbox->addLayout(hbox);
//     
//     connect(startBtn, SIGNAL(clicked()), this, SLOT(startServer()));
//     
// }
// 
// void StartStopBtnWidget::startServer() {
//     ui_settings.start_stop_pause = 1;
// }
// 
// void StartStopBtnWidget::stopServer() {
//     ui_settings.start_stop_pause = 0;
// }
// 
// void StartStopBtnWidget::pauseServer() {
//     ui_settings.start_stop_pause = 2;
// }
