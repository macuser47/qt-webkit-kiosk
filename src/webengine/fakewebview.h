#ifndef FAKEWEBVIEW_H
#define FAKEWEBVIEW_H

#include <QtNetwork>

#include <QtWebEngineWidgets/QWebEngineView>

class FakeWebView : public QWebEngineView
{
    Q_OBJECT

public:
    explicit FakeWebView(QWidget* parent = 0);

    void load(const QUrl& url);
    void load(const QWebEngineHttpRequest& request);
    void setUrl(const QUrl &url);

};

#endif // FAKEWEBVIEW_H
