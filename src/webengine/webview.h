#ifndef WEBVIEW_H
#define WEBVIEW_H

#include <QtWebEngineWidgets/QWebEngineView>
#include <QtWebEngineWidgets/QWebEnginePage>

#include <QPrinter>
#include "qplayer.h"
#include "qwk_webpage.h"
#include "fakewebview.h"
#include "qwk_settings.h"

class WebView : public QWebEngineView
{
    Q_OBJECT

public:
    explicit WebView(QWidget* parent = 0);

    void setSettings(QwkSettings *settings);
    QwkSettings* getSettings();

    void loadCustomPage(QString uri);
    void loadHomepage();
    void initSignals();

    void setPage(QwkWebPage* page);
    void resetLoadTimer();
    void stopLoadTimer();

    QWebEngineView *createWindow(QWebEnginePage::WebWindowType type);

    void playSound(QString soundSetting);

    // http://slow-tone.blogspot.com/2011/04/qt-hide-scrollbars-qwebview.html?showComment=1318404188431#c5258624438625837585
    void scrollUp();
    void scrollDown();
    void scrollPageUp();
    void scrollPageDown();
    void scrollHome();
    void scrollEnd();

    void goBack();

    //Compatibility functions for WebKit/WebEngine API compatibility
    QWebEnginePage* mainFrame();
    void applySettings(QwkSettings* settings);
    void hideScrollbars();
    void showScrollbars();
    bool disableTextSelection();

    enum TargetTag {
        HEAD,
        BODY
    };
    void addHTML(QString code, TargetTag appendTo);
    void setPersistentCookies(bool enabled);
    QWidget* getInspector();

    //Slot registry abstractions
    void registerIconChanged(QObject* caller, void (QObject::*handler) (const QIcon&));

public slots:
    void handlePrintRequested();
    void handleFakeviewUrlChanged(const QUrl &);
    void handleFakeviewLoadFinished(bool);

protected:
    void mousePressEvent(QMouseEvent *event);
    QPlayer *getPlayer();
    QWebEngineView *getFakeLoader();
    QTimer *getLoadTimer();

signals:

    void qwkNetworkError(QNetworkReply::NetworkError error, QString message);
    void qwkNetworkReplyUrl(QUrl url);

private:
    QPlayer     *player;
    QwkSettings *qwkSettings;
    FakeWebView *loader;
    QPrinter    *printer;
    QTimer      *loadTimer;

private slots:
	#ifndef QT_NO_SSL
    void forwardPageNetworkErrors(QNetworkReply::NetworkError error, QString message);
    #endif
    void handleWindowCloseRequested();

    void handleAuthReply(const QUrl& aUrl, QAuthenticator* aAuth);
    void handleProxyAuthReply(const QUrl& aUrl, QAuthenticator* aAuth, const QString& proxyHost);

    void handleLoadTimerTimeout();
};

#endif // WEBVIEW_H
