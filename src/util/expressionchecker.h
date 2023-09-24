#ifndef EXPRESSION_CHECKER_H
#define EXPRESSION_CHECKER_H

#include <QObject>
#include "graphdatamodel.h"
#include "graphdatahandler.h"

class ExpressionChecker : public QObject
{
    Q_OBJECT
public:

    explicit ExpressionChecker(QObject *parent = nullptr);

    bool parseExpression(QString expr);
    void descriptions(QStringList& descr);

    void setValues(ResultDoubleList results);

    bool isValid();
    double result();
    QString strError();
    qint32 errorPos();

signals:
    void resultsReady();

private slots:
    void handleDataReady(ResultDoubleList resultList);

private:

    GraphDataModel _localGraphDataModel;
    GraphDataHandler _graphDataHandler;

    bool _bValid;
    double _result;
    QString _strError;
    qint32 _errorPos;

};

#endif // EXPRESSION_CHECKER_H
