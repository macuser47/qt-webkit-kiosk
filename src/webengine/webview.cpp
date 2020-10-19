#include "config.h"

#include <QtDebug>
#include <QtGui>
#include "webview.h"
#include <signal.h>
#include "unixsignals.h"

#include <QtWebEngineWidgets>

#ifdef QT5
#include <QNetworkReply>
#ifndef QT_NO_SSL
#include <QSslError>
#endif
#endif


WebView::WebView(QWidget* parent): QWebEngineView(parent)
{
    player = NULL;
    loader = NULL;
    loadTimer = NULL;
}

/**
 * Call after setPage
 * @brief WebView::initSignals
 */
void WebView::initSignals()
{
    #ifndef QT_NO_SSL
    connect(page(),
            SIGNAL(qwkNetworkError(QNetworkReply::NetworkError, QString)),
            this,
            SLOT(forwardPageNetworkErrors(QNetworkReply::NetworkError, QString)));
    #endif

    connect(page(),
            SIGNAL(windowCloseRequested()),
            this,
            SLOT(handleWindowCloseRequested()));

    connect(page(),
            SIGNAL(printRequested()),
            this,
            SLOT(handlePrintRequested()));

    /*handle auth request to be in stable state*/
    connect(page(),
            SIGNAL(authenticationRequired(const QUrl&, QAuthenticator*)),
            this,
            SLOT(handleAuthReply(const QUrl&, QAuthenticator*)));

    /*handle proxy auth request to be in stable state*/
    connect(page(),
            SIGNAL(proxyAuthenticationRequired(const QUrl&, QAuthenticator*, const QString&)),
            this,
            SLOT(handleProxyAuthReply(const QUrl&, QAuthenticator*, const QString&)));

}

void WebView::setPage(QwkWebPage *page)
{
    QString ua = qwkSettings->getQString("browser/custom_user_agent_header");
    if (ua.length()) {
        page->profile()->setHttpUserAgent(ua);
    }

    QWebEngineView::setPage(page);
    initSignals();
}

/*TODO: merge this with applySettings*/
void WebView::setSettings(QwkSettings *settings)
{
    qwkSettings = settings;

    if (qwkSettings->getBool("printing/enable")) {
        if (!qwkSettings->getBool("printing/show-printer-dialog")) {
            if (!printer) {
                printer = new QPrinter();
            }
            printer->setPrinterName(qwkSettings->getQString("printing/printer"));
            printer->setPageMargins(
                qwkSettings->getReal("printing/page_margin_left"),
                qwkSettings->getReal("printing/page_margin_top"),
                qwkSettings->getReal("printing/page_margin_right"),
                qwkSettings->getReal("printing/page_margin_bottom"),
                QPrinter::Millimeter
            );
        }
    }

}

QwkSettings* WebView::getSettings()
{
    return qwkSettings;
}

void WebView::loadHomepage()
{
    loadCustomPage(qwkSettings->getQString("browser/homepage"));
}

void WebView::loadCustomPage(QString uri)
{
    QFileInfo finfo = QFileInfo();
    finfo.setFile(uri);

    qDebug() << "Page: check local file = " <<
                uri <<
                ", absolute path = " <<
                finfo.absoluteFilePath() <<
                ", local uri = " <<
                QUrl::fromLocalFile(
                    uri
                ).toString();

    if (finfo.isFile()) {
        qDebug() << "Page: Local FILE";
        this->stop();
        this->load(QUrl::fromLocalFile(
            finfo.absoluteFilePath()
        ));
    } else {
        qDebug() << "Page: Remote URI";
        this->stop();
        this->load(QUrl(uri));
    }
    if (this->getLoadTimer()) {
        this->getLoadTimer()->start(qwkSettings->getInt("browser/page_load_timeout"));
    }
}

#ifndef QT_NO_SSL
void WebView::forwardPageNetworkErrors(QNetworkReply::NetworkError error, QString message)
{
    emit qwkNetworkError(error, message);
}
#endif

void WebView::handleAuthReply(const QUrl& aUrl, QAuthenticator* aAuth)
{
    Q_UNUSED(aUrl)
    if( aAuth ) {
        qDebug() << QDateTime::currentDateTime().toString() << "handleAuthReply, need authorization, do nothing for now";
        emit qwkNetworkError(QNetworkReply::AuthenticationRequiredError, QString("Web-site need authorization! Nothing to do for now :("));
    }
}

