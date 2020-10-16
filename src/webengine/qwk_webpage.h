#ifndef QWK_WEBPAGE_H
#define QWK_WEBPAGE_H

#include "qwk_settings.h"

#include <QtWebEngineWidgets/QWebEngineView>

class QwkWebPage : public QWebEnginePage
{
    Q_OBJECT

public:
    explicit QwkWebPage(QWidget* parent = 0);

    QwkSettings* getSettings();

private:
    qint64  javascriptHangStarted;

/*
public Q_SLOTS:
    bool shouldInterruptJavaScript();
*/

protected:
    void javaScriptConsoleMessage(QWebEnginePage::JavaScriptConsoleMessageLevel level, const QString &message, int lineNumber, const QString &sourceID);
};

#endif // QWK_WEBPAGE_H
