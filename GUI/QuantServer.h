#ifndef QUANTSERVER_H
#define QUANTSERVER_H

#include <QMainWindow>


class QuantServer : public QMainWindow
{
  Q_OBJECT
  
  signals:
    void signalLoadSettings();
    void signalShutDown();
    
  public:
    QuantServer();
    void createGUI();
    void loadSettings();
    
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
