//from: http://stackoverflow.com/questions/14161100/which-qt-widget-should-i-use-for-message-display

class LogWidget : public QPlainTextEdit
{
    Q_OBJECT
/* snip */
public:
    void appendMessage(const QString& text);

private:
    QFile m_logFile;
};


