#include "queueModel.h"
#include "../database/database.hpp"
#include <iostream>
#include <string>

using std::cout;
using std::endl;
using std::to_string;

QueueModel::QueueModel(QObject *parent) : SqlModel(parent)
{
    //The queue needs to have a separate sorting index.
    //When the queue is reordered, the original indices must remain, because
    // the associated delegates move around otherwise when the data is updated.
    //Thus, we simply track the sorted index and on fresh initialization copy it to the
    //main index.

    //Each thread needs a unique database connection
    QSqlDatabase db = getDB();
    QSqlQuery query(db);
    query.exec("UPDATE QueueTable SET QTindex = QTsortedIndex;");
    maxIndex = 0;
    tableName = "QueueTable";
    resetIndex();
}

QSqlQuery QueueModel::modelQuery()
{
    QString queryString = "SELECT QueueTable.QTindex AS QTindex ";
    queryString.append("         ,QueueTable.QTsortedIndex AS QTsortedIndex ");
    queryString.append("         ,QueueTable.QTprocessed AS QTprocessed ");
    queryString.append("         ,QueueTable.QTexported AS QTexported ");
    queryString.append("         ,QueueTable.QToutput AS QToutput ");
    queryString.append("         ,QueueTable.QTsearchID AS QTsearchID ");
    queryString.append("         ,SearchTable.STrating AS STrating ");
    queryString.append("FROM QueueTable ");
    queryString.append("INNER JOIN SearchTable "
                       "WHERE SearchTable.STsearchID=QueueTable.QTsearchID ");
    queryString.append("ORDER BY ");
    queryString.append("QueueTable.QTindex ASC;");

    //Each thread needs a unique database connection
    QSqlDatabase db = getDB();
    QSqlQuery tempQuery(queryString, db);
    return tempQuery;
}

void QueueModel::setQueueQuery()
{
    setQuery(modelQuery());
    resetIndex();
}

void QueueModel::resetIndex()
{
    //Each thread needs a unique database connection
    QSqlDatabase db = getDB();
    QSqlQuery query(db);
    query.exec("SELECT COUNT(QTindex) FROM QueueTable;");
    const bool success = query.next();
    if (success)
    {
        maxIndex = query.value(0).toInt();
    } else {
        maxIndex = 0;
    }
}

void QueueModel::deQueue(const QString searchID)
{
    //Each thread needs a unique database connection
    QSqlDatabase db = getDB();
    QSqlQuery query(db);
    query.exec("BEGIN TRANSACTION;");
    // We need to update all the indices.
    // We grab the index of the removed item, then decrement all the greater ones.

    //First we grab the index of the row to be removed.
    query.prepare("SELECT QTindex FROM QueueTable WHERE QTsearchID = ?;");
    query.bindValue(0, searchID);
    query.exec();
    query.next();
    const int indexRemoved = query.value(0).toInt();

    //We also need to get the sorted index.
    query.prepare("SELECT QTsortedIndex FROM QueueTable WHERE QTsearchID = ?;");
    query.bindValue(0, searchID);
    query.exec();
    query.next();
    const int sortedIndexRemoved = query.value(0).toInt();

    //Now we remove the item in question.
    query.prepare("DELETE FROM QueueTable WHERE QTsearchID = ?;");
    query.bindValue(0, searchID);
    query.exec();

    //Update what the largest index is.
    resetIndex();

    //Everything after the index in question needs to be decremented.
    query.prepare("UPDATE QueueTable SET QTindex = QTindex - 1 WHERE QTindex > ?;");
    query.bindValue(0, indexRemoved);
    query.exec();

    //We need to also do the same for the sorted index.
    query.prepare("UPDATE QueueTable SET QTsortedIndex = QTsortedIndex - 1 WHERE QTsortedIndex > ?;");
    query.bindValue(0, sortedIndexRemoved);
    query.exec();

    query.exec("COMMIT TRANSACTION");

    //Tell the model which row was removed
    beginRemoveRows(QModelIndex(),indexRemoved,indexRemoved);
    queryModel.setQuery(modelQuery());
    while (queryModel.canFetchMore())
    {
        queryModel.fetchMore();
    }
    endRemoveRows();
    //Now tell the view to update the indices of all of the rows above that.
    //We update everything just for thoroughness's sake.
    emit dataChanged(createIndex(0,0),createIndex(rowCount(),columnCount()));

}

