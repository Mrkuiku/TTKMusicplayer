#ifndef MUSICWIDGETUTILS_H
#define MUSICWIDGETUTILS_H

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

class QMenu;
class QPainter;
class QComboBox;

#include "musicobject.h"
#include "musicglobaldefine.h"

/*! @brief The namespace of the utils widget.
 * @author Greedysky <greedysky@163.com>
 */
namespace MusicUtils
{
    namespace Widget
    {
        /*!
         * Set widget border shadow.
         */
        TTK_MODULE_EXPORT void setBorderShadow(QWidget *widget, QPainter *painter);
        /*!
         * Set widget label font.
         */
        TTK_MODULE_EXPORT void setLabelFontSize(QWidget *widget, int size);
        /*!
         * Set widget label font.
         */
        TTK_MODULE_EXPORT void setLabelFontStyle(QWidget *widget, MusicObject::FontStyleMode type);
        /*!
         * Set text elided text by font.
         */
        TTK_MODULE_EXPORT QString elidedText(const QFont &font, const QString &text, Qt::TextElideMode mode, int width);
        /*!
         * Get text width by font.
         */
        TTK_MODULE_EXPORT int fontTextWidth(const QFont &font, const QString &text);
        /*!
         * Get font height by font.
         */
        TTK_MODULE_EXPORT int fontTextHeight(const QFont &font);

        /*!
         * Adjust the menu position.
         */
        TTK_MODULE_EXPORT void adjustMenuPosition(QMenu *menu);
        /*!
         * Adjust the widget in screen center.
         */
        TTK_MODULE_EXPORT void positionInCenter(QWidget *widget);

        /*!
         * Set widget transparent.
         */
        TTK_MODULE_EXPORT void setTransparent(QWidget *widget, int alpha);
        /*!
         * Set ComboBox text.
         */
        TTK_MODULE_EXPORT void setComboBoxText(QComboBox *object, const QString &text);
        /*!
         * Set widget to round by ratioX and ratioY.
         */
        TTK_MODULE_EXPORT void widgetToRound(QWidget *w, int ratioX, int ratioY);

    }
}

#endif // MUSICWIDGETUTILS_H
