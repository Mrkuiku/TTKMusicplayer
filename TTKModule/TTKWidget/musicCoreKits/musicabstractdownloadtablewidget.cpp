#include "musicabstractdownloadtablewidget.h"
#include "musicsongssummariziedwidget.h"
#include "musicdownloadmanager.h"
#include "musicconnectionpool.h"
#include "musicitemdelegate.h"
#include "musicmessagebox.h"
#include "musicstringutils.h"

MusicAbstractDownloadTableWidget::MusicAbstractDownloadTableWidget(QWidget *parent)
    : MusicAbstractSongsListTableWidget(parent)
{
    m_delegate = new MusicProgressBarDelegate(this);

    G_CONNECTION_PTR->setValue(getClassName(), this);
    G_CONNECTION_PTR->poolConnect(getClassName(), MusicSongsSummariziedWidget::getClassName());

    connect(this, SIGNAL(cellDoubleClicked(int,int)), SLOT(itemCellDoubleClicked(int,int)));
}

MusicAbstractDownloadTableWidget::~MusicAbstractDownloadTableWidget()
{
    G_CONNECTION_PTR->removeValue(getClassName());
    MusicDownloadRecordConfigManager xml(m_type, this);
    xml.writeDownloadData(*m_musicSongs);
    clear();

    delete m_delegate;
}

void MusicAbstractDownloadTableWidget::updateSongsFileName(const MusicSongs &songs)
{
    Q_UNUSED(songs);
    MusicDownloadRecordConfigManager xml(m_type, this);
    if(!xml.readConfig())
    {
        return;
    }
    xml.readDownloadData(*m_musicSongs);

    setRowCount(m_musicSongs->count()); //reset row count

    for(int i=0; i<m_musicSongs->count(); ++i)
    {
        MusicSong *song = &((*m_musicSongs)[i]);
        createItem(i, *song);
        G_DOWNLOAD_MANAGER_PTR->reconnectMusicDownload(MusicDownLoadPairData(song->getMusicAddTimeStr().toULongLong(), this, m_type));
    }
}

void MusicAbstractDownloadTableWidget::musicPlay()
{
    if(rowCount() == 0 || currentRow() < 0)
    {
        return;
    }

    const QString &path = (*m_musicSongs)[currentRow()].getMusicPath();
    Q_EMIT addSongToPlaylist(QStringList(QFile::exists(path) ? path : QString()));
}

void MusicAbstractDownloadTableWidget::setDeleteItemAt()
{
    MusicMessageBox message;
    message.setText(tr("Are you sure to delete?"));
    if(!message.exec() || rowCount() == 0)
    {
       return;
    }

    const TTKIntList deleteList(getMultiSelectedIndex());
    if(deleteList.isEmpty())
    {
        return;
    }

    for(int i=deleteList.count() - 1; i>=0; --i)
    {
        const int index = deleteList[i];
        removeRow(index);
        m_musicSongs->removeAt(index);
    }
    //just fix table widget size hint
    setFixedHeight(totalHeight());
    Q_EMIT updateItemTitle(m_parentToolIndex);
}

void MusicAbstractDownloadTableWidget::itemCellClicked(int row, int column)
{
    Q_UNUSED(row);
    Q_UNUSED(column);
}

void MusicAbstractDownloadTableWidget::itemCellDoubleClicked(int row, int column)
{
    Q_UNUSED(row);
    Q_UNUSED(column);

    musicPlay();
}

void MusicAbstractDownloadTableWidget::downloadProgressChanged(float percent, const QString &total, qint64 time)
{
    for(int i=0; i<rowCount(); ++i)
    {
        QTableWidgetItem *it = item(i, 3);
        if(it && it->data(MUSIC_TIME_ROLE).toLongLong() == time)
        {
            item(i, 2)->setData(MUSIC_PROGRESS_ROLE, percent);
            item(i, 3)->setText(total);

            (*m_musicSongs)[i].setMusicSizeStr(total);
            break;
        }
    }
}

void MusicAbstractDownloadTableWidget::createDownloadItem(const QString &name, qint64 time)
{
    setRowCount(rowCount() + 1);
    QString musicName = name;
    musicName.remove(MusicUtils::String::musicPrefix()).chop(4);

    MusicSong record;
    record.setMusicName(musicName);
    record.setMusicPath(QFileInfo(name).absoluteFilePath());
    record.setMusicSizeStr("0.00M");
    record.setMusicAddTimeStr(QString::number(time));
    m_musicSongs->append(record);

    createItem(rowCount() - 1, record);
    Q_EMIT updateItemTitle(m_parentToolIndex);
}

void MusicAbstractDownloadTableWidget::contextMenuEvent(QContextMenuEvent *event)
{
    Q_UNUSED(event);
    QMenu rightClickMenu(this);

    rightClickMenu.setStyleSheet(MusicUIObject::MQSSMenuStyle02);
    rightClickMenu.addAction(QIcon(":/contextMenu/btn_play"), tr("Play"), this, SLOT(musicPlayClicked()));
    rightClickMenu.addAction(tr("Download More..."), this, SLOT(musicSongDownload()));
    rightClickMenu.addSeparator();

    createMoreMenu(&rightClickMenu);

    const bool empty = !m_musicSongs->isEmpty();
    rightClickMenu.addAction(tr("Song Info..."), this, SLOT(musicFileInformation()))->setEnabled(empty);
    rightClickMenu.addAction(QIcon(":/contextMenu/btn_localFile"), tr("Open File Dir"), this, SLOT(musicOpenFileDir()))->setEnabled(empty);
    rightClickMenu.addAction(QIcon(":/contextMenu/btn_ablum"), tr("Ablum"), this, SLOT(musicAlbumQueryWidget()));
    rightClickMenu.addSeparator();

    rightClickMenu.addAction(QIcon(":/contextMenu/btn_delete"), tr("Delete"), this, SLOT(setDeleteItemAt()))->setEnabled(empty);
    rightClickMenu.addAction(tr("Delete All"), this, SLOT(setDeleteItemAll()))->setEnabled(empty);

    rightClickMenu.exec(QCursor::pos());
}