void QueueModel::enQueue(const QString searchID)
{
    //Each thread needs a unique database connection
    QSqlDatabase db = getDB();
    //First check to see if it's already in the queue.
    QSqlQuery query(db);
    query.prepare("SELECT COUNT(*) FROM QueueTable WHERE QTsearchID = ?;");
    query.bindValue(0, searchID);
    query.exec();
    const bool success = query.next();
    bool alreadyInQueue = false;
    if (success)
    {
        alreadyInQueue = query.value(0).toInt() == 1;
    }

    if (alreadyInQueue)
    {
        //do nothing
    }
    else
    {

        query.exec("BEGIN TRANSACTION;");
        query.prepare("SELECT STimportTime,STlastProcessedTime FROM SearchTable WHERE STsearchID=?;");
        query.bindValue(0, searchID);
        query.exec();
        query.next();
        const int importTime = query.value(0).toInt();
        const int lastProcessedTime = query.value(1).toInt();

        //When edited and import times were the same, this sometimes led to false positives.
        //I subtract 1 from the lastProcessedTime to give it some buffer for (floating point?) error.
        const bool edited = importTime < (lastProcessedTime-1);
        query.prepare("INSERT OR IGNORE INTO QueueTable "
                      "(QTindex, QTprocessed, QTexported, QToutput, QTsearchID, QTsortedIndex) "
                      "VALUES (?,?,?,?,?,?);");
        query.bindValue(0, maxIndex);
        query.bindValue(1, edited);
        query.bindValue(2, false);
        query.bindValue(3, false);
        query.bindValue(4, searchID);
        query.bindValue(5, maxIndex);
        query.exec();
        query.exec("END TRANSACTION;");

        //Tell the model which row was added, then fetch the new data.
        beginInsertRows(QModelIndex(),maxIndex,maxIndex);
        queryModel.setQuery(modelQuery());
        while (queryModel.canFetchMore())
        {
            queryModel.fetchMore();
        }
        endInsertRows();

        //Increment the index.
        resetIndex();
    }
}

void QueueModel::batchEnqueue(const QString searchQuery)
{
    cout << "batch enqueue!" << endl;
    cout << searchQuery.toStdString() << endl;
    //Each thread needs a unique database connection
    QSqlDatabase db = getDB();
    //The query must return only STsearchID
    QSqlQuery query(searchQuery, db);

    //get the IDs to enqueue
    QStringList searchIDList;
    while(query.next())
    {
        searchIDList << query.value(0).toString();
    }

    //write to queue
    int newItemCount = 0;
    int oldMaxIndex = maxIndex;
    query.exec("BEGIN TRANSACTION;");
    for(int i = 0; i < searchIDList.size(); i++)
    {
        QString searchID = searchIDList.at(i);

        //First check to see if it's already in the queue.
        query.prepare("SELECT COUNT(*) FROM QueueTable WHERE QTsearchID = ?;");
        query.bindValue(0, searchID);
        query.exec();
        const bool success = query.next();
        bool alreadyInQueue = false;
        if (success)
        {
            alreadyInQueue = query.value(0).toInt() == 1;
        }

        if (alreadyInQueue)
        {
            //do nothing
        }
        else
        {
            newItemCount++;
            query.prepare("SELECT STimportTime,STlastProcessedTime FROM SearchTable WHERE STsearchID=?;");
            query.bindValue(0, searchID);
            query.exec();
            query.next();
            const int importTime = query.value(0).toInt();
            const int lastProcessedTime = query.value(1).toInt();

            //When edited and import times were the same, this sometimes led to false positives.
            //I subtract 1 from the lastProcessedTime to give it some buffer for (floating point?) error.
            const bool edited = importTime < (lastProcessedTime-1);
            query.prepare("INSERT OR IGNORE INTO QueueTable "
                          "(QTindex, QTprocessed, QTexported, QToutput, QTsearchID, QTsortedIndex) "
                          "VALUES (?,?,?,?,?,?);");
            query.bindValue(0, maxIndex);
            query.bindValue(1, edited);
            query.bindValue(2, false);
            query.bindValue(3, false);
            query.bindValue(4, searchID);
            query.bindValue(5, maxIndex);
            query.exec();

            //Increment the index.
            resetIndex();
        }
    }
    query.exec("END TRANSACTION;");

    //notify the model that a bunch of rows were added
    if (newItemCount > 0)
    {
        beginInsertRows(QModelIndex(),oldMaxIndex,maxIndex-1);
        queryModel.setQuery(modelQuery());
        while (queryModel.canFetchMore())
        {
            queryModel.fetchMore();
        }
        endInsertRows();
    }
}

