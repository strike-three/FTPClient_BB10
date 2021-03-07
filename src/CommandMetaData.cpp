/*
 * CommandMetaData.cpp
 *
 *  Created on: Feb 27, 2021
 *      Author: aranade
 */

#include <src/CommandMetaData.h>


CommandMetaData::CommandMetaData()
{
    this->initCommandMetaData();

}

CommandMetaData::~CommandMetaData()
{
    // TODO Auto-generated destructor stub
}

void CommandMetaData::initCommandMetaData()
{
    this->url.clear();
    this->uname.clear();
    this->password.clear();
    this->port = 0;

    this->sequenceId = SEQUENCE_NONE;
    this->sequence = 0;
    this->listPath.clear();

    this->error = false;
    this->errorString.clear();

    this->ioDevice = NULL;
}

void CommandMetaData::setUrl(QString url)
{
    this->url = url;
}

QString CommandMetaData::getUrl()
{
    return this->url;
}

void CommandMetaData::setUname(QString uname)
{
    this->uname = uname;
}

QString CommandMetaData::getUName()
{
    return this->uname;
}

void CommandMetaData::setPassword(QString password)
{
    this->password = password;
}

QString CommandMetaData::getPassword()
{
    return this->password;
}

void CommandMetaData::setPort(int32_t port)
{
    this->port = port;
}

int32_t CommandMetaData::getPort()
{
    return this->port;
}

void CommandMetaData::appendToListPath(QString path)
{
    this->listPath.append(path);
}

void CommandMetaData::removeFromListPath()
{
    if(this->listPath.size() > 1)
    {
        this->listPath.removeLast();
    }
}

QString CommandMetaData::getLastFromListPath()
{
    return this->listPath.last();
}

QString CommandMetaData::getListPath()
{
    return this->listPath.join("/");
}

bool CommandMetaData::listingRootFolder()
{
    if(this->listPath.size() > 1)
    {
        return false;
    }
    else
    {
        return true;
    }
}

void CommandMetaData::addActiontoSequence(uint32_t action)
{
    this->sequence |= action;
}

void CommandMetaData::removeActionFromSequence(uint32_t action)
{
    this->sequence &= ~action;
}

void CommandMetaData::emptySequence()
{
    this->sequence = 0;
}

bool CommandMetaData::isActionSet(uint32_t action)
{
    if(this->sequence & action)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool CommandMetaData::isSequenceEmpty()
{
    if(this->sequence == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void CommandMetaData::setSequenceId(SEQUENCE_ID seqId)
{
    this->sequenceId = seqId;
}

bool CommandMetaData::openDestFile(QString path)
{
    this->ioDevice = new QFile(path);

    this->error = this->ioDevice->open(QIODevice::WriteOnly);

    if(!this->error)
    {
        this->errorString = this->ioDevice->errorString();
    }

    return this->error;
}

bool CommandMetaData::openSourceFile(QString  path)
{
    this->ioDevice = new QFile(path);

    this->error = this->ioDevice->open(QIODevice::ReadOnly);

    if(!this->error)
    {
        this->errorString = this->ioDevice->errorString();
    }

    return this->error;
}

QFile *CommandMetaData::getIoDevice()
{
    return this->ioDevice;
}

void CommandMetaData::closeIoDevice()
{
    this->ioDevice->close();
}

void CommandMetaData::setFileName(QString fname)
{
    this->fileName = fname;
}

QString CommandMetaData::getFileName()
{
    return this->fileName;
}

void CommandMetaData::setNewFileName(QString fname)
{
    this->newFileName = fname;
}

QString CommandMetaData::getNewFileName()
{
    return this->newFileName;
}
