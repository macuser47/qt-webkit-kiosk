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

    static QString userAgent;

    //Needed for API compatibility with WebKit
    QwkWebPage* mainFrame();

private:
    qint64  javascriptHangStarted;

/*
public Q_SLOTS:
    bool shouldInterruptJavaScript();
*/

protected:
    void javaScriptConsoleMessage(QWebEnginePage::JavaScriptConsoleMessageLevel level, const QString &message, int lineNumber, const QString &sourceID);

    //TODO: replace with https://doc.qt.io/qt-5/qwebengineprofile.html
    //virtual QString userAgentForUrl(const QUrl & url) const;

};

#endif // QWK_WEBPAGE_H
