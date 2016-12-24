#ifndef QTXGUI_H
#define QTXGUI_H

#include <QMainWindow>
#include "dialogs/connDialog.h"
#include "dialogs/configdialog.h"
#include "widgets/perfWidget/perfwidget.h"
#include "widgets/digitalclock/digitalclock.h"
#include "widgets/resourcesWidget/resourcesWidget.h"
//#include "LED.h"
#include "ui_QtxGui.h"


//class ConfigDialog;

class QPlainTextEdit;

class QtxGui : public QMainWindow
{
  Q_OBJECT
    
  public:
    QtxGui(QWidget* parent = 0);
    //~QuantServerMainWindow();
    Ui::QtxGui ui;

    
    // TODO make modal and set parents 

  private:
    void createActions();
    void createMenu();
    void createToolBars();
    void loadSettings();
    void initGUI();
    void createDockWindows();
    
    ConnDialog* connDialog;
    ConfigDialog* configDialog;
    
    
    QToolBar* mainToolBar;
    QPlainTextEdit* logWindow; //TODO REMOVE AND SUB FOR LOGWINDOW CLASS
    PerfWidget* perfWidget;
    ResourcesWidget* resourcesWidget;
    //DigitalClock clock;declared and defined in cpp
    //LED* led;//declared and defined in cpp
// QMenuBar* menuBar;
// QToolBar* toolBar;
//     void createStatusBar();

    QAction* actionConfig;
    QAction* actionConnect;
    QAction* actionExit;
    QAction* actionAboutQtxMDI;
    QAction* actionAboutQuanticks;
    QAction* actionPlayFeed;
    QAction* actionPauseFeed;
    QAction* actionStopFeed;
    //QToolBar* toolMenu;

    QMenu* menuFile;
    QMenu* menuSettings;
    QMenu* menuView;
    QMenu* menuHelp;
    
  public slots:
    void save();
    void statusMessage(QString, int);
    void wakeup();
    void aboutQtxMDI();
    void aboutQuanticks();

  private slots:
    void onActionConfig();
    void onActionConnect();
    void onActionPlayFeed();
    void onActionPauseFeed();
    void onActionStopFeed();
//     void onExit();
    void shutDown();
};



// class StartStopBtnWidget : public QWidget {
//     
//     Q_OBJECT
//     
// public:
//     StartStopBtnWidget(QWidget *parent = 0);
//     SETTINGS ui_settings;
//     
// public slots:
//     void startServer(); 
//     void stopServer(); 
//     void pauseServer(); 
//     
//     
// private:
//     QPushButton *startBtn;
//     QPushButton *stopBtn;
//     QPushButton *pauseBtn;
// };

#endif // QUANTSERVER_H