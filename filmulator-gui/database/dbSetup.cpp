#include "filmulatorDB.h"
#include <QString>
#include <QVariant>
#include <iostream>
#include <QStandardPaths>
#include <QFile>

DBSuccess setupDB(QSqlDatabase *db)
{
    QDir dir = QDir::home();
    QString dirstr = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    dirstr.append("/filmulator");
    if (!dir.cd(dirstr))
    {
        if (!dir.mkpath(dirstr))
        {
            std::cout << "Could not create database directory" << std::endl;
            return DBSuccess::failure;
        }
        else
        {
            dir.cd(dirstr);
        }
    }
    db -> setDatabaseName(dir.absoluteFilePath("filmulatorDB"));
    //this should create the database if it doesn't already exist.

    if (db -> open())
    {
//        std::cout << "Database open!" << std::endl;
        //Success
    }
    else
    {
        std::cout << "what?!?!?!?" << std::endl;
        return DBSuccess::failure;
    }

    QSqlQuery query;

    //Check the database version.
    query.exec("PRAGMA user_version;");
    query.next();
    const int oldVersion = query.value(0).toInt();
    if (oldVersion > 12)//=================================================================version check here!
    {
        std::cout << "Newer database format. Aborting." << std::endl;
        return DBSuccess::failure;
    }
    else if (oldVersion < 12)//============================================================version check here!
    {
        std::cout << "Backing up old database" << std::endl;
        QFile file(dir.absoluteFilePath("filmulatorDB"));
        QString name = "filmulatorDB_schema_";
        name.append(QString::number(oldVersion));
        file.copy(dir.absoluteFilePath(name));
    }

    //We need to set up 3 tables for the processing.
    //1. The master table for searching. This should be small
    //  for speed. It points at the other two.
    query.exec("create table if not exists SearchTable ("
               "STsearchID varchar primary key,"
               "STcaptureTime integer,"//unix time
               "STname varchar,"//name of this instance
               "STfilename varchar,"//name of source file
               "STsourceHash varchar(32),"//primary key in the file table
               "STrating integer,"//for easy culling
               "STlatitude real,"
               "STlongitude real,"
               "STimportTime integer,"//unix time
               "STlastProcessedTime integer,"//unix time
               "STimportStartTime integer,"//unix time
               "STthumbWritten integer,"//-1 for error, 0 for not written or invalidated, 1 for written
               "STbigThumbWritten integer"//same as above
               ");"
               );
    query.exec("create index if not exists TimeIndex"
               " on SearchTable (STcaptureTime);"
               );

    //2. The table which carries more file info, like exifs and thumbnail locations.
    query.exec("create table if not exists FileTable ("
                "FTfileID varchar(32) primary key," //md5sum of the file
                "FTfilePath varchar,"
                "FTcameraMake varchar,"
                "FTcameraModel varchar,"
                "FTsensitivity integer,"
                "FTexposureTime varchar,"
                "FTaperture real,"
                "FTfocalLength real,"
                "FTusageIncrement integer"
                ");"
               );

    //3. The table which holds processing parameters.
    //     This table should have the same number of entries as the SearchTable.
    query.exec("create table if not exists ProcessingTable ("
               "ProcTprocID varchar primary key,"           //0
               "ProcTinitialDeveloperConcentration real,"   //1
               "ProcTreservoirThickness real,"              //2
               "ProcTactiveLayerThickness real,"            //3
               "ProcTcrystalsPerPixel real,"                //4
               "ProcTinitialCrystalRadius real,"            //5
               "ProcTinitialSilverSaltDensity real,"        //6
               "ProcTdeveloperConsumptionConst real,"       //7
               "ProcTcrystalGrowthConst real,"              //8
               "ProcTsilverSaltConsumptionConst real,"      //9
               "ProcTtotalDevelopmentTime real,"            //10
               "ProcTagitateCount integer,"                 //11
               "ProcTdevelopmentSteps integer,"             //12
               "ProcTfilmArea real,"                        //13
               "ProcTsigmaConst real,"                      //14
               "ProcTlayerMixConst real,"                   //15
               "ProcTlayerTimeDivisor real,"                //16
               "ProcTrolloffBoundary integer,"              //17
               "ProcTexposureComp real,"                    //18
               "ProcTwhitepoint real,"                      //19
               "ProcTblackpoint real,"                      //20
               "ProcTshadowsX real,"                        //21
               "ProcTshadowsY real,"                        //22
               "ProcThighlightsX real,"                     //23
               "ProcThighlightsY real,"                     //24
               "ProcThighlightRecovery integer,"            //25
               "ProcTcaEnabled integer,"                    //26
               "ProcTtemperature real,"                     //27
               "ProcTtint real,"                            //28
               "ProcTvibrance real,"                        //29
               "ProcTsaturation real,"                      //30
               "ProcTrotation integer,"                     //31
               "ProcTcropHeight real,"                      //32
               "ProcTcropAspect real,"                      //33
               "ProcTcropVoffset real,"                     //34
               "ProcTcropHoffset real,"                     //35
               "ProcTmonochrome integer,"                   //36
               "ProcTbwRmult real,"                         //37
               "ProcTbwGmult real,"                         //38
               "ProcTbwBmult real,"                         //39
               "ProcTtoeBoundary real"                      //40
               "ProcTlensfunName varchar,"                  //41
               "ProcTlensfunCa integer,"                    //42
               "ProcTlensfunVign integer,"                  //43
               "ProcTlensfunDist integer"                   //44
               ");"
               );

    //Next, we set up a table for default processing parameters.
    //This will be of the same structure as ProcessingTable except for orientation and crop.
    query.exec("create table if not exists ProfileTable ("
               "ProfTprofileId varchar primary key,"        //0
               "ProfTinitialDeveloperConcentration real,"   //1
               "ProfTreservoirThickness real,"              //2
               "ProfTactiveLayerThickness real,"            //3
               "ProfTcrystalsPerPixel real,"                //4
               "ProfTinitialCrystalRadius real,"            //5
               "ProfTinitialSilverSaltDensity real,"        //6
               "ProfTdeveloperConsumptionConst real,"       //7
               "ProfTcrystalGrowthConst real,"              //8
               "ProfTsilverSaltConsumptionConst real,"      //9
               "ProfTtotalDevelopmentTime real,"            //10
               "ProfTagitateCount integer,"                 //11
               "ProfTdevelopmentSteps integer,"             //12
               "ProfTfilmArea real,"                        //13
               "ProfTsigmaConst real,"                      //14
               "ProfTlayerMixConst real,"                   //15
               "ProfTlayerTimeDivisor real,"                //16
               "ProfTrolloffBoundary integer,"              //17
               "ProfTexposureComp real,"                    //18
               "ProfTwhitepoint real,"                      //19
               "ProfTblackpoint real,"                      //20
               "ProfTshadowsX real,"                        //21
               "ProfTshadowsY real,"                        //22
               "ProfThighlightsX real,"                     //23
               "ProfThighlightsY real,"                     //24
               "ProfThighlightRecovery integer,"            //25
               "ProfTcaEnabled integer,"                    //26
               "ProfTtemperature real,"                     //27
               "ProfTtint real,"                            //28
               "ProfTvibrance real,"                        //29
               "ProfTsaturation real,"                      //30 rotation and 4x crop params
               "ProfTmonochrome integer,"                   //31 (+5 to match ProcessingTable)
               "ProfTbwRmult real,"                         //32
               "ProfTbwGmult real,"                         //33
               "ProfTbwBmult real,"                         //34
               "ProfTtoeBoundary real,"                     //35
               "ProfTlensfunName varchar,"                 //36
               "ProfTlensfunCa integer,"                    //37
               "ProfTlensfunVign integer,"                  //38
               "ProfTlensfunDist integer"                   //39
               ");"
               );

    //Next, we make a table for the queue.
    //It will hold a number for the order, and a field identical to
    // STsearchID.
    query.exec("create table if not exists QueueTable ("
               "QTindex integer,"
               "QTprocessed bool,"
               "QTexported bool,"
               "QToutput bool,"
               "QTsearchID varchar unique"
               ");"
               );

    //Next, we make a table that stores preferred lens corrections.
    //These will override the ProcT values if absent
    query.exec("CREATE TABLE IF NOT EXISTS LensPrefs ("
               "ExifCamera varchar, "
               "ExifLens varchar, "
               "LensfunLens varchar, "
               "LensfunCa integer, "
               "LensfunVign integer, "
               "LensfunDist integer, "
               "AutoCa integer, "
               "PRIMARY KEY (ExifCamera, ExifLens)"
               ");"
               );

    //Now we set the default Default profile.
    query.prepare("REPLACE INTO ProfileTable values "
                  "(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);");
                  //                    1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3 3 3 3 3 3 3 3 3
                  //0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9
    //Name of profile; must be unique.
    query.bindValue(0, "Default");
    //Initial Developer Concentration
    query.bindValue(1, 1.0f); //initialDeveloperConcentration
    //Thickness of developer reservoir
    query.bindValue(2, 1000.0f); //reservoirThickness
    //Thickness of active layer (not necessarily the same units as above. I think. Maybe. -CV)
    query.bindValue(3, 0.1f); //activeLayerThickness
    //Number of crystals per pixel that might become activated
    query.bindValue(4, 500.0f); //crystalsPerPixel
    //Initial radius for just-activated silver crystals
    query.bindValue(5, 0.00001f); //initialCrystalRadius
    //Initial surface density of silver salt crystals
    query.bindValue(6, 1.0f); //initalSilverSaltDensity
    //Rate constant for consumption of developer
    query.bindValue(7, 2000000.0f); //developerConsumptionConst
    //Proportionality constant for growth of silver crystals relative to consumption of developer
    query.bindValue(8, 0.00001f); //crystalGrowthConst
    //Rate constant for consumption of silver halides in the film layers
    query.bindValue(9, 2000000.0f); //silverSaltConsumptionConst
    //Total duration of the simulated development
    query.bindValue(10, 100.0f); //totalDevelopmentTime
    //Number of times the developer solution is mixed during the development process
    query.bindValue(11, 1); //agitateCount
    //Number of simulation steps for the development process
    query.bindValue(12, 12); //developmentSteps
    //Area of the simulated film
    query.bindValue(13, 864.0f); //filmArea
    //Constant that influences the extent of the diffusion; yes this is redundant.
    query.bindValue(14, 0.2f); //sigmaConst
    //Constant that affects the amount of layer-reservoir diffusion
    query.bindValue(15, 0.2f); //layerMixConst
    //Constant that affects the ratio of intra-layer and layer-reservoir diffusion; yes this is redundant.
    query.bindValue(16, 20.0f); //layerTimeDivisor
    //Constant that affects where the rolloff starts in exposure
    query.bindValue(17, 51275); //rolloffBoundary
    //Exposure compensation
    query.bindValue(18, 0.0f); //exposureComp
    //White clipping point
    query.bindValue(19, 0.002f); //whitepoint
    //Black clipping point
    query.bindValue(20, 0.0f); //blackpoint
    //Input value defining the general shadow region control point
    query.bindValue(21, 0.25f); //shadowsX
    //Output value defining the general shadow region control point
    query.bindValue(22, 0.25f); //shadowsY
    //Input value defining the general highlight region control point
    query.bindValue(23, 0.75f); //highlightsX
    //Output value defining the general highlight region control point
    query.bindValue(24, 0.75f); //highlightsY
    //dcraw highlight recovery parameter
    query.bindValue(25, 0); //highlightRecovery
    //Automatic CA correct switch
    query.bindValue(26, -1); //caEnabled | -1 indicates use lens preferences
    //Color temperature WB adjustment
    query.bindValue(27, 5200.0f); //temperature
    //Magenta/green tint WB adjustment
    query.bindValue(28, 1.0f); //tint
    //Saturation of less-saturated stuff
    query.bindValue(29, 0.0f); //vibrance
    //Saturation of whole image
    query.bindValue(30, 0.0f); //saturation
    //Whether the image is monochrome
    query.bindValue(31, 0); //monochrome
    //weighting for black and white conversion
    query.bindValue(32, 0.21f); //bwRmult
    query.bindValue(33, 0.78f); //bwGmult
    query.bindValue(34, 0.07f); //bwBmult
    //How much to offset the exposure tone curve to the right while maintaining the toe
    query.bindValue(35, 0.0f); //toeBoundary
    query.bindValue(36, "NoLens"); //lensfun lens
    query.bindValue(37, -1); //lensfun CA - negative 1 indicates use preferences
    query.bindValue(38, -1); //lensfun vignetting
    query.bindValue(39, -1); //lensfun distortion

    //Well, orientation and crop obviously don't get presets.
    query.exec();

    //Because older versions would erroneously overwrite the file usage count on setting a new location, we need to set them all to 1
    //SELECT COUNT(*), FTfileID FROM FileTable INNER JOIN SearchTable WHERE STsourceHash=FTfileID GROUP BY FTfileID;

    //Update old versions of the database
    QString versionString = ";";

    query.exec("BEGIN TRANSACTION;");//begin a transaction
    switch (oldVersion) {
    case 0:
        //Generate a list of 100000 integers for useful purposes
        /*
        query.exec("CREATE TABLE integers (i integer);");
        query.exec("INSERT INTO integers (i) VALUES (0);");
        query.exec("INSERT INTO integers (i) VALUES (1);");
        query.exec("INSERT INTO integers (i) VALUES (2);");
        query.exec("INSERT INTO integers (i) VALUES (3);");
        query.exec("INSERT INTO integers (i) VALUES (4);");
        query.exec("INSERT INTO integers (i) VALUES (5);");
        query.exec("INSERT INTO integers (i) VALUES (6);");
        query.exec("INSERT INTO integers (i) VALUES (7);");
        query.exec("INSERT INTO integers (i) VALUES (8);");
        query.exec("INSERT INTO integers (i) VALUES (9);");
        query.exec("CREATE VIEW integers9 as "
                   "SELECT 100000*a.i+10000*b.i+1000*c.i+100*d.i+10*e.i+f.i as ints "
                   "FROM integers a "
                   "CROSS JOIN integers b "
                   "CROSS JOIN integers c "
                   "CROSS JOIN integers d "
                   "CROSS JOIN integers e "
                   "CROSS JOIN integers f;");
                   */
        versionString = "PRAGMA user_version = 1;";
        std::cout << "Upgrading from old db version 0" << std::endl;
        [[fallthrough]];
    case 1:
        /*
        query.exec("CREATE VIEW integers5 as "
                   "SELECT 10000*a.i+1000*b.i+100*c.i+10*d.i+e.i as ints "
                   "FROM integers a "
                   "CROSS JOIN integers b "
                   "CROSS JOIN integers c "
                   "CROSS JOIN integers d "
                   "CROSS JOIN integers e;");
        query.exec("CREATE VIEW integers4 as "
                   "SELECT 1000*a.i+100*b.i+10*c.i+d.i as ints "
                   "FROM integers a "
                   "CROSS JOIN integers b "
                   "CROSS JOIN integers c "
                   "CROSS JOIN integers d;");
                   */
        versionString = "PRAGMA user_version = 2;";
        std::cout << "Upgrading from old db version 1" << std::endl;
        [[fallthrough]];
    case 2:
        query.exec("DROP TABLE QueueTable;");
        query.exec("CREATE TABLE QueueTable ("
                   "QTindex integer,"
                   "QTprocessed bool,"
                   "QTexported bool,"
                   "QToutput bool,"
                   "QTsearchID varchar unique);");
        versionString = "PRAGMA user_version = 3;";
        std::cout << "Upgrading from old db version 2" << std::endl;
        [[fallthrough]];
    case 3:
        query.exec("DROP VIEW integers9;");
        query.exec("DROP VIEW integers4;");
        query.exec("DROP VIEW integers3;");
        versionString = "PRAGMA user_version = 4;";
        std::cout << "Upgrading from old db version 3" << std::endl;
        [[fallthrough]];
    case 4:
        query.exec("ALTER TABLE SearchTable "
                   "ADD COLUMN STimportStartTime integer;");
        query.exec("UPDATE SearchTable SET STimportStartTime = STimportTime;");
        versionString = "PRAGMA user_version = 5;";
        std::cout << "Upgrading from old db version 4" << std::endl;
        [[fallthrough]];
    case 5:
        query.exec("UPDATE SearchTable SET STrating = min(5,max(0,STrating));");
        versionString = "PRAGMA user_version = 6;";
        std::cout << "Upgrading from old db version 5" << std::endl;
        [[fallthrough]];
    case 6:
        query.exec("ALTER TABLE SearchTable "
                   "ADD COLUMN STthumbWritten bool;");
        query.exec("ALTER TABLE SearchTable "
                   "ADD COLUMN STbigThumbWritten bool;");
        query.exec("UPDATE SearchTable SET STthumbWritten = 1;");
        query.exec("UPDATE SearchTable SET STbigThumbWritten = 0;");
        versionString = "PRAGMA user_version = 7;";
        std::cout << "Upgrading from old db version 6" << std::endl;
        [[fallthrough]];
    case 7:
        query.exec("ALTER TABLE QueueTable "
                   "ADD COLUMN QTsortedIndex;");
        query.exec("UPDATE QueueTable SET QTsortedIndex = QTindex;");
        versionString = "PRAGMA user_version = 8;";
        std::cout << "Upgrading from old db version 7" << std::endl;
        [[fallthrough]];
    case 8:
        query.exec("ALTER TABLE ProcessingTable ADD COLUMN ProcTcropHeight;");
        query.exec("ALTER TABLE ProcessingTable ADD COLUMN ProcTcropAspect;");
        query.exec("ALTER TABLE ProcessingTable ADD COLUMN ProcTcropVoffset;");
        query.exec("ALTER TABLE ProcessingTable ADD COLUMN ProcTcropHoffset;");
        query.exec("UPDATE ProcessingTable SET ProcTcropHeight = 0;");
        query.exec("UPDATE ProcessingTable SET ProcTcropAspect = 0;");
        query.exec("UPDATE ProcessingTable SET ProcTcropVoffset = 0;");
        query.exec("UPDATE ProcessingTable SET ProcTcropHoffset = 0;");
        versionString = "PRAGMA user_version = 9;";
        std::cout << "Upgrading from old db version 8" << std::endl;
        [[fallthrough]];
    case 9:
        query.exec("ALTER TABLE ProcessingTable ADD COLUMN ProcTmonochrome;");
        query.exec("ALTER TABLE ProcessingTable ADD COLUMN ProcTbwRmult;");
        query.exec("ALTER TABLE ProcessingTable ADD COLUMN ProcTbwGmult;");
        query.exec("ALTER TABLE ProcessingTable ADD COLUMN ProcTbwBmult;");
        query.exec("UPDATE ProcessingTable SET ProcTmonochrome = 0;");
        query.exec("UPDATE ProcessingTable SET ProcTbwRmult = 0.21");
        query.exec("UPDATE ProcessingTable SET ProcTbwGmult = 0.78");
        query.exec("UPDATE ProcessingTable SET ProcTbwBmult = 0.07");
        query.exec("ALTER TABLE ProfileTable ADD COLUMN ProfTmonochrome;");
        query.exec("ALTER TABLE ProfileTable ADD COLUMN ProfTbwRmult;");
        query.exec("ALTER TABLE ProfileTable ADD COLUMN ProfTbwGmult;");
        query.exec("ALTER TABLE ProfileTable ADD COLUMN ProfTbwBmult;");
        query.exec("UPDATE ProfileTable SET ProfTmonochrome = 0;");
        query.exec("UPDATE ProfileTable SET ProfTbwRmult = 0.21");
        query.exec("UPDATE ProfileTable SET ProfTbwGmult = 0.78");
        query.exec("UPDATE ProfileTable SET ProfTbwBmult = 0.07");
        versionString = "PRAGMA user_version = 10;";
        std::cout << "Upgrading from old db version 9" << std::endl;
        [[fallthrough]];
    case 10:
        query.exec("ALTER TABLE ProcessingTable ADD COLUMN ProcTtoeBoundary;");
        query.exec("UPDATE ProcessingTable SET ProcTtoeBoundary = 0;");
        query.exec("ALTER TABLE ProfileTable ADD COLUMN ProfTtoeBoundary;");
        query.exec("UPDATE ProfileTable SET ProfTtoeBoundary = 0;");
        versionString = "PRAGMA user_version = 11;";
        std::cout << "Upgrading from old db version 10" << std::endl;
        [[fallthrough]];
    case 11:
        query.exec("ALTER TABLE ProcessingTable ADD COLUMN ProcTlensfunName;");
        query.exec("ALTER TABLE ProcessingTable ADD COLUMN ProcTlensfunCa;");
        query.exec("ALTER TABLE ProcessingTable ADD COLUMN ProcTlensfunVign;");
        query.exec("ALTER TABLE ProcessingTable ADD COLUMN ProcTlensfunDist;");
        query.exec("UPDATE ProcessingTable SET ProcTlensfunName = \"NoLens\";");
        query.exec("UPDATE ProcessingTable SET ProcTlensfunCa    = -1");
        query.exec("UPDATE ProcessingTable SET ProcTlensfunVign  = -1");
        query.exec("UPDATE ProcessingTable SET ProcTlensfunDist  = -1");
        query.exec("ALTER TABLE ProfileTable ADD COLUMN ProfTlensfunName;");
        query.exec("ALTER TABLE ProfileTable ADD COLUMN ProfTlensfunCa;");
        query.exec("ALTER TABLE ProfileTable ADD COLUMN ProfTlensfunVign;");
        query.exec("ALTER TABLE ProfileTable ADD COLUMN ProfTlensfunDist;");
        query.exec("UPDATE ProfileTable SET ProfTlensfunName = \"NoLens\";");
        query.exec("UPDATE ProfileTable SET ProfTlensfunCa    = -1");
        query.exec("UPDATE ProfileTable SET ProfTlensfunVign  = -1");
        query.exec("UPDATE ProfileTable SET ProfTlensfunDist  = -1");
        versionString = "PRAGMA user_version = 12;";
        std::cout << "Upgrading from old db version 11" << std::endl;
    }
    query.exec(versionString);
    query.exec("COMMIT TRANSACTION;");//finalize the transaction only after writing the version.

    return DBSuccess::success;
}
