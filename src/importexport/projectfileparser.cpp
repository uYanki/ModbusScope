
#include <QtWidgets>
#include "util.h"
#include "projectfileparser.h"
#include "projectfiledefinitions.h"

ProjectFileParser::ProjectFileParser()
{
    _msgBox.setWindowTitle(tr("ModbusScope project file load error"));
    _msgBox.setIcon(QMessageBox::Warning);
}


bool ProjectFileParser::parseFile(QIODevice *device, ProjectSettings *pSettings)
{
    bool bRet = true;
    QString errorStr;
    qint32 errorLine;
    qint32 errorColumn;

    if (!_domDocument.setContent(device, true, &errorStr, &errorLine, &errorColumn))
    {
        _msgBox.setText(tr("Parse error at line %1, column %2:\n%3")
                .arg(errorLine)
                .arg(errorColumn)
                .arg(errorStr));
        _msgBox.exec();

        bRet = false;
    }
    else
    {
        QDomElement root = _domDocument.documentElement();
        if (root.tagName() != ProjectFileDefinitions::cModbusScopeTag)
        {
            _msgBox.setText(tr("The file is not a valid ModbusScope project file."));
            _msgBox.exec();
            bRet = false;
        }
        else
        {
            // Check data level attribute
            QString strDataLevel = root.attribute(ProjectFileDefinitions::cDatalevelAttribute, "1");
            quint32 datalevel = strDataLevel.toUInt(&bRet);

            if (bRet)
            {
                if (datalevel != ProjectFileDefinitions::cCurrentDataLevel)
                {
                    _msgBox.setText(tr("Data level (%1) is not supported. Only datalevel 2 is allowed. Project file loading is aborted.").arg(datalevel));
                    _msgBox.exec();
                    bRet = false;
                }
            }
            else
            {
                _msgBox.setText(tr("Data level (%1) is not a valid number").arg(strDataLevel));
                _msgBox.exec();
            }

            if (bRet)
            {
                QDomElement tag = root.firstChildElement();
                while (!tag.isNull())
                {
                    if (tag.tagName() == ProjectFileDefinitions::cModbusTag)
                    {
                        bRet = parseModbusTag(tag, &pSettings->general);
                        if (!bRet)
                        {
                            break;
                        }
                    }
                    else if (tag.tagName() == ProjectFileDefinitions::cScopeTag)
                    {
                        bRet = parseScopeTag(tag, &pSettings->scope);
                        if (!bRet)
                        {
                            break;
                        }
                    }
                    else if (tag.tagName() == ProjectFileDefinitions::cViewTag)
                    {
                        bRet = parseViewTag(tag, &pSettings->view);
                        if (!bRet)
                        {
                            break;
                        }
                    }

                    tag = tag.nextSiblingElement();
                }
            }
        }
    }

    return bRet;
}


bool ProjectFileParser::parseModbusTag(const QDomElement &element, GeneralSettings * pGeneralSettings)
{
    bool bRet = true;
    QDomElement child = element.firstChildElement();
    while (!child.isNull())
    {
        if (child.tagName() == ProjectFileDefinitions::cConnectionTag)
        {
            bRet = parseConnectionTag(child, &pGeneralSettings->connectionSettings);
            if (!bRet)
            {
                break;
            }
        }
        else if (child.tagName() == ProjectFileDefinitions::cLogTag)
        {
            bRet = parseLogTag(child, &pGeneralSettings->logSettings);
            if (!bRet)
            {
                break;
            }
        }
        else
        {
            // unkown tag: ignore
        }
        child = child.nextSiblingElement();
    }

    return bRet;
}

bool ProjectFileParser::parseConnectionTag(const QDomElement &element, ConnectionSettings * pConnectionSettings)
{
    bool bRet = true;
    QDomElement child = element.firstChildElement();
    while (!child.isNull())
    {
        if (child.tagName() == ProjectFileDefinitions::cIpTag)
        {
            pConnectionSettings->bIp = true;
            pConnectionSettings->ip = child.text();
        }
        else if (child.tagName() == ProjectFileDefinitions::cPortTag)
        {
            pConnectionSettings->bPort = true;
            pConnectionSettings->port = child.text().toUInt(&bRet);
            if (!bRet)
            {
                _msgBox.setText(tr("Port ( %1 ) is not a valid number").arg(child.text()));
                _msgBox.exec();
                break;
            }
        }
        else if (child.tagName() == ProjectFileDefinitions::cSlaveIdTag)
        {
            pConnectionSettings->bSlaveId = true;
            pConnectionSettings->slaveId = child.text().toUInt(&bRet);
            if (!bRet)
            {
                _msgBox.setText(tr("Slave id ( %1 ) is not a valid number").arg(child.text()));
                _msgBox.exec();

                break;
            }
        }
        else if (child.tagName() == ProjectFileDefinitions::cTimeoutTag)
        {
            pConnectionSettings->bTimeout = true;
            pConnectionSettings->timeout = child.text().toUInt(&bRet);
            if (!bRet)
            {
                _msgBox.setText(tr("Timeout ( %1 ) is not a valid number").arg(child.text()));
                _msgBox.exec();

                break;
            }
        }
        else if (child.tagName() == ProjectFileDefinitions::cConsecutiveMaxTag)
        {
            pConnectionSettings->bConsecutiveMax = true;
            pConnectionSettings->consecutiveMax = child.text().toUInt(&bRet);
            if (!bRet)
            {
                _msgBox.setText(tr("Consecutive register maximum ( %1 ) is not a valid number").arg(child.text()));
                _msgBox.exec();

                break;
            }
        }
        else
        {
            // unkown tag: ignore
        }
        child = child.nextSiblingElement();
    }

    return bRet;
}

