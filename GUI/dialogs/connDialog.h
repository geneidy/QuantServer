#ifndef CONNDIALOG_H
#define CONNDIALOG_H

#include "ui_connDialog.h"
#include <QDialog>

class ConnDialog : public QDialog
{
    Q_OBJECT
    
public:
    ConnDialog(QWidget* parent = 0);
    Ui::connDialog ui;

    
};

#endif // CONNDIALOG_H
