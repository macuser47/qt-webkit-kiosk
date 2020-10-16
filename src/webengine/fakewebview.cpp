#include "fakewebview.h"

#include <QtWebEngineWidgets>

FakeWebView::FakeWebView(QWidget *parent) : QWebEngineView(parent)
{
    this->settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, false);
    this->settings()->setAttribute(QWebEngineSettings::WebGLEnabled, false);
    //this->settings()->setAttribute(QWebEngineSettings::JavaEnabled, false);
    this->settings()->setAttribute(QWebEngineSettings::PluginsEnabled, false);
}

void FakeWebView::setUrl(const QUrl &url)
{
    emit ( urlChanged( url ) );
}


void FakeWebView::load(const QUrl &url)
{
    emit ( urlChanged( url ) );
}

void FakeWebView::load(const QWebEngineHttpRequest& request)
{
    emit ( urlChanged( request.url() ) );
}
