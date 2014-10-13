/* IconCache - Smart Icon Loading Component
 *
 * Copyright 2014 Centria research and development
 * http://tki.centria.fi/
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     (1) Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *     (2) Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *
 *     (3)The name of the author may not be used to
 *     endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file    iconcache.h
 * @brief   IconCache header file.
 */

#ifndef ICONCACHE_H
#define ICONCACHE_H

#include <QIcon>

/**
 * Macro for easy IconCache instance access.
 */
#define iIconCache  IconCache::instance()

/**
 * Macro that generates access function and variable for icon.
 *
 * @param NAME      Name for icon.
 * @param FILTER    Name filter for icon image files.
 */
#define CACHEICON(NAME, FILTER) \
    public: \
    QIcon NAME() \
{ \
    if (m_##NAME.isNull()) \
    m_##NAME = loadIcon(QStringList(FILTER)); \
    return m_##NAME; \
    } \
    \
    private: \
    QIcon m_##NAME;

/**
 * @brief   Icon cache.
 *
 * Icon cache is sigleton class that uses lazy loading for icons. Icons are
 * loaded from resources when required. Loaded icons are kept in memory untill
 * application exits.
 */
class IconCache
{
    CACHEICON(alignBottom,              "align-bottom.png")
    CACHEICON(alignHCenter,             "align-hcenter.png")
    CACHEICON(alignLeft,                "align-left.png")
    CACHEICON(alignRight,               "align-right.png")
    CACHEICON(alignTop,                 "align-top.png")
    CACHEICON(alignVCenter,             "align-vcenter.png")
    CACHEICON(applicationExit,          "application-exit.png")
    CACHEICON(charsetDocumentOpen,      "charset-document-open.png")
    CACHEICON(charsetDocumentSaveAs,    "charset-document-save-as.png")
    CACHEICON(codeBlock,                "code-block.png")
    CACHEICON(colorsDocumentOpen,       "colors-document-open.png")
    CACHEICON(colorsDocumentSaveAs,     "colors-document-save-as.png")
    CACHEICON(configureToolbars,        "configure-toolbars.png")
    CACHEICON(documentNew,              "document-new.png")
    CACHEICON(documentOpenRecent,       "document-open-recent.png")
    CACHEICON(documentOpen,             "document-open.png")
    CACHEICON(documentRevert,           "document-revert.png")
    CACHEICON(documentSaveAs,           "document-save-as.png")
    CACHEICON(documentSave,             "document-save.png")
    CACHEICON(drawFreehand,             "draw-freehand.png")
    CACHEICON(editClear,                "edit-clear.png")
    CACHEICON(editRedo,                 "edit-redo.png")
    CACHEICON(editUndo,                 "edit-undo.png")
    CACHEICON(fillColor,                "fill-color.png")
    CACHEICON(mediaFlash,               "media-flash.png")
    CACHEICON(pageZoom,                 "page-zoom.png")
    CACHEICON(textPaint64,              "text-paint-64.png")
    CACHEICON(qtLogo,                   "qt-logo.png")
    CACHEICON(screenDocumentOpen,       "screen-document-open.png")
    CACHEICON(screenDocumentSaveAs,     "screen-document-save-as.png")
    CACHEICON(viewForm,                 "view-form.png")
    CACHEICON(viewGrid,                 "view-grid.png")

public:
    static IconCache &instance();
    QString iconsPath() const;
    void setIconsPath(const QString &iconsPath);

private:
    explicit IconCache();
    QIcon loadIcon(const QStringList &nameFilter) const;
    void appendIcons(QIcon &icon, const QString &path, const QStringList &nameFilter) const;

private:
    QString m_iconsPath;
};

#endif // ICONCACHE_H
