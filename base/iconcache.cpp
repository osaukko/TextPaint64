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
 * @file    iconcache.cpp
 * @brief   IconCache implementation.
 */

#include "iconcache.h"

#include <QDir>

// Public interface
//--------------------------------------------------------------------------------------------------

/**
 * @return  Refence to IconCache instance.
 * @see     iIconCache.
 */
IconCache &IconCache::instance()
{
    static IconCache iconCacheInstance;
    return iconCacheInstance;
}

QString IconCache::iconsPath() const
{
    return m_iconsPath;
}

void IconCache::setIconsPath(const QString &iconsPath)
{
    m_iconsPath = iconsPath;
}

// Private interface
//--------------------------------------------------------------------------------------------------

/**
 * Private constructor is used to make this a singleton class.
 * Use IconCache::instance() or iIconCache to access shared object.
 */
IconCache::IconCache()
    : m_iconsPath(":/icons/")
{
}

/**
 * Make icon from image files that matches given <i>nameFilter</i>.
 *
 * @param   nameFilter  Name filter for QDir.
 * @return  QIcon object that contains images that matched name filter.
 *
 * @see     QDir::setNameFilters.
 */
QIcon IconCache::loadIcon(const QStringList &nameFilter) const
{
    QIcon icon;
    appendIcons(icon, m_iconsPath, nameFilter);
    return icon;
}

/**
 * Recursive searching for files that matches given name filter.
 *
 * @param[in,out]   icon        Append files that matches nameFilter into this icon object.
 * @param[in]       path        Search from this path.
 * @param[in]       nameFilter  Name filter for QDir.
 * @see             QDir::setNameFilters.
 */
void IconCache::appendIcons(QIcon &icon, const QString &path, const QStringList &nameFilter) const
{
    QDir dir(path);

    // First check subfolders
    QFileInfoList subfolders = dir.entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot);
    foreach (const QFileInfo subfolder, subfolders)
        appendIcons(icon, subfolder.absoluteFilePath(), nameFilter);

    // Then try to add all files that match given filter
    dir.setNameFilters(nameFilter);
    QFileInfoList iconFiles = dir.entryInfoList(QDir::Files);
    foreach (const QFileInfo &iconFile, iconFiles)
        icon.addFile(iconFile.absoluteFilePath());
}
