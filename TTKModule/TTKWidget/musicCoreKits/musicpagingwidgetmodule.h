#ifndef MUSICPAGINGWIDGETMODULE_H
#define MUSICPAGINGWIDGETMODULE_H

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

#include "musicglobaldefine.h"

class MusicClickedLabel;

/*! @brief The class of the paging widget object.
 * @author Greedysky <greedysky@163.com>
 */
class TTK_MODULE_EXPORT MusicPagingWidgetModule : public QObject
{
    Q_OBJECT
    TTK_DECLARE_MODULE(MusicPagingWidgetModule)
public:
    /*!
     * Object contsructor.
     */
    explicit MusicPagingWidgetModule(QObject *parent = nullptr);
    ~MusicPagingWidgetModule();

    /*!
     * Create paging items.
     */
    QWidget* createPagingWidget(QWidget *parent, int total);
    /*!
     * Reset page to origin.
     */
    void reset(int total);
    /*!
     * Start to page by given index and total.
     */
    void paging(int index, int total);
    /*!
     * Get current page index.
     */
    int currentIndex() const;

Q_SIGNALS:
    /*!
     * Mapped the clicked page index.
     */
    void clicked(int index);

protected:
    int m_currentPage;
    QWidget *m_pagingWidget;
    QList<MusicClickedLabel*> m_pagingItems;

};

#endif // MUSICPAGINGWIDGETMODULE_H