bool ProjectFileParser::parseLogTag(const QDomElement &element, LogSettings * pLogSettings)
{
    bool bRet = true;
    QDomElement child = element.firstChildElement();
    while (!child.isNull())
    {
        if (child.tagName() == ProjectFileDefinitions::cPollTimeTag)
        {
            pLogSettings->bPollTime = true;
            pLogSettings->pollTime = child.text().toUInt(&bRet);
            if (!bRet)
            {
                _msgBox.setText(tr("Poll time ( %1 ) is not a valid number").arg(child.text()));
                _msgBox.exec();
                break;
            }
        } 
        else if (child.tagName() == ProjectFileDefinitions::cAbsoluteTimesTag)
        {
            if (!child.text().toLower().compare(ProjectFileDefinitions::cTrueValue))
            {
                pLogSettings->bAbsoluteTimes = true;
            }
            else
            {
                pLogSettings->bAbsoluteTimes = false;
            }
        }
        else if (child.tagName() == ProjectFileDefinitions::cLogToFileTag)
        {
            bRet = parseLogToFile(child, pLogSettings);
            if (!bRet)
            {
                break;
            }
        }
        else
        {
            // unkown tag: ignore
        }
        child = child.nextSiblingElement();
    }

    return bRet;
}

bool ProjectFileParser::parseLogToFile(const QDomElement &element, LogSettings *pLogSettings)
{
    bool bRet = true;

    // Check attribute
    QString enabled = element.attribute(ProjectFileDefinitions::cEnabledAttribute, ProjectFileDefinitions::cTrueValue);

    if (!enabled.toLower().compare(ProjectFileDefinitions::cTrueValue))
    {
        pLogSettings->bLogToFile = true;
    }
    else
    {
        pLogSettings->bLogToFile = false;
    }

    // Check nodes
    QDomElement child = element.firstChildElement();
    while (!child.isNull())
    {
        if (child.tagName() == ProjectFileDefinitions::cFilenameTag)
        {
            QFileInfo fileInfo = QFileInfo(child.text());

            bool bValid = true;

            /* check file path points to existing file */
            if (!fileInfo.isFile())
            {
                /* Check if file path points to something else that already exists */
                if (fileInfo.exists())
                {
                    /* path exist, but it is not a file */
                    bValid = false;
                    _msgBox.setText(tr("Log file path (%1) already exists, but it is not a file. Log file is set to default.").arg(fileInfo.filePath()));
                    _msgBox.exec();
                }
                else
                {
                    /* file path does not exist yet */

                    /* Does parent directory exist? */
                    if (!fileInfo.dir().exists())
                    {
                        bValid = false;
                        _msgBox.setText(tr("Log file path (parent directory) does not exists (%1). Log file is set to default.").arg(fileInfo.filePath()));
                        _msgBox.exec();
                    }
                }
            }

            if (bValid)
            {
                pLogSettings->bLogToFileFile = true;
                pLogSettings->logFile = fileInfo.filePath();
            }
            else
            {
                pLogSettings->bLogToFileFile = false;
            }
        }
        else
        {
            // unkown tag: ignore
        }
        child = child.nextSiblingElement();
    }

    return bRet;
}

bool ProjectFileParser::parseScopeTag(const QDomElement &element, ScopeSettings *pScopeSettings)
{
    bool bRet = true;
    QDomElement child = element.firstChildElement();
    while (!child.isNull())
    {
        if (child.tagName() == ProjectFileDefinitions::cRegisterTag)
        {
            RegisterSettings registerData;
            bRet = parseRegisterTag(child, &registerData);
            if (!bRet)
            {
                break;
            }

            // check for duplicate registers
            bool bFound = false;

            for (int i = 0; i < pScopeSettings->registerList.size(); i++)
            {
                if (
                        (pScopeSettings->registerList[i].address == registerData.address)
                        && (pScopeSettings->registerList[i].bitmask == registerData.bitmask)
                    )
                {
                    bFound = true;
                    break;
                }
            }

            if (bFound)
            {
                _msgBox.setText(tr("Register %1 with bitmask 0x%2 is defined twice in the list.").arg(registerData.address).arg(registerData.bitmask, 0, 16));
                _msgBox.exec();
                bRet = false;
                break;
            }
            else
            {
                pScopeSettings->registerList.append(registerData);
            }
        }
        else
        {
            // unkown tag: ignore
        }
        child = child.nextSiblingElement();
    }

    return bRet;
}


