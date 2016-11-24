#ifndef CONNECTIONDIALOG_H
#define CONNECTIONDIALOG_H

#include "ui_connectionDialog.h"
#include <QDialog>

class ConnectionDialog : public QDialog {
  
  Q_OBJECT
  
public:
    ConnectionDialog();
    Ui::connectionDialog ui;
};


#endif // CONNECTIONDIALOG_H
