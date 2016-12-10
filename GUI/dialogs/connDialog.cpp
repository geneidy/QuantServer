#include <QString>
#include <QFileDialog>
#include <QPushButton>
#include <QSettings>
#include <QtDebug>

#include "connDialog.h"


//TODO Functionality for everything except find local file & buttonBox not implemented!
/*
 * 1) Apply Btn Behavior: disable after clicking once
 * 2) once file is selected, keep persistant in itchLineEdit
 */

ConnDialog::ConnDialog(QWidget* child)
    : QDialog(child)
{
    ui.setupUi(this);
    ui.buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
    connect(ui.toolButton, SIGNAL(clicked()), this, SLOT(browse()));
    //connect(ui.okBtn, SIGNAL(clicked()), this, SLOT(accepted()));
    
    if (ui.itchLineEdit->text().isEmpty()) {
        if (fileName.isEmpty()) {
           ui.buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
        }
    }
    
    QSettings settings/*(QSettings::UserScope)*/;
    ui.itchLineEdit->setText(settings.value("file_name").toString());
    
    connect(ui.itchLineEdit, SIGNAL(textChanged(const QString &)), this, SLOT(enableApplyBtn()));
    
    //connect(ui.buttonBox.button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(accepted()));
    
    connect(ui.buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()), this, SLOT(passFileHandle()));
    connect(ui.buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(passFileHandle()));
    connect(ui.buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(accept()));
    connect(ui.buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(reject()));
    connect(ui.buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(clearLineEditOnCancel()));
    
    connect(ui.buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()), this, SLOT(disableApplyBtn()));
}

void ConnDialog::browse() {
    fileName = QFileDialog::getOpenFileName(this, "Choose ITCH File", QDir::currentPath(), "gz Files (*.gz)");
    
    ui.itchLineEdit->setText(fileName);
    QSettings settings/*(QSettings::UserScope)*/;
    settings.setValue("file_name", fileName);
}

void ConnDialog::enableApplyBtn() {
    ui.buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
}

void ConnDialog::disableApplyBtn() {
    ui.buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
}

void ConnDialog::passFileHandle() {
    //Pass the file here on clicking ok or accept to settings struct
    qDebug() << "passFileHandle tamam ->" << fileName; 
}

void ConnDialog::clearLineEditOnCancel() {
    /*TODO Need to implement this view logic*/
    ui.itchLineEdit->clear();
    QSettings settings/*(QSettings::UserScope)*/;
    settings.setValue("file_name", "");

}
