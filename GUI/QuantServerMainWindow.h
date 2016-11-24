#ifndef QUANTSERVERMAINWINDOW_H
#define QUANTSERVERMAINWINDOW_H

#include <QMainWindow>
#include "connectionDialog.h"
#include "ui_QuantServerMainWindow.h"

class QuantServerMainWindow : public QMainWindow
{
  Q_OBJECT
    
  public:
    QuantServerMainWindow();
    //~QuantServerMainWindow();
    Ui::QuantServerMainWindow ui;
    
    ConnectionDialog *connectionDialog;
    
  public slots:
    void save();
    void statusMessage(QString);
    void wakeup();
    void shutDown();
    void about();       
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
