#ifndef CONNDIALOG_H
#define CONNDIALOG_H

#include "ui_connDialog.h"
#include <QDialog>

class QPushButton;

class ConnDialog : public QDialog
{
    Q_OBJECT
    
public:
    ConnDialog(QWidget* parent = 0);
    Ui::connDialog ui;
    
private slots:
    void browse();
    void enableApplyBtn();
    void disableApplyBtn();
    void passFileHandle();
    void clearLineEditOnCancel();
    
    
private:
    QString fileName; //TODO pass you a pointer...testing right now.


    
};

#endif // CONNDIALOG_H
