#ifndef IMPORTWORKER_H
#define IMPORTWORKER_H

#include <QObject>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QString>
#include <QtSql/QSqlQuery>
#include <QCryptographicHash>
#include "exifFunctions.h"
#include <QVariant>
#include "sqlInsertion.h"

class ImportWorker : public QObject
{
    Q_OBJECT

public:
    explicit ImportWorker( QObject *parent = 0 );

public slots:
    void importFile( const QFileInfo infoIn,
                     const int importTZ,
                     const int cameraTZ,
                     const QString photoDir,
                     const QString backupDir,
                     const QString dirConfig );

signals:
    void doneProcessing();

};

#endif // IMPORTWORKER_H