void WebView::handleProxyAuthReply(const QUrl& aUrl, QAuthenticator* aAuth, const QString& proxyHost)
{
    Q_UNUSED(aUrl);
    Q_UNUSED(proxyHost);
    if( aAuth ) {
        qDebug() << QDateTime::currentDateTime().toString() << "handleProxyAuthReply, need proxy authorization, do nothing for now";
        emit qwkNetworkError(QNetworkReply::AuthenticationRequiredError, QString("Proxy need authorization! Check your proxy auth settings!"));
    }
}

void WebView::handleWindowCloseRequested()
{
    qDebug() << QDateTime::currentDateTime().toString() << "Handle windowCloseRequested: " << qwkSettings->getQString("browser/show_homepage_on_window_close");
    if (qwkSettings->getBool("browser/show_homepage_on_window_close")) {
        qDebug() << "-- load homepage";
        loadHomepage();
    } else {
        qDebug() << "-- exit application";
        UnixSignals::signalHandler(SIGINT);
    }
}

void WebView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        qDebug() << QDateTime::currentDateTime().toString() << "Window Clicked!";
        playSound("event-sounds/window-clicked");
    }
    QWebView::mousePressEvent(event);
}


/**
 * @brief WebView::getFakeLoader
 * Fake window handler for 'window.open()' and '_blank' target
 * Just need to catch url setup
 * @return FakeWebView
 */
QWebView *WebView::getFakeLoader()
{
    if (!loader) {
        qDebug() << QDateTime::currentDateTime().toString() << "New fake webview loader";
        loader = new FakeWebView(this);
        loader->hide();
        QWebPage *newWeb = new QWebPage(loader);
        loader->setPage(newWeb);

        connect(loader, SIGNAL(urlChanged(const QUrl&)), SLOT(handleFakeviewUrlChanged(const QUrl&)));
        connect(loader, SIGNAL(loadFinished(bool)), SLOT(handleFakeviewLoadFinished(bool)));
    }
    return loader;
}

void WebView::handleFakeviewUrlChanged(const QUrl &url)
{
    qDebug() << QDateTime::currentDateTime().toString() << "WebView::handleFakeviewUrlChanged";
    Q_UNUSED(url);
    if (loader) {
        loader->stop();
    }
}

void WebView::handleFakeviewLoadFinished(bool ok)
{
    qDebug() << QDateTime::currentDateTime().toString() << "WebView::handleFakeviewLoadFinished: ok=" << (int)ok;
    if (loader) {
        QUrl url = loader->url();
        if (this->url().toString() != url.toString()) {
            qDebug() << "url Changed!" << url.toString();
            this->loadCustomPage(url.toString());
            qDebug() << "-- load url";
        }
    }
}


QPlayer *WebView::getPlayer()
{
    if (qwkSettings->getBool("event-sounds/enable")) {
        if (player == NULL) {
            player = new QPlayer();
        }
    }
    return player;
}


QTimer *WebView::getLoadTimer()
{
    if (qwkSettings->getUInt("browser/page_load_timeout")) {
        if (loadTimer == NULL) {
            loadTimer = new QTimer();
            connect(loadTimer, SIGNAL(timeout()), SLOT(handleLoadTimerTimeout()));
        }
    }
    return loadTimer;
}

void WebView::handleLoadTimerTimeout()
{
    loadTimer->stop();
    this->stop();
    if (loader) {
        loader->stop();
    }
    emit qwkNetworkError(QNetworkReply::TimeoutError, QString("Page load timed out! Connection problems?"));
}


/**
 * @brief WebView::resetLoadTimer
 * If page loading, progress event emited
 * And we reset timer
 * If page not loading - timer will be triggered
 */
void WebView::resetLoadTimer()
{
    if (getLoadTimer()) {
        getLoadTimer()->stop();
        getLoadTimer()->start(qwkSettings->getInt("browser/page_load_timeout"));
    }
}


/**
 * @brief WebView::stopLoadTimer
 * If page loaded, we stop timer
 */
void WebView::stopLoadTimer()
{
    if (getLoadTimer()) {
        getLoadTimer()->stop();
    }
}


void WebView::playSound(QString soundSetting)
{
    if (getPlayer()) {
        QString sound = qwkSettings->getQString(soundSetting);
        QFileInfo finfo = QFileInfo();
        finfo.setFile(sound);
        if (finfo.exists()) {
            qDebug() << "Play sound: " << sound;
            getPlayer()->play(sound);
        } else {
            qDebug() << "Sound file '" << sound << "' not found!";
        }
    }
}

