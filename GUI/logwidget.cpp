#include "logwidget.h"


void LogWidget::appendMessage(const QString& text)
{
    this->appendPlainText(text); // Adds the message to the widget
    this->verticalScrollBar()->setValue(this->verticalScrollBar()->maximum()); // Scrolls to the bottom
    m_logFile.write(text); // Logs to file
}

QFile m_logFile(QApplication::applicationDirPath() + "../Logs/Log.txt");