void QueueModel::clearQueue()
{
    //Each thread needs a unique database connection
    QSqlDatabase db = getDB();
    QSqlQuery query(db);
    beginRemoveRows(QModelIndex(),0,maxIndex-1);
    query.exec("DELETE FROM QueueTable");
    resetIndex();
    endRemoveRows();
}

//Move moves the item with searchID to destIndex,
// shifting the others along the way.
//It determines the source index from the database.
void QueueModel::move(const QString searchID, const int destIndex)
{
    //Each thread needs a unique database connection
    QSqlDatabase db = getDB();
    QSqlQuery query(db);

    query.exec("BEGIN TRANSACTION;");


    //Get the source index.
    query.prepare("SELECT QTsortedIndex FROM QueueTable  WHERE QTsearchID = ?;");
    query.bindValue(0, searchID);
    query.exec();
    query.next();
    const int sourceIndex = query.value(0).toInt();

    if (sourceIndex < destIndex) //Move one to the right, shift the rest leftward
    {
        std::string queryString = "UPDATE QueueTable SET QTsortedIndex = QTsortedIndex - 1 ";
        queryString.append("WHERE QTsortedIndex >= ");
        queryString.append(to_string(sourceIndex));
        queryString.append(" AND QTsortedIndex <= ");
        queryString.append(to_string(destIndex));
        queryString.append(";");
        query.exec(QString::fromStdString(queryString));
        query.prepare("UPDATE QueueTable "
                      "SET QTsortedIndex = ? "
                      "WHERE QTsearchID = ?;");
        query.bindValue(0, destIndex);
        query.bindValue(1, searchID);
        query.exec();
    }
    else if (sourceIndex > destIndex) //move one to the left, shift the rest rightward
    {
        std::string queryString = "UPDATE QueueTable SET QTsortedIndex = QTsortedIndex + 1 ";
        queryString.append("WHERE QTsortedIndex >= ");
        queryString.append(to_string(destIndex));
        queryString.append(" AND QTsortedIndex <= ");
        queryString.append(to_string(sourceIndex));
        queryString.append(";");
        query.exec(QString::fromStdString(queryString));
        query.prepare("UPDATE QueueTable "
                      "SET QTsortedIndex = ? "
                      "WHERE QTsearchID = ?;");
        query.bindValue(0, destIndex);
        query.bindValue(1, searchID);
        query.exec();
    }
    else
    {
        //Do nothing
    }
    query.exec("END TRANSACTION");
    //Now tell the view to update all of the indices.
    //We update everything just for thoroughness's sake.
    refreshAll();
}

void QueueModel::markSaved(const QString searchID)
{
    //Each thread needs a unique database connection
    QSqlDatabase db = getDB();
    QSqlQuery query(db);
    query.prepare("UPDATE QueueTable "
                  "SET QTexported = 1 "
                  "WHERE QTsearchID = ?;");
    query.bindValue(0, searchID);
    query.exec();
    updateAll();
}
