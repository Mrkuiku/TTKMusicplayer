#ifndef MUSICBACKGROUNDREMOTEWIDGET_H
#define MUSICBACKGROUNDREMOTEWIDGET_H

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

#include <QWidget>
#include "musicbackgroundlistwidget.h"
#include "musicdownloadbackgroundskinrequest.h"

class QPushButton;
class QListWidgetItem;
class MusicDownloadQueueRequest;

/*! @brief The class of the remote background widget.
 * @author Greedysky <greedysky@163.com>
 */
class TTK_MODULE_EXPORT MusicBackgroundRemoteWidget : public QWidget
{
    Q_OBJECT
    TTK_DECLARE_MODULE(MusicBackgroundRemoteWidget)
public:
    /*!
     * Object contsructor.
     */
    explicit MusicBackgroundRemoteWidget(QWidget *parent = nullptr);
    ~MusicBackgroundRemoteWidget();

    /*!
     * Abort the current download request.
     */
    void abort();

    /*!
     * Output remote skin.
     */
    virtual void outputRemoteSkin(MusicBackgroundImage &image, const QString &data) = 0;

public Q_SLOTS:
    /*!
     * Send download data from net.
     */
    void downLoadFinished(const QString &bytes);
    /*!
     * Send download data from net.
     */
    virtual void downLoadFinished(const MusicSkinRemoteGroups &bytes);

protected:
    /*!
     * Start to download data.
     */
    void startToDownload(const QString &prefix);

    int m_currentIndex;
    MusicSkinRemoteGroups m_groups;
    MusicBackgroundListWidget *m_backgroundList;
    MusicDownloadQueueRequest *m_downloadQueue;
    MusicDownloadBackgroundRemoteRequest *m_downloadRequest;

};


/*! @brief The class of the thunder background widget.
 * @author Greedysky <greedysky@163.com>
 */
class TTK_MODULE_EXPORT MusicBackgroundThunderWidget : public MusicBackgroundRemoteWidget
{
    Q_OBJECT
    TTK_DECLARE_MODULE(MusicBackgroundThunderWidget)
public:
    /*!
     * Object contsructor.
     */
    explicit MusicBackgroundThunderWidget(QWidget *parent = nullptr);
    ~MusicBackgroundThunderWidget();

    /*!
     * Init parameters.
     */
    void initialize();

    /*!
     * Create functions widget.
     */
    QWidget* createFunctionsWidget(bool revert, QWidget *object = nullptr);
    /*!
     * Output remote skin.
     */
    virtual void outputRemoteSkin(MusicBackgroundImage &image, const QString &data) override;

public Q_SLOTS:
    /*!
     * Remote background type selected by index.
     */
    void buttonClicked(int index);
    /*!
     * Send download data from net.
     */
    virtual void downLoadFinished(const MusicSkinRemoteGroups &bytes) override;

protected:
    /*!
     * Create button.
     */
    QPushButton *createButton(const QString &name);
    /*!
     * Button style changed.
     */
    void buttonStyleChanged();

    QWidget *m_functionsWidget;
    QList<QPushButton*> m_functionsItems;

};



/*! @brief The class of the daily background widget.
 * @author Greedysky <greedysky@163.com>
 */
class TTK_MODULE_EXPORT MusicBackgroundDailyWidget : public MusicBackgroundRemoteWidget
{
    Q_OBJECT
    TTK_DECLARE_MODULE(MusicBackgroundDailyWidget)
public:
    /*!
     * Object contsructor.
     */
    explicit MusicBackgroundDailyWidget(QWidget *parent = nullptr);

    /*!
     * Init parameters.
     */
    void initialize();

    /*!
     * Output remote skin.
     */
    virtual void outputRemoteSkin(MusicBackgroundImage &image, const QString &data) override;

public Q_SLOTS:
    /*!
     * Send download data from net.
     */
    virtual void downLoadFinished(const MusicSkinRemoteGroups &bytes) override;

};

#endif // MUSICBACKGROUNDREMOTEWIDGET_H
