/*
 * Copyright (c) 2011-2015 BlackBerry Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "applicationui.hpp"

#include <bb/cascades/Application>
#include <bb/cascades/Page>
#include <bb/cascades/Container>

#include <src/ServerListItemFactory.h>

using namespace bb::cascades;

ApplicationUI::ApplicationUI() :
        QObject()
{
    this->rootPage = new Page();

    this->invokemanager = new bb::system::InvokeManager();

    bool res = QObject::connect(this->invokemanager,
                                SIGNAL(invoked(const bb::system::InvokeRequest&)),
                                this,
                                SLOT(onInvoke(const bb::system::InvokeRequest&)));

    Q_UNUSED(res);

    switch(this->invokemanager->startupMode())
    {
        case bb::system::ApplicationStartupMode::LaunchApplication:
        case bb::system::ApplicationStartupMode::InvokeApplication:
            this->initAppUI();
            break;

        case bb::system::ApplicationStartupMode::InvokeCard:
            break;

        default:
            this->initAppUI();
            break;
    }
}


void ApplicationUI::initAppUI()
{
    QVariantMap map;
    ServerListItemFactory *serverListItemFactory = new ServerListItemFactory();

    this->list = new ListView();
    Container *appContainer = new Container();
    this->list->setDataModel(&this->listViewDataModel);
    this->list->setListItemProvider(serverListItemFactory);
    this->listViewDataModel.clear();

    map["servername"] = "bajirao";

    this->listViewDataModel << map;

    appContainer->add(this->list);

    this->rootPage->setContent(appContainer);
    Application::instance()->setScene(this->rootPage);

}

void ApplicationUI::initCardUI()
{
    QVariantMap map;

    this->list = new ListView();
    Container *cardContainer = new Container();
    ServerListItemFactory *serverListItemFactory = new ServerListItemFactory();

    this->list->setDataModel(&this->listViewDataModel);
    this->list->setListItemProvider(serverListItemFactory);
    this->listViewDataModel.clear();
    map["servername"] = "ftp.strato.de";

    this->listViewDataModel << map;

    cardContainer->add(this->list);

    this->rootPage->setContent(cardContainer);
    Application::instance()->setScene(this->rootPage);

}
void ApplicationUI::onInvoke(const bb::system::InvokeRequest& data)
{
    Q_UNUSED(data);
    this->initCardUI();
}
