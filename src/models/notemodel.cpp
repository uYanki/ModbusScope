#include "notemodel.h"

#include "util.h"

NoteModel::NoteModel(QObject *parent) : QAbstractTableModel(parent)
{
    _noteList.clear();

    connect(this, SIGNAL(valueDataChanged(quint32)), this, SLOT(modelDataChanged(quint32)));
    connect(this, SIGNAL(keyDataChanged(quint32)), this, SLOT(modelDataChanged(quint32)));
    connect(this, SIGNAL(textChanged(quint32)), this, SLOT(modelDataChanged(quint32)));

    connect(this, SIGNAL(added(quint32)), this, SLOT(modelDataChanged()));
    connect(this, SIGNAL(removed(quint32)), this, SLOT(modelDataChanged()));
}

QVariant NoteModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal)
        {
            switch (section)
            {
            case 0:
                return QString("Key");
            case 1:
                return QString("Value");
            case 2:
                return QString("Text");
            default:
                return QVariant();
            }
        }
        else
        {
            //Can't happen because it is hidden
        }
    }

    return QVariant();
}

int NoteModel::rowCount(const QModelIndex & /*parent*/) const
{
    return size();
}

int NoteModel::columnCount(const QModelIndex & /*parent*/) const
{
    /*
    * keyData
    * valueData
    * text
    * */
    return 3; // Number of visible members of struct
}

QVariant NoteModel::data(const QModelIndex &index, int role) const
{
    switch (index.column())
    {
    case 0:
        if ((role == Qt::DisplayRole))
        {
            return Util::formatTime(_noteList[index.row()].keyData(), false);
        }
        break;
    case 1:
        if ((role == Qt::DisplayRole))
        {
            return Util::formatDoubleForExport(_noteList[index.row()].valueData());
        }
        break;
    case 2:
        if ((role == Qt::DisplayRole) || (role == Qt::EditRole))
        {
            return _noteList[index.row()].text();
        }
        break;

    default:
        return QVariant();
        break;

    }

    return QVariant();
}

bool NoteModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    bool bRet = true;

    switch (index.column())
    {

    case 2:
        if (role == Qt::EditRole)
        {
            setText(index.row(), value.toString());
        }
        break;
    default:
        bRet = false;
        break;
    }

    // Notify view(s) of change
    emit dataChanged(index, index);

    return bRet;
}

bool NoteModel::removeRows (int row, int count, const QModelIndex & parent)
{
    Q_UNUSED(parent);
    Q_UNUSED(count);

    remove(row);

    return true;
}

Qt::ItemFlags NoteModel::flags(const QModelIndex &index) const
{
    if (index.column() == 2)
    {
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
    }

    return Qt::ItemIsEnabled;
}

qint32 NoteModel::size() const
{
    return _noteList.size();
}

void NoteModel::add(Note &note)
{
    /* Call function to prepare view */
    beginInsertRows(QModelIndex(), size(), size());

    _noteList.append(note);

    /* Call function to trigger view update */
    endInsertRows();

    emit added(size() - 1);
}

void NoteModel::remove(qint32 idx)
{
    beginRemoveRows(QModelIndex(), idx, idx);

    _noteList.removeAt(idx);

    endRemoveRows();

    emit removed(idx);
}

double NoteModel::valueData(quint32 idx)
{
    return _noteList[idx].valueData();
}

double NoteModel::keyData(quint32 idx)
{
    return _noteList[idx].keyData();
}

QString NoteModel::textData(quint32 idx)
{
    return _noteList[idx].text();
}

void NoteModel::setValueData(quint32 idx, double value)
{
    if (_noteList[idx].valueData() != value)
    {
         _noteList[idx].setValueData(value);
         emit valueDataChanged(idx);
    }
}

void NoteModel::setKeyData(quint32 idx, double key)
{
    if (_noteList[idx].keyData() != key)
    {
         _noteList[idx].setKeyData(key);
         emit keyDataChanged(idx);
    }
}

void NoteModel::setText(quint32 idx, QString text)
{
    if (_noteList[idx].text() != text)
    {
         _noteList[idx].setText(text);
         emit textChanged(idx);
    }
}

void NoteModel::modelDataChanged(quint32 idx)
{
    // Notify view(s) of changes
    emit dataChanged(index(idx, 0), index(idx, columnCount() - 1));
}

void NoteModel::modelDataChanged()
{
    emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
}
