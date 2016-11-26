#include <QtGui>

#include "configpages.h"

ConfigurationPage::ConfigurationPage(QWidget *parent)
    : QWidget(parent)
{
    QGroupBox *configGroup = new QGroupBox("Server configuration");
    
    QLabel *serverLabel = new QLabel("Server:");
    QComboBox *serverCombo = new QComboBox;
    serverCombo->addItem("Server 1");
    serverCombo->addItem("Server 2");
    serverCombo->addItem("Server 3");
    serverCombo->addItem("Server 4");
    serverCombo->addItem("Server 5");
    
    QHBoxLayout *serverLayout = new QHBoxLayout;
    serverLayout->addWidget(serverLabel);
    serverLayout->addWidget(serverCombo);
    
    QVBoxLayout *configLayout = new QVBoxLayout;
    configLayout->addLayout(serverLayout);
    configGroup->setLayout(configLayout);
    
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(configGroup);
    mainLayout->addStretch(1);
    setLayout(mainLayout);
}

UpdatePage::UpdatePage(QWidget *parent)
    : QWidget(parent)
{
    QGroupBox *updateGroup = new QGroupBox("Package selection");
    QCheckBox *systemCheckBox = new QCheckBox("Update system");
    QCheckBox *appsCheckBox = new QCheckBox("Update applications");
    QCheckBox *docsCheckBox = new QCheckBox("Update documentation");
    
    QGroupBox *packageGroup = new QGroupBox("Existing packages");
    
    QListWidget *packageList = new QListWidget;
    QListWidgetItem *qtItem = new QListWidgetItem(packageList);
    qtItem->setText("Qt");
    QListWidgetItem *qsaItem = new QListWidgetItem(packageList);
    qsaItem->setText("QSA");
    QListWidgetItem *teamBuilderItem = new QListWidgetItem(packageList);
    teamBuilderItem->setText("Teambuilder");
    
    QPushButton *startUpdateButton = new QPushButton("Start update");
    
    QVBoxLayout *updateLayout = new QVBoxLayout;
    updateLayout->addWidget(systemCheckBox);
    updateLayout->addWidget(appsCheckBox);
    updateLayout->addWidget(docsCheckBox);
    updateGroup->setLayout(updateLayout);
    
    QVBoxLayout *packageLayout = new QVBoxLayout;
    packageLayout->addWidget(packageList);
    packageGroup->setLayout(packageLayout);
    
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(updateGroup);
    mainLayout->addWidget(packageGroup);
    mainLayout->addSpacing(12);
    mainLayout->addWidget(startUpdateButton);
    mainLayout->addStretch(1);
    setLayout(mainLayout);
}

QueryPage::QueryPage(QWidget *parent)
    : QWidget(parent)
{
    QGroupBox *packageGroup = new QGroupBox("Look for packages");
    
    QLabel *nameLabel = new QLabel("Name:");
    QLineEdit *nameEdit = new QLineEdit;
    
    QLabel *dateLabel = new QLabel("Released after:");
    QDateTimeEdit *dateEdit = new QDateTimeEdit(QDate::currentDate());
    
    QCheckBox *releaseCheckBox = new QCheckBox("Releases");
    QCheckBox *upgradesCheckBox = new QCheckBox("Upgrades");
    
    QSpinBox *hitsSpinBox = new QSpinBox;
    hitsSpinBox->setPrefix("Return up to ");
    hitsSpinBox->setSuffix(" results");
    hitsSpinBox->setSpecialValueText("Return only the first result");
    hitsSpinBox->setMinimum(1);
    hitsSpinBox->setMaximum(100);
    hitsSpinBox->setSingleStep(10);
    
    QPushButton *startQueryButton = new QPushButton("Start query");
    
    QGridLayout *packagesLayout = new QGridLayout;
    packagesLayout->addWidget(nameLabel, 0, 0);
    packagesLayout->addWidget(nameEdit, 0, 1);
    packagesLayout->addWidget(dateLabel, 1, 0);
    packagesLayout->addWidget(dateEdit, 1, 1);
    packagesLayout->addWidget(releaseCheckBox, 2, 0);
    packagesLayout->addWidget(upgradesCheckBox, 3, 0);
    packagesLayout->addWidget(hitsSpinBox, 4, 0, 1, 2);
    packageGroup->setLayout(packagesLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(packageGroup);
    mainLayout->addSpacing(12);
    mainLayout->addWidget(startQueryButton);
    mainLayout->addStretch(1);
    setLayout(mainLayout);
}
