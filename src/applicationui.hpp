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

#ifndef ApplicationUI_HPP_
#define ApplicationUI_HPP_

#include <stdint.h>

#include <QObject>

#include <bb/cascades/Page>
#include <bb/cascades/NavigationPane>

#include <bb/cascades/QListDataModel>
#include <bb/cascades/ListView>
#include <bb/cascades/Label>

#include <bb/system/InvokeManager>
#include <bb/system/InvokeRequest>

#include <bb/device/DisplayInfo>

using namespace bb::cascades;

#define FTP_PROTOCOL    0
#define SFTP_PROTOCOL   1

/*!
 * @brief Application UI object
 *
 * Use this object to create and init app UI, to create context objects, to register the new meta types etc.
 */
class ApplicationUI : public QObject
{
    Q_OBJECT
public:
    ApplicationUI();
    virtual ~ApplicationUI() {}

public slots:
    void onInvoke(const bb::system::InvokeRequest&);
private slots:
    void addServerPressed();
    void pushFinished(bb::cascades::Page*);
    void popFinished(bb::cascades::Page*);
    void onServerEntryEdit();
    void onServerEntryDelete();
private:
void renderServerListPage(bb::cascades::Page*, bool);
void renderAddServerPage(bb::cascades::Page*, bool, int);
int32_t readAccountInfo();
QVariantListDataModel listViewDataModel;
NavigationPane* navigationPane;
Page *rootPage;
ListView *list;
Label *label;
bb::system::InvokeManager *invokemanager;
bb::device::DisplayInfo *displayInfo;
};

#endif /* ApplicationUI_HPP_ */
