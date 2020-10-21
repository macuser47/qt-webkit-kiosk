/****************************************************************************
**
** Copyright (C) 2011 Sergey Dryabzhinsky
** All rights reserved.
** Contact: Sergey Dryabzhinsky (sergey.dryabzhinsky@gmail.com)
**
** This file is part of the examples of the Qt Toolkit.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QApplication>
#include "mainwindow.h"
#include "anyoption.h"

#ifdef WEB_ENGINE
QString getChromiumVersion()
{
    QString version;
    QString user_agent = QWebEngineProfile::defaultProfile()->httpUserAgent();
    for(const QString & text : user_agent.split(" ")){
        if(text.startsWith(QStringLiteral("Chrome/"))){
        version = text.mid(QStringLiteral("Chrome/").length());
        }
    }
    return version;
}
#endif

void preconfigure()
{
    //Enable inspector in WebEngine.
    //Needs to be set before any other WebEngine library calls.
    //May not work on windows?
#ifdef WEB_ENGINE
    qputenv("QTWEBENGINE_REMOTE_DEBUGGING", "2222");
#endif
}

bool setupOptions(AnyOption *cmdopts)
{
    cmdopts->addUsage("This is a simple web-browser working in fullscreen kiosk-mode."); cmdopts->addUsage("");
    cmdopts->addUsage("Usage: ");
    cmdopts->addUsage("");
    cmdopts->addUsage(" -h --help                           Print usage and exit");
    cmdopts->addUsage(" -v --version                        Print version and exit");
    cmdopts->addUsage(" -c --config options.ini             Configuration INI-file");
    cmdopts->addUsage(" -u --uri http://www.example.com/    Open this URI, home page");
    cmdopts->addUsage(" -C --clear-cache                    Clear cached request data");
    cmdopts->addUsage("");
    cmdopts->addUsage("Build with:");
    cmdopts->addUsage("        Qt: " QT_VERSION_STR);
#ifdef WEB_KIT
    cmdopts->addUsage("    WebKit: library v" QTWEBKIT_VERSION_STR);
#endif
#ifdef WEB_ENGINE
    cmdopts->addUsage(" WebEngine: library v" QTWEBENGINECORE_VERSION_STR);
#endif
    cmdopts->addUsage("");
    cmdopts->addUsage("Runing with:");

    QString qtv = QString("         Qt: ") + QString(qVersion());
    QByteArray qtvb = qtv.toLocal8Bit();
    const char * qtvch = qtvb.constData();
    cmdopts->addUsage(qtvch);

#ifdef WEB_KIT
    QString qtwv = QString("     WebKit: engine v") + qWebKitVersion();
    QByteArray qtwvb = qtwv.toLocal8Bit();
    const char * qtwvch = qtwvb.constData();
    cmdopts->addUsage(qtwvch);
    cmdopts->addUsage("");
#endif
#ifdef WEB_ENGINE
    QString qtwv = QString("  WebEngine: chromium v") + getChromiumVersion();
    QByteArray qtwvb = qtwv.toLocal8Bit();
    const char * qtwvch = qtwvb.constData();
    cmdopts->addUsage(qtwvch);
    cmdopts->addUsage("");
#endif

#ifdef WEB_KIT
    qDebug() << "Build with: Qt = " QT_VERSION_STR << "; WebKit = " QTWEBKIT_VERSION_STR;
    qDebug() << "Runing with: Qt =" << qVersion() << "; WebKit =" << qWebKitVersion();
#endif
#ifdef WEB_ENGINE
    qDebug() << "Build with: Qt = " QT_VERSION_STR << "; WebEngine = " QTWEBENGINECORE_VERSION_STR;
    qDebug() << "Runing with: Qt =" << qVersion() << "; WebEngine = chromium" <<  getChromiumVersion();
#endif

    cmdopts->setFlag("help", 'h');
    cmdopts->setFlag("version", 'v');
    cmdopts->setFlag("clear-cache", 'C');

    cmdopts->setOption("config", 'c');
    cmdopts->setOption("uri", 'u');

    cmdopts->setVersion(VERSION);

#ifdef QT5
    cmdopts->processCommandArgs( QCoreApplication::arguments().length(), QCoreApplication::arguments() );
#else
    cmdopts->processCommandArgs( QCoreApplication::argc(), QCoreApplication::argv() );
#endif

    if (cmdopts->getFlag('h') || cmdopts->getFlag("help")) {
        qDebug(">> Help option in command prompt...");
        cmdopts->printUsage();
        return false;
    }

    if (cmdopts->getFlag('v') || cmdopts->getFlag("version")) {
        qDebug(">> Version option in command prompt...");
        cmdopts->printVersion();
        return false;
    }

    return true;
}

int main(int argc, char * argv[])
{

    QApplication app(argc, argv);

    preconfigure();

    AnyOption *cmdopts = new AnyOption();
    if (!setupOptions(cmdopts)) {
        return 0;
    }

    MainWindow *browser = new MainWindow();
    browser->init(cmdopts);

    // executes browser.cleanupSlot() when the Qt framework emits aboutToQuit() signal.
    QObject::connect(qApp, SIGNAL(aboutToQuit()),
                     browser, SLOT(cleanupSlot()));

    int ret = app.exec();
    qDebug() << "Application return: " << ret;
}
