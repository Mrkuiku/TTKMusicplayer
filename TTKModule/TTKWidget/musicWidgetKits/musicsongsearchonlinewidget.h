#ifndef MUSICSONGSEARCHONLINEWIDGET_H
#define MUSICSONGSEARCHONLINEWIDGET_H

/* =================================================
 * This file is part of the TTK Music Player project
 * Copyright (C) 2015 - 2021 Greedysky Studio

 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License along
 * with this program; If not, see <http://www.gnu.org/licenses/>.
 ================================================= */

#include "musicitemsearchtablewidget.h"
#include "musicabstractqueryrequest.h"

/*! @brief The class of the data download item.
 * @author Greedysky <greedysky@163.com>
 */
typedef struct TTK_MODULE_EXPORT MusicDownloadData
{
    QString m_songName;
    QString m_timestamp;
    QString m_format;

    void clear()
    {
        m_songName.clear();
        m_timestamp.clear();
        m_format.clear();
    }

    bool isValid() const
    {
        return !(m_songName.isEmpty() && m_timestamp.isEmpty() && m_format.isEmpty());
    }

}MusicDownloadData;
TTK_DECLARE_LIST(MusicDownloadData)

class QLabel;
class QPushButton;
class MusicCoreMPlayer;

/*! @brief The class of the song search online table widget.
 * @author Greedysky <greedysky@163.com>
 */
class TTK_MODULE_EXPORT MusicSongSearchTableWidget : public MusicItemSearchTableWidget
{
    Q_OBJECT
    TTK_DECLARE_MODULE(MusicSongSearchTableWidget)
public:
    /*!
     * Object contsructor.
     */
    explicit MusicSongSearchTableWidget(QWidget *parent = nullptr);
    ~MusicSongSearchTableWidget();

    /*!
     * Set wheather query all quality of records.
     */
    inline void setQueryAllRecords(bool state) { m_queryAllRecords = state; }
    /*!
     * Start search query by text.
     */
    virtual void startSearchQuery(const QString &text) override;
    /*!
     * Start search query by given id.
     */
    void startSearchSingleQuery(const QString &text);
    /*!
     * Data download to local file.
     */
    virtual void musicDownloadLocal(int row) override;

    /*!
     * Set search data quality.
     */
    void setSearchQuality(const QString &quality);

    /*!
     * Resize window bound by widget resize called.
     */
    void resizeWindow();

public Q_SLOTS:
    /*!
     * Table widget item cell enter.
     */
    virtual void itemCellEntered(int row, int column) override;
    /*!
     * Table widget item cell click.
     */
    virtual void itemCellClicked(int row, int column) override;
    /*!
     * Clear All Items.
     */
    virtual void clearAllItems() override;
    /*!
     * Create searched items.
     */
    virtual void createSearchedItem(const MusicSearchedItem &songItem) override;
    /*!
     * Item has double clicked.
     */
    virtual void itemDoubleClicked(int row, int column) override;
    /*!
     * Left context menu action group click by action.
     */
    virtual void actionGroupClick(QAction *action) override;
    /*!
     * Search data dwonload finished.
     */
    void searchDataDwonloadFinished();
    /*!
     * Open music song download widget.
     */
    void musicSongDownload(int row);

protected:
    /*!
     * Override the widget event.
     */
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void contextMenuEvent(QContextMenuEvent *event) override;
    /*!
     * Add search music to play list by index.
     */
    void addSearchMusicToPlaylist(int row);

    bool m_queryAllRecords;
    MusicDownloadData m_downloadData;

};



/*! @brief The class of the song search online widget.
 * @author Greedysky <greedysky@163.com>
 */
class TTK_MODULE_EXPORT MusicSongSearchOnlineWidget : public QWidget
{
    Q_OBJECT
    TTK_DECLARE_MODULE(MusicSongSearchOnlineWidget)
public:
    /*!
     * Object contsructor.
     */
    explicit MusicSongSearchOnlineWidget(QWidget *parent = nullptr);
    ~MusicSongSearchOnlineWidget();

    /*!
     * Start search query by text.
     */
    void startSearchQuery(const QString &name, bool all);
    /*!
     * Start search query by given id.
     */
    void startSearchSingleQuery(const QString &name);
    /*!
     * Research query by quality it changed.
     */
    void researchQueryByQuality(const QString &name, const QString &quality);
    /*!
     * Resize window bound by widget resize called.
     */
    void resizeWindow();

public Q_SLOTS:
    /*!
     * Set diff button clicked by index.
     */
    void buttonClicked(int index);

protected:
    /*!
     * Override the widget event.
     */
    virtual void resizeEvent(QResizeEvent *event) override;
    /*!
     * Create tool widget.
     */
    void createToolWidget(QWidget *widget);
    /*!
     * Set resize labelt ext.
     */
    void setResizeLabelText(const QString &name);

    QLabel *m_textLabel;
    QPushButton *m_playButton;
    QList<QWidget*> m_resizeLabels;
    MusicSongSearchTableWidget *m_searchTableWidget;

};

#endif // MUSICSONGSEARCHONLINEWIDGET_H
