#include "expressionsdialog.h"
#include "ui_expressionsdialog.h"

#include "graphdatamodel.h"

ExpressionsDialog::ExpressionsDialog(GraphDataModel *pGraphDataModel, qint32 idx, QWidget *parent) :
    QDialog(parent),
    _pUi(new Ui::ExpressionsDialog),
    _pGraphDataModel(pGraphDataModel),
    _bUpdating(false)
{
    _pUi->setupUi(this);

    _graphIdx = idx;

    connect(&_graphDataHandler, &GraphDataHandler::graphDataReady, this, &ExpressionsDialog::handleDataReady);

    _pUi->tblExpressionInput->setRowCount(0);
    _pUi->tblExpressionInput->setColumnCount(2);
    _pUi->tblExpressionInput->setHorizontalHeaderLabels(QStringList() << "Register" << "Value");
    _pUi->tblExpressionInput->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    _pUi->tblExpressionInput->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);

    connect(_pUi->tblExpressionInput, &QTableWidget::cellChanged, this, &ExpressionsDialog::handleInputChange);

    _pUi->widgetInfo->setVisible(false);
    connect(_pUi->btnInfo, &QAbstractButton::toggled, _pUi->widgetInfo, &QWidget::setVisible);
    connect(_pUi->btnCancel, &QPushButton::clicked, this, &ExpressionsDialog::handleCancel);
    connect(_pUi->btnAccept, &QPushButton::clicked, this, &ExpressionsDialog::handleAccept);

    _pUi->lineExpression->setText(_pGraphDataModel->expression(_graphIdx));
    connect(_pUi->lineExpression, &QLineEdit::textChanged, this, &ExpressionsDialog::handleExpressionChange);

    handleExpressionChange();
}

ExpressionsDialog::~ExpressionsDialog()
{
    delete _pUi;
}

void ExpressionsDialog::handleExpressionChange()
{
    _localGraphDataModel.clear();
    _localGraphDataModel.add();
    _localGraphDataModel.setExpression(0, _pUi->lineExpression->text());

    _graphDataHandler.processActiveRegisters(&_localGraphDataModel);

    QList<ModbusRegister> registerList;
    _graphDataHandler.modbusRegisterList(registerList);

    _bUpdating = true;
    _pUi->tblExpressionInput->setRowCount(registerList.size());

    for(qint32 idx = 0; idx < registerList.size(); idx++)
    {
        ModbusRegister reg = registerList[idx];

        QTableWidgetItem *newRegisterDescr = new QTableWidgetItem(reg.description());
        newRegisterDescr->setFlags(newRegisterDescr->flags() & ~Qt::ItemIsEditable);
        _pUi->tblExpressionInput->setItem(idx, 0, newRegisterDescr);

        QTableWidgetItem *newRegisterValue = new QTableWidgetItem("0");
        _pUi->tblExpressionInput->setItem(idx, 1, newRegisterValue);
    }

    _bUpdating = false;

    handleInputChange();
}

void ExpressionsDialog::handleInputChange()
{
    if (!_bUpdating)
    {
        const auto lightRed = QColor(255, 0, 0, 127);
        const auto white = QColorConstants::White;

        QList<Result> results;
        for(qint32 idx = 0; idx < _pUi->tblExpressionInput->rowCount(); idx++)
        {
            QTableWidgetItem* pValueItem = _pUi->tblExpressionInput->item(idx, 1);
            QString valueStr = pValueItem->text();
            bool bOk = false;
            int value = valueStr.toInt(&bOk);
            results.append(Result(value, bOk));

            /* Avoid recursive signal/slots calling */
            _pUi->tblExpressionInput->blockSignals(true);

            pValueItem->setBackground(bOk ? white: lightRed);
            pValueItem->setToolTip(bOk ? QString(): QStringLiteral("Not a valid number"));

            _pUi->tblExpressionInput->blockSignals(false);
        }

        _graphDataHandler.handleRegisterData(results);
    }
}

void ExpressionsDialog::handleCancel()
{
    done(QDialog::Rejected);
}

void ExpressionsDialog::handleAccept()
{
    _pGraphDataModel->setExpression(_graphIdx, _pUi->lineExpression->text());

    done(QDialog::Accepted);
}

void ExpressionsDialog::handleDataReady(QList<bool> successList, QList<double> values)
{
    QString numOutput;
    QString strTooltip;

    if (successList.first())
    {
        numOutput = QString("%0").arg(values.first());
        strTooltip = QString();
    }
    else
    {
        numOutput = QStringLiteral("-");
        strTooltip = _graphDataHandler.expressionParseMsg(0);
    }

    _pUi->lblOut->setText(numOutput);
    _pUi->lblOut->setToolTip(strTooltip);
}