bool ProjectFileParser::parseRegisterTag(const QDomElement &element, RegisterSettings *pRegisterSettings)
{
    bool bRet = true;

    // Check attribute
    QString active = element.attribute(ProjectFileDefinitions::cActiveAttribute, ProjectFileDefinitions::cTrueValue);

    if (!active.toLower().compare(ProjectFileDefinitions::cTrueValue))
    {
        pRegisterSettings->bActive = true;
    }
    else
    {
        pRegisterSettings->bActive = false;
    }

    // Check nodes
    QDomElement child = element.firstChildElement();
    while (!child.isNull())
    {
        if (child.tagName() == ProjectFileDefinitions::cAddressTag)
        {
            pRegisterSettings->address = child.text().toUInt(&bRet);
            if (!bRet)
            {
                _msgBox.setText(tr("Address ( %1 ) is not a valid number").arg(child.text()));
                _msgBox.exec();
                break;
            }
        }
        else if (child.tagName() == ProjectFileDefinitions::cTextTag)
        {
            pRegisterSettings->text = child.text();
        }
        else if (child.tagName() == ProjectFileDefinitions::cUnsignedTag)
        {
            if (!child.text().toLower().compare(ProjectFileDefinitions::cTrueValue))
            {
                pRegisterSettings->bUnsigned = true;
            }
            else
            {
                pRegisterSettings->bUnsigned = false;
            }
        }
        else if (child.tagName() == ProjectFileDefinitions::cDivideTag)
        {
            // use settings from system locale
            pRegisterSettings->divideFactor = QLocale::system().toDouble(child.text(), &bRet);

            if (!bRet)
            {
                _msgBox.setText(QString("Divide factor (%1) is not a valid double. Expected decimal separator is \"%2\".").arg(child.text()).arg(QLocale::system().decimalPoint()));
                _msgBox.exec();
                break;
            }
        }
        else if (child.tagName() == ProjectFileDefinitions::cMultiplyTag)
        {
            // use settings from system locale
            pRegisterSettings->multiplyFactor = QLocale::system().toDouble(child.text(), &bRet);

            if (!bRet)
            {
                _msgBox.setText(QString("Multiply factor (%1) is not a valid double. Expected decimal separator is \"%2\".").arg(child.text()).arg(QLocale::system().decimalPoint()));
                _msgBox.exec();
                break;
            }
        }
        else if (child.tagName() == ProjectFileDefinitions::cColorTag)
        {
            bRet = QColor::isValidColor(child.text());
            pRegisterSettings->bColor = bRet;
            if (bRet)
            {
                pRegisterSettings->color = QColor(child.text());
            }
            else
            {
                _msgBox.setText(tr("Color is not a valid color. Did you use correct color format? Expecting #FF0000 (red)"));
                _msgBox.exec();
                break;
            }
        }
        else if (child.tagName() == ProjectFileDefinitions::cBitmaskTag)
        {
            const quint16 newBitMask = child.text().toUInt(&bRet, 0);

            if (bRet)
            {
                pRegisterSettings->bitmask = newBitMask;
            }
            else
            {
                _msgBox.setText(tr("Bitmask (\"%1\") is not a valid integer.").arg(child.text()));
                _msgBox.exec();
                break;
            }
        }
        else if (child.tagName() == ProjectFileDefinitions::cShiftTag)
        {
            const qint32 newShift = child.text().toInt(&bRet);

            if (
                    (bRet)
                    &&
                    (
                        (newShift < -15)
                        ||
                        (newShift > 15)
                    )
                )
            {
                bRet = false;
            }

            if (bRet)
            {
                pRegisterSettings->shift = newShift;
            }
            else
            {
                _msgBox.setText(tr("Shift factor (%1) is not a valid integer between -16 and 16.").arg(child.text()));
                _msgBox.exec();
                break;
            }
        }
        else
        {
            // unkown tag: ignore
        }

        child = child.nextSiblingElement();
    }

    return bRet;
}

