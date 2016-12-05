#ifndef QTXGUI_H
#define QTXGUI_H

#include <QMainWindow>
#include "dialogs/connectionDialog.h"
#include "dialogs/configdialog.h"
#include "ui_QtxGui.h"

//class ConfigDialog;

class QtxGui : public QMainWindow
{
  Q_OBJECT
    
  public:
    QtxGui(QWidget* parent = 0);
    //~QuantServerMainWindow();
    Ui::QtxGui ui;
    void loadSettings();
    void createGUI();
    
    // TODO make modal and set parents 
    //ConnectionDialog *connectionDialog;
    
    ConfigDialog *configDialog;
    
private:
    void createActions();
    void createMenu();
    void createToolBar();
// QMenuBar* menuBar;
// QToolBar* toolBar;
//     void createStatusBar();
    QAction* actionConnectConfig;
    QAction* actionExit;
    QAction* actionAboutQtxMDI;
    QAction* actionAboutQuanticks;
    //QToolBar* toolMenu;
    QMenu* menuFile;
    QMenu* menuSettings;
    QMenu* menuView;
    QMenu* menuHelp;
    
  public slots:
    void save();
    void statusMessage(QString);
    void wakeup();
    void aboutQtxMDI();
    void aboutQuanticks();

private slots:
    void onActionConfigManager();
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
