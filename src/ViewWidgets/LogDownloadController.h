/*=====================================================================

 QGroundControl Open Source Ground Control Station

 (c) 2009 - 2014 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>

 This file is part of the QGROUNDCONTROL project

 QGROUNDCONTROL is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 QGROUNDCONTROL is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with QGROUNDCONTROL. If not, see <http://www.gnu.org/licenses/>.

 ======================================================================*/

#ifndef LogDownloadController_H
#define LogDownloadController_H

#include <QObject>
#include <QTimer>
#include <QAbstractListModel>

#include <memory>

#include "UASInterface.h"
#include "AutoPilotPlugin.h"
#include "FactPanelController.h"

class  MultiVehicleManager;
class  UASInterface;
class  Vehicle;
class  QGCLogEntry;
class  LogDownloadData;

Q_DECLARE_LOGGING_CATEGORY(LogDownloadLog)

//-----------------------------------------------------------------------------
class QGCLogModel : public QAbstractListModel
{
    Q_OBJECT
public:

    enum QGCLogModelRoles {
        ObjectRole = Qt::UserRole + 1
    };

    QGCLogModel(QObject *parent = 0);

    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_INVOKABLE QGCLogEntry* get(int index);

    int         count           (void) const;
    void        append          (QGCLogEntry* entry);
    void        clear           (void);
    QGCLogEntry*operator[]      (int i);

    int         rowCount        (const QModelIndex & parent = QModelIndex()) const;
    QVariant    data            (const QModelIndex & index, int role = Qt::DisplayRole) const;

signals:
    void        countChanged    ();

protected:
    QHash<int, QByteArray> roleNames() const;
private:
    QList<QGCLogEntry*> _logEntries;
};

//-----------------------------------------------------------------------------
class QGCLogEntry : public QObject {
    Q_OBJECT
    Q_PROPERTY(uint         id          READ id                             CONSTANT)
    Q_PROPERTY(QDateTime    time        READ time                           NOTIFY timeChanged)
    Q_PROPERTY(uint         size        READ size                           NOTIFY sizeChanged)
    Q_PROPERTY(bool         received    READ received                       NOTIFY receivedChanged)
    Q_PROPERTY(bool         selected    READ selected   WRITE setSelected   NOTIFY selectedChanged)
    Q_PROPERTY(QString      status      READ status                         NOTIFY statusChanged)

public:
    QGCLogEntry(uint logId, const QDateTime& dateTime = QDateTime(), uint logSize = 0, bool received = false);

    uint        id          () const { return _logID; }
    uint        size        () const { return _logSize; }
    QDateTime   time        () const { return _logTimeUTC; }
    bool        received    () const { return _received; }
    bool        selected    () const { return _selected; }
    QString     status      () const { return _status; }

    void        setId       (uint id_)          { _logID = id_; }
    void        setSize     (uint size_)        { _logSize = size_;     emit sizeChanged(); }
    void        setTime     (QDateTime date_)   { _logTimeUTC = date_;  emit timeChanged(); }
    void        setReceived (bool rec_)         { _received = rec_;     emit receivedChanged(); }
    void        setSelected (bool sel_)         { _selected = sel_;     emit selectedChanged(); }
    void        setStatus   (QString stat_)     { _status = stat_;      emit statusChanged(); }

signals:
    void        idChanged       ();
    void        timeChanged     ();
    void        sizeChanged     ();
    void        receivedChanged ();
    void        selectedChanged ();
    void        statusChanged   ();

private:
    uint        _logID;
    uint        _logSize;
    QDateTime   _logTimeUTC;
    bool        _received;
    bool        _selected;
    QString     _status;
};

//-----------------------------------------------------------------------------
class LogDownloadData {
public:
    LogDownloadData(QGCLogEntry* entry);
    QList<uint>     offsets;
    QFile           file;
    QString         filename;
    uint            ID;
    QTimer          processDataTimer;
    QGCLogEntry*    entry;
    uint            written;
};

//-----------------------------------------------------------------------------
class LogDownloadController : public FactPanelController
{
    Q_OBJECT
public:

    LogDownloadController();

    Q_PROPERTY(QGCLogModel* model           READ model              NOTIFY modelChanged)
    Q_PROPERTY(bool         requestingList  READ requestingList     NOTIFY requestingListChanged)
    Q_PROPERTY(bool         downloadingLogs READ downloadingLogs    NOTIFY downloadingLogsChanged)

    QGCLogModel*    model                   () { return &_logEntriesModel; }
    bool            requestingList          () { return _requestingLogEntries; }
    bool            downloadingLogs         () { return _downloadingLogs; }

    Q_INVOKABLE void refresh                ();
    Q_INVOKABLE void download               ();
    Q_INVOKABLE void eraseAll               ();
    Q_INVOKABLE void cancel                 ();

signals:
    void requestingListChanged  ();
    void downloadingLogsChanged ();
    void modelChanged           ();
    void selectionChanged       ();

private slots:
    void _setActiveVehicle  (Vehicle* vehicle);
    void _logEntry          (UASInterface *uas, uint32_t time_utc, uint32_t size, uint16_t id, uint16_t num_logs, uint16_t last_log_num);
    void _logData           (UASInterface *uas, uint32_t ofs, uint16_t id, uint8_t count, const uint8_t *data);
    void _processDownload   ();

private:

    bool _entriesComplete   ();
    bool _logComplete       ();
    void _findMissingEntries();
    void _receivedAllEntries();
    void _receivedAllData   ();
    void _resetSelection    ();
    void _findMissingData   ();
    void _requestLogList    (uint32_t start = 0, uint32_t end = 0xFFFF);
    void _requestLogData    (uint8_t id, uint32_t offset = 0, uint32_t count = 0xFFFFFFFF);
    bool _prepareLogDownload();

    QGCLogEntry* _getNextSelected();

    UASInterface*       _uas;
    LogDownloadData*    _downloadData;
    QTimer              _timer;
    QGCLogModel         _logEntriesModel;
    Vehicle*            _vehicle;
    bool                _requestingLogEntries;
    bool                _downloadingLogs;
    int                 _retries;
    QString             _downloadPath;
};

#endif