bool ProjectFileParser::parseViewTag(const QDomElement &element, ViewSettings *pViewSettings)
{
    bool bRet = true;
    QDomElement child = element.firstChildElement();
    while (!child.isNull())
    {
        if (child.tagName() == ProjectFileDefinitions::cScaleTag)
        {
            bRet = parseScaleTag(child, &pViewSettings->scaleSettings);
            if (!bRet)
            {
                break;
            }
        }
        else
        {
            // unkown tag: ignore
        }
        child = child.nextSiblingElement();
    }

    return bRet;
}

bool ProjectFileParser::parseScaleTag(const QDomElement &element, ScaleSettings *pScaleSettings)
{
    bool bRet = true;
    QDomElement child = element.firstChildElement();
    while (!child.isNull())
    {
        if (child.tagName() == ProjectFileDefinitions::cXaxisTag)
        {
            // Check attribute
            QString active = child.attribute(ProjectFileDefinitions::cModeAttribute);

            if (!active.toLower().compare(ProjectFileDefinitions::cSlidingValue))
            {
                // Sliding interval mode
                pScaleSettings->bSliding = true;

                bRet = parseScaleXAxis(child, pScaleSettings);
                if (!bRet)
                {
                    break;
                }
            }
            else if (!active.toLower().compare(ProjectFileDefinitions::cAutoValue))
            {
                // auto interval mode
                pScaleSettings->bSliding = false;
            }
        }
        else if (child.tagName() == ProjectFileDefinitions::cYaxisTag)
        {
            // Check attribute
            QString active = child.attribute(ProjectFileDefinitions::cModeAttribute);

            if (!active.toLower().compare(ProjectFileDefinitions::cWindowAutoValue))
            {
                pScaleSettings->bWindowScale = true;
            }
            else if (!active.toLower().compare(ProjectFileDefinitions::cMinmaxValue))
            {
                // min max mode
                pScaleSettings->bMinMax = true;

                bRet = parseScaleYAxis(child, pScaleSettings);
                if (!bRet)
                {
                    break;
                }
            }
            else if (!active.toLower().compare(ProjectFileDefinitions::cAutoValue))
            {
                // auto interval mode
            }
        }
        else
        {
            // unkown tag: ignore
        }
        child = child.nextSiblingElement();
    }

    return bRet;
}

bool ProjectFileParser::parseScaleXAxis(const QDomElement &element, ScaleSettings *pScaleSettings)
{
    bool bRet = true;
    bool bSlidingInterval = false;

    // Check nodes
    QDomElement child = element.firstChildElement();
    while (!child.isNull())
    {
        if (child.tagName() == ProjectFileDefinitions::cSlidingintervalTag)
        {
            pScaleSettings->slidingInterval = child.text().toUInt(&bRet);
            if (bRet)
            {
                bSlidingInterval = true;
            }
            else
            {
                _msgBox.setText(tr("Scale (x-axis) has an incorrect sliding interval. \"%1\" is not a valid number").arg(child.text()));
                _msgBox.exec();
                break;
            }
        }
        else
        {
            // unkown tag: ignore
        }
        child = child.nextSiblingElement();
    }

    if (!bSlidingInterval)
    {
        _msgBox.setText(tr("If x-axis has sliding window scaling then slidinginterval variable should be defined."));
        _msgBox.exec();
        bRet = false;
    }

    return bRet;
}

bool ProjectFileParser::parseScaleYAxis(const QDomElement &element, ScaleSettings *pScaleSettings)
{
    bool bRet = true;
    bool bMin = false;
    bool bMax = false;

    // Check nodes
    QDomElement child = element.firstChildElement();
    while (!child.isNull())
    {
        if (child.tagName() == ProjectFileDefinitions::cMinTag)
        {
            pScaleSettings->scaleMin = child.text().toInt(&bRet);
            if (bRet)
            {
                bMin = true;
            }
            else
            {
                _msgBox.setText(tr("Scale (y-axis) has an incorrect minimum. \"%1\" is not a valid number").arg(child.text()));
                _msgBox.exec();
                break;
            }
        }
        else if (child.tagName() == ProjectFileDefinitions::cMaxTag)
        {
            pScaleSettings->scaleMax = child.text().toInt(&bRet);
            if (bRet)
            {
                bMax = true;
            }
            else
            {
                _msgBox.setText(tr("Scale (y-axis) has an incorrect maximum. \"%1\" is not a valid number").arg(child.text()));
                _msgBox.exec();
                break;
            }
        }
        else
        {
            // unkown tag: ignore
        }
        child = child.nextSiblingElement();
    }

    if (!bMin)
    {
        _msgBox.setText(tr("If y-axis has min max scaling then min variable should be defined."));
        _msgBox.exec();
        bRet = false;
    }
    else if (!bMax)
    {
        _msgBox.setText(tr("If y-axis has min max scaling then max variable should be defined."));
        _msgBox.exec();
        bRet = false;
    }

    return bRet;
}
