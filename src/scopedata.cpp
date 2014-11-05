

#include "modbusmaster.h"
#include "modbussettings.h"
#include "scopegui.h"
#include "QTimer"
#include "QDebug"

#include "scopedata.h"

ScopeData::ScopeData(QObject *parent) :
    QObject(parent), _master(NULL), _active(false), _timer(new QTimer()), _successCount(0), _errorCount(0)
{

    qRegisterMetaType<QList<quint16> *>("QList<quint16> *");
    qRegisterMetaType<QList<quint16> >("QList<quint16>");
    qRegisterMetaType<QList<bool> >("QList<bool>");

    _registerlist.clear();

    /* Setup modbus master */
    _master = new ModbusMaster();

    connect(this, SIGNAL(requestStop()), _master, SLOT(stopThread()));

    connect(_master, SIGNAL(threadStopped()), this, SLOT(masterStopped()));
    connect(_master, SIGNAL(threadStopped()), _master, SLOT(deleteLater()));

    _master->startThread();

    connect(this, SIGNAL(registerRequest(ModbusSettings *, QList<quint16> *)), _master, SLOT(readRegisterList(ModbusSettings *, QList<quint16> *)));

    connect(_master, SIGNAL(readRegisterResult(QList<bool>, QList<quint16>)), this, SIGNAL(handleReceivedData(QList<bool>, QList<quint16>)));
    connect(_master, SIGNAL(modbusCommDone(quint32, quint32)), this, SLOT(processCommStats(quint32, quint32)));
}

ScopeData::~ScopeData()
{
    emit requestStop();

    if (_master)
    {
        _master->wait();
    }

    delete _timer;
}

bool ScopeData::startCommunication(ModbusSettings * pSettings)
{
    bool bResetted = false;

    if (!_active)
    {
        _settings.copy(pSettings);

        _successCount = 0;
        _errorCount = 0;

        // Trigger read immediatly
        _timer->singleShot(1, this, SLOT(readData()));

        _active = true;
        bResetted = true;
    }

    return bResetted;
}

quint32 ScopeData::getRegisterCount()
{
    return _registerlist.size();
}

void ScopeData::getRegisterList(QList<quint16> * pList)
{
    pList->clear();
    pList->append(_registerlist);
}

void ScopeData::clearRegisterList()
{
    _registerlist.clear();
}

void ScopeData::toggleRegister(quint16 registerAddress)
{
    bool bFound = false;
    const quint16 regAddr = registerAddress;

    for(qint32 i = 0; i < _registerlist.size(); i++)
    {
        if (_registerlist.at(i) == regAddr)
        {
            _registerlist.removeAt(i);
            bFound = true;
            break;
        }
    }

    if (!bFound)
    {
        _registerlist.append(regAddr);
    }
}


void ScopeData::removedRegister(quint16 registerAddress)
{
    const quint16 regAddr = registerAddress;

    for(qint32 i = 0; i < _registerlist.size(); i++)
    {
        if (_registerlist.at(i) == regAddr)
        {
            _registerlist.removeAt(i);
            break;
        }
    }
}

void ScopeData::processCommStats(quint32 success,quint32 error)
{
    _successCount += success;
    _errorCount += error;

    emit triggerStatUpdate(_successCount, _errorCount);
}


void ScopeData::masterStopped()
{
    _master = NULL;
}

void ScopeData::stopCommunication()
{
    _active = false;
}

void ScopeData::readData()
{
    if(_active)
    {
        emit registerRequest(&_settings, &_registerlist);

        _timer->singleShot(_settings.getPollTime(), this, SLOT(readData()));
    }
}

