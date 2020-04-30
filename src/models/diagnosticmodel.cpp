#include "diagnosticmodel.h"
#include "QModelIndex"
#include "QAbstractItemModel"

/*!
 * \brief Constructor for DiagnosticModel
 * \param parent    parent object
 */
DiagnosticModel::DiagnosticModel(QObject *parent) : QAbstractListModel(parent)
{
    _maxSeverityLevel = Diagnostic::LOG_INFO;
}

/*!
 * \brief Return numbers of rows in model
 * \return Numbers of rows in model
 */
int DiagnosticModel::rowCount(const QModelIndex & /*parent*/) const
{
    return size();
}

/*!
 * \brief Return numbers of columns in model
 * \return Numbers of columns in model
 */
int DiagnosticModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 1;
}

/*!
 * \brief Get data from model
 * \param index modelindex referring to requested data
 * \param role Requested data role
 * \return Requested data from model, Empty QVariant() on invalid argument
 */
QVariant DiagnosticModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid() && (role == Qt::DisplayRole))
    {
        return _logList[index.row()].toString();
    }

    return QVariant();
}

/*!
 * \brief Get data severity from model
 * \param index row referring to requested data
 * \return Requested data from model, -1 on invalid argument
 */
Diagnostic::LogSeverity DiagnosticModel::dataSeverity(quint32 index) const
{
    if (index < static_cast<quint32>(size()))
    {
        return _logList[static_cast<qint32>(index)].severity();
    }

    return static_cast<Diagnostic::LogSeverity>(-1);
}

/*!
 * \brief Get header data from model
 * \param section Requested column header data
 * \param orientation Unused
 * \param role Requested header role
 * \return Requested header data from model, Emptye QVariant() on invalid argument
 */
QVariant DiagnosticModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(orientation);

    if (
            (section == 0)
            && (role == Qt::DisplayRole)
        )
    {
        return QString("Messages");
    }

    return QVariant();
}

/*!
 * \brief Get flags for specific index
 * \param index modelindex referring to requested data
 * \return Flags of index
 */
Qt::ItemFlags DiagnosticModel::flags(const QModelIndex & index) const
{
    Q_UNUSED(index);
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

/*!
 * \brief Return numbers of rows in model
 * \return Numbers of rows in model
 */
qint32 DiagnosticModel::size() const
{
    return _logList.size();
}

/*!
 * \brief Clear the model data
 */
void DiagnosticModel::clear()
{
    beginRemoveRows(QModelIndex(), 0, size());

    _logList.clear();

    endRemoveRows();
}

/*!
 * \brief Set the maximum severity level of the allowed logs.
 *  All logs with higher severity level aren't saved.
 */
void DiagnosticModel::setMaxSeverityLevel(Diagnostic::LogSeverity maxSeverity)
{
    _maxSeverityLevel = maxSeverity;
}

/*!
 * \brief Add item to model
 * \param log
 */
void DiagnosticModel::addLog(Diagnostic& log)
{
    if (log.severity() <= _maxSeverityLevel)
    {
        /* Call function to prepare view */
        beginInsertRows(QModelIndex(), size(), size());

        _logList.append(log);

        /* Call functions to trigger view update */
        endInsertRows();

        QModelIndex nIndex = index(size() - 1, 0);

        emit dataChanged(nIndex, nIndex);
    }
}

void DiagnosticModel::addCommunicationLog(Diagnostic::LogSeverity severity, QString message)
{
    auto log = Diagnostic(Diagnostic::LOG_COMMUNICATION,
                        severity,
                        QDateTime::currentDateTime(),
                        message
                        );

    addLog(log);
}
