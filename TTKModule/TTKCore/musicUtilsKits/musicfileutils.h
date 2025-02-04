#ifndef MUSICFILEUTILS_H
#define MUSICFILEUTILS_H

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

#include "musicobject.h"
#include "musicglobaldefine.h"

/*! @brief The namespace of the utils file.
 * @author Greedysky <greedysky@163.com>
 */
namespace MusicUtils
{
    namespace File
    {
        /*!
         * Check given dir size is bigger than given size.
         */
        TTK_MODULE_EXPORT void checkCacheSize(quint64 cacheSize, bool disabled, const QString &path);
        /*!
         * Get all files in given dir.
         */
        TTK_MODULE_EXPORT QFileInfoList getFileListByDir(const QString &dpath, bool recursively);
        /*!
         * Get all files in given dir.
         */
        TTK_MODULE_EXPORT QFileInfoList getFileListByDir(const QString &dpath, const QStringList &filter, bool recursively);
        /*!
         * Dir remove recursively.
         */
        TTK_MODULE_EXPORT bool removeRecursively(const QString &dir, bool self = true);

        /*!
         * Get open dir dialog.
         */
        TTK_MODULE_EXPORT QString getOpenDirectoryDialog(QWidget *obj);
        /*!
         * Get open file dialog.
         */
        TTK_MODULE_EXPORT QString getOpenFileDialog(QWidget *obj, const QString &filter = "Images (*.png *.bmp *.jpg)");
        /*!
         * Get open files dialog.
         */
        TTK_MODULE_EXPORT QStringList getOpenFilesDialog(QWidget *obj, const QString &filter = "Images (*.png *.bmp *.jpg)");
        /*!
         * Get save file dialog.
         */
        TTK_MODULE_EXPORT QString getSaveFileDialog(QWidget *obj, const QString &filter = "Images (*.png *.bmp *.jpg)");

    }
}

#endif // MUSICFILEUTILS_H
