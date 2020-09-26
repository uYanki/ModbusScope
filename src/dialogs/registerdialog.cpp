
#include <QColorDialog>
#include "util.h"
#include "registerdialog.h"
#include "importmbcdialog.h"
#include "registerconndelegate.h"
#include "expressioncomposedelegate.h"

#include "ui_registerdialog.h"

RegisterDialog::RegisterDialog(GuiModel *pGuiModel, GraphDataModel * pGraphDataModel, SettingsModel * pSettingsModel, QWidget *parent) :
    QDialog(parent),
    _pUi(new Ui::RegisterDialog)
{
    _pUi->setupUi(this);

    /* Disable question mark button */
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    _pGraphDataModel = pGraphDataModel;
    _pGuiModel = pGuiModel;

    // Setup registerView
    _pUi->registerView->setModel(_pGraphDataModel);
    _pUi->registerView->verticalHeader()->hide();

    RegisterConnDelegate* cbConn = new RegisterConnDelegate(pSettingsModel,_pUi->registerView);
    _pUi->registerView->setItemDelegateForColumn(GraphDataModel::column::CONNECTION_ID, cbConn);
    _pUi->registerView->setItemDelegateForColumn(GraphDataModel::column::EXPRESSION, new ExpressionComposeDelegate(_pGraphDataModel, _pUi->registerView));

    /* Don't stretch columns */
    _pUi->registerView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    /* Except following columns */
    _pUi->registerView->horizontalHeader()->setSectionResizeMode(GraphDataModel::column::TEXT, QHeaderView::Stretch);

    // Select using click, shift and control
    _pUi->registerView->setSelectionBehavior(QAbstractItemView::SelectRows);
    _pUi->registerView->setSelectionMode(QAbstractItemView::ExtendedSelection);

    // Handle color cell active signal
    connect(_pUi->registerView, &QTableView::activated, this, &RegisterDialog::activatedCell);

    // Handle delete
    QShortcut* shortcut = new QShortcut(QKeySequence(QKeySequence::Delete), _pUi->registerView);
    connect(shortcut, &QShortcut::activated, this, &RegisterDialog::removeRegisterRow);

    // Setup handler for buttons
    connect(_pUi->btnImportFromMbc, &QPushButton::released, this, &RegisterDialog::showImportDialogDefault);
    connect(_pUi->btnAdd, &QPushButton::released, this, &RegisterDialog::addRegisterRow);
    connect(_pUi->btnRemove, &QPushButton::released, this, &RegisterDialog::removeRegisterRow);
    connect(_pGraphDataModel, &GraphDataModel::rowsInserted, this, &RegisterDialog::onRegisterInserted);
}

RegisterDialog::~RegisterDialog()
{
    delete _pUi;
}

void RegisterDialog::done(int r)
{
    bool bValid = true;

    if(QDialog::Accepted == r)  // ok was pressed
    {
        quint16 duplicateReg = 0;
        QString duplicateExpression;
        quint8 duplicateConnectionId = 0;
        if (!_pGraphDataModel->getDuplicate(&duplicateReg, &duplicateExpression, &duplicateConnectionId))
        {
            bValid = false;

            Util::showError(tr("Register %1 with expression \"%2\" of connection %3 is defined twice in the list.").arg(duplicateReg).arg(duplicateExpression).arg(duplicateConnectionId + 1));
        }
    }
    else
    {
        // cancel, close or exc was pressed
        bValid = true;
    }

    if (bValid)
    {
        QDialog::done(r);
    }
}

int RegisterDialog::exec()
{
    return QDialog::exec();
}

int RegisterDialog::exec(QString mbcFile)
{
    showImportDialog(mbcFile);

    return exec();
}

void RegisterDialog::addRegisterRow()
{
    _pGraphDataModel->insertRow(_pGraphDataModel->size());
}

void RegisterDialog::showImportDialogDefault()
{
    showImportDialog(QString(""));
}

void RegisterDialog::showImportDialog(QString mbcPath)
{
    MbcRegisterModel mbcRegisterModel(_pGraphDataModel);
    ImportMbcDialog importMbcDialog(_pGuiModel, _pGraphDataModel, &mbcRegisterModel, this);

    if (importMbcDialog.exec(mbcPath) == QDialog::Accepted)
    {
        QList<GraphData> regList = mbcRegisterModel.selectedRegisterList();

        if (regList.size() > 0)
        {
            _pGraphDataModel->add(regList);
        }
    }
}

void RegisterDialog::activatedCell(QModelIndex modelIndex)
{
    if (modelIndex.column() == GraphDataModel::column::COLOR)
    {
        if (modelIndex.row() < _pGraphDataModel->size())
        {
            // Let user pick color
            QColor color = QColorDialog::getColor(_pGraphDataModel->color(modelIndex.row()));

            if (color.isValid())
            {
                // Set color in model
                _pGraphDataModel->setData(modelIndex, color, Qt::EditRole);
            }
            else
            {
                // user aborted
            }
        }
    }
}

void RegisterDialog::onRegisterInserted(const QModelIndex &parent, int first, int last)
{
    Q_UNUSED(parent);
    Q_UNUSED(last);

    /* select the first new row, this will also make the row visible */
    _pUi->registerView->selectRow(first);
}

void RegisterDialog::removeRegisterRow()
{
    // get list of selected rows
    QItemSelectionModel *selected = _pUi->registerView->selectionModel();
    QModelIndexList rowList = selected->selectedRows();

    // sort QModelIndexList
    // We need to remove the highest rows first
    std::sort(rowList.begin(), rowList.end(), &RegisterDialog::sortRegistersLastFirst);

    foreach(QModelIndex rowIndex, rowList)
    {
        _pGraphDataModel->removeRow(rowIndex.row());
    }
}

bool RegisterDialog::sortRegistersLastFirst(const QModelIndex &s1, const QModelIndex &s2)
{
    return s1.row() > s2.row();
}
