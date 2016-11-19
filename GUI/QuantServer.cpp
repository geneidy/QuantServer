#define ORG_NAME "Quanticks"
#define DOM_NAME "quanticks.com"
#define APP_NAME "Quanticks MDI Server"

#include <QtDebug>
#include <QApplication>
#include <QSettings>
#include <QStatusBar>
#include <QInputDialog>
#include <QToolButton>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QLayout>
//#include <QProcess>

#include "QuantServer.h"
#include "../Include/Settings.h"

QuantServer::QuantServer() {
    
    QCoreApplication::setOrganizationName(ORG_NAME);
    QCoreApplication::setOrganizationDomain(DOM_NAME);
    QCoreApplication::setApplicationName(APP_NAME);
    
    createGUI();
    loadSettings();
    
    setWindowTitle(APP_NAME);
}

void QuantServer::shutDown() {
    save();
}

void QuantServer::createGUI() {
    QMenuBar *menuBar = new QMenuBar();
    setMenuBar(menuBar);
    QMenu *menu = new QMenu("&Help");
    
    QAction* a = new QAction("&About", this);
    a->setIconVisibleInMenu(true);
    connect(a, SIGNAL(activated()), this, SLOT(about()));
    menu->addAction(a);
    menuBar->addMenu(menu);
    
    statusBar()->showMessage("Ready", 2000);
    
}

void QuantServer::loadSettings() {
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

void QuantServer::save() {
    QSettings settings(ORG_NAME, APP_NAME);
    
    settings.setValue("main_window_geometry", saveGeometry());
    settings.setValue("main_window_state", saveState());
    settings.setValue("main_window_size", size());
    settings.setValue("main_window_pos", pos());
}

void QuantServer::statusMessage(QString d) {
    // update the status bar with a new message from somewhere
    statusBar()->showMessage(d, 0);
    wakeup();
}

void QuantServer::wakeup() {
    // force app to process the event que to keep from blocking
    qApp->processEvents();
}

void QuantServer::about() {
    QMessageBox::information(this, "Quanticks!", " Market Data Infrastructure (MDI) Server");
}













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
