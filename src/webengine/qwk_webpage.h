#ifndef QWK_WEBPAGE_H
#define QWK_WEBPAGE_H

#include "qwk_settings.h"

#include <QtWebEngineWidgets/QWebEngineView>

#include <qnetworkreply.h>

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

#ifndef QT_NO_SSL
    bool certificateError(const QWebEngineCertificateError& certificateError);
#endif

#ifndef QT_NO_SSL
signals:
    void qwkNetworkError(QNetworkReply::NetworkError error, QString message);
#endif

};

#endif // QWK_WEBPAGE_H