QWebView *WebView::createWindow(QWebPage::WebWindowType type)
{
    if (type != QWebPage::WebBrowserWindow) {
        return NULL;
    }
    qDebug() << QDateTime::currentDateTime().toString() << "Handle createWindow...";

    return getFakeLoader();
}

void WebView::handlePrintRequested(/*QWebFrame *wf*/)
{
    qDebug() << QDateTime::currentDateTime().toString() << "Handle printRequested...";
    if (qwkSettings->getBool("printing/enable")) {
        if (!qwkSettings->getBool("printing/show-printer-dialog")) {
            if (printer->printerState() != QPrinter::Error) {
                qDebug() << "... got printer, try use it";
                wf->print(printer);
            }
        }
    }
}

void WebView::scrollDown()
{
    QWebFrame* frame = this->page()->mainFrame();
    QPoint point = frame->scrollPosition();
    frame->setScrollPosition(point + QPoint(0, 100));
}

void WebView::scrollPageDown()
{
    QWebFrame* frame = this->page()->mainFrame();
    QPoint point = frame->scrollPosition();
    frame->setScrollPosition(point + QPoint(0, this->page()->mainFrame()->geometry().height()));
}

void WebView::scrollEnd()
{
    QWebFrame* frame = this->page()->mainFrame();
    frame->setScrollPosition(QPoint(0, frame->contentsSize().height()));
}

void WebView::scrollUp()
{
    QWebFrame* frame = this->page()->mainFrame();
    QPoint point = frame->scrollPosition();
    frame->setScrollPosition(point - QPoint(0, 100));
}

void WebView::scrollPageUp()
{
    QWebFrame* frame = this->page()->mainFrame();
    QPoint point = frame->scrollPosition();
    frame->setScrollPosition(point - QPoint(0, this->page()->mainFrame()->geometry().height()));
}

void WebView::scrollHome()
{
    QWebFrame* frame = this->page()->mainFrame();
    frame->setScrollPosition(QPoint(0, 0));
}

QWebEnginePage WebView::mainFrame()
{
    return page();
}

void WebView::applySettings(QwkSettings* qwkSettings)
{
    settings()->setAttribute(QWebEngineSettings::JavascriptEnabled,
        qwkSettings->getBool("browser/javascript")
    );

    settings()->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows,
        qwkSettings->getBool("browser/javascript_can_open_windows")
    );

    settings()->setAttribute(QWebEngineSettings::WebGLEnabled,
        qwkSettings->getBool("browser/webgl")
    );

    settings()->setAttribute(QWebEngineSettings::PluginsEnabled,
        qwkSettings->getBool("browser/plugins")
    );

    settings()->setAttribute(QWebEngineSettings::LocalStorageEnabled,
        qwkSettings->getBool("localstorage/enable")
    );

#if QT_VERSION >= 0x050400
    settings()->setAttribute(QWebEngineSettings::Accelerated2dCanvasEnabled, true);
#endif

    settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls,
        qwkSettings->getBool("security/local_content_can_access_remote_urls")
    );

}

void WebView::hideScrollbars()
{
#if QT_VERSION >= 0x051000
    settings()->setAttribute(QWebEngineSettings::ShowScrollbars, false);
#else
    qDebug() << "Scrollbar hiding unsupported for Qt<5.10 with WebEngine.";
#endif
}

void WebView::showScrollbars()
{
#if QT_VERSION >= 0x051000
    settings()->setAttribute(QWebEngineSettings::ShowScrollbars, true);
#endif
}

bool WebView::disableTextSelection()
{
    QWebEngineScript* injectionScript = new QWebEngineScript();
    QString code = "\
var style = document.createElement('style');\
style.innerHtml = '\
        body, div, p, span, h1, h2, h3, h4, h5, h6, caption, td, li, dt, dd {\
        {\
          -moz-user-select: none;\
          -khtml-user-select: none;\
          -webkit-user-select: none;\
          user-select: none;\
        }\
';\
document.head.appendChild(style);\
    ";
    injectionScript->setSourceCode(code);
    page()->scripts().insert(*injectionScript);
    return true;
}

void WebView::addHTML(QString content, TargetTag appendTo)
{
    QWebEngineScript* injectionScript = new QWebEngineScript();
    QString code;
    code += "var content='" + content + "';";
    switch (appendTo) {
    case TargetTag::BODY:
        code += "document.body.insertAdjacentHTML('beforeend', content);";
        break;
    case TargetTag::HEAD:
        code += "document.head.inssertAdjacentHTML('beforeend', content);";
        break;
    }

    injectionScript->setSourceCode(code);
    page()->scripts().insert(*injectionScript);
}
