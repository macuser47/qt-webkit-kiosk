#ifndef QWEB_H
#define QWEB_H

#ifdef WEB_ENGINE

#include <QtWebEngineWidgets>

#include "webengine/fakewebview.h"
#include "webengine/webview.h"
#include "webengine/qwk_webpage.h"
#endif

#ifdef WEB_KIT

#include <QtWebKit>

#include "webkit/fakewebview.h"
#include "webkit/webview.h"
#include "webkit/qwk_webpage.h"
#endif

#endif // QWEB_H
