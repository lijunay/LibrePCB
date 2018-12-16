/*
 * LibrePCB - Professional EDA for everyone!
 * Copyright (C) 2013 LibrePCB Developers, see AUTHORS.md for contributors.
 * https://librepcb.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*******************************************************************************
 *  Includes
 ******************************************************************************/
#include "transactionalfilesystem.h"

#include "fileutils.h"

#include <quazip/quazip.h>
#include <quazip/quazipdir.h>
#include <quazip/quazipfile.h>

/*******************************************************************************
 *  Namespace
 ******************************************************************************/
namespace librepcb {

/*******************************************************************************
 *  Constructors / Destructor
 ******************************************************************************/

TransactionalFileSystem::TransactionalFileSystem(QObject* parent) noexcept
  : FileSystem(parent), mTmpDir(FilePath::getRandomTempPath()) {
}

TransactionalFileSystem::~TransactionalFileSystem() noexcept {
  if (!QDir(mTmpDir.toStr()).removeRecursively()) {
    qWarning() << "Could not remove directory" << mTmpDir.toNative();
  }
}

/*******************************************************************************
 *  File Operations
 ******************************************************************************/

// QString TransactionalFileSystem::getRootName() const noexcept {
//  return mOriginFilePath.getFilename();
//}

QString TransactionalFileSystem::getPrettyPath(const QString& path) const
    noexcept {
  return toPrettyPath(mOriginFilePath, path);
}

QStringList TransactionalFileSystem::getSubDirs(const QString& path) const
    noexcept {
  // if (!path.endsWith('/')) {
  //  path.append('/');
  //}
  QStringList dirnames;
  // foreach (const QString& filepath, mFiles.keys()) {
  //  if (filepath.toLower().startsWith(path))
  //
  //
  //  int     pathlen = filepath.lastIndexOf('/') + 1;
  //  QString path    = filepath.left(pathlen);
  //  if (path.toLower() == dir.toLower()) {
  //    QString filename = filepath.right(filepath.length() - pathlen);
  //    if (filters.isEmpty()) {
  //      filenames.append(filename);
  //    } else {
  //      foreach (const QString& filter, filters) {
  //        QRegExp rx(filter);
  //        rx.setPatternSyntax(QRegExp::Wildcard);
  //        if (rx.exactMatch(filename)) {
  //          filenames.append(filename);
  //          break;
  //        }
  //      }
  //    }
  //  }
  //}
  return dirnames;
}

QStringList TransactionalFileSystem::getFilesInDir(
    QString dir, const QStringList& filters) const {
  if (!dir.endsWith('/')) {
    dir.append('/');
  }
  QStringList filenames;
  foreach (const QString& filepath, mFiles.keys()) {
    int     pathlen = filepath.lastIndexOf('/') + 1;
    QString path    = filepath.left(pathlen);
    if (path.toLower() == dir.toLower()) {
      QString filename = filepath.right(filepath.length() - pathlen);
      if (filters.isEmpty()) {
        filenames.append(filename);
      } else {
        foreach (const QString& filter, filters) {
          QRegExp rx(filter);
          rx.setPatternSyntax(QRegExp::Wildcard);
          if (rx.exactMatch(filename)) {
            filenames.append(filename);
            break;
          }
        }
      }
    }
  }
  return filenames;
}

bool TransactionalFileSystem::fileExists(const QString& path) const noexcept {
  return mFiles.contains(path);
}

QByteArray TransactionalFileSystem::read(const QString& path) const {
  const QByteArray& content = getFile(path);  // can throw
  if (content.isNull()) {
    // no temporary changes -> load from disk
    return FileUtils::readFile(mOriginFilePath.getPathTo(path));  // can throw
  } else {
    return content;
  }
}

void TransactionalFileSystem::write(const QString&    path,
                                    const QByteArray& content) {
  QByteArray& file = getOrCreateFile(path);  // can throw
  file             = content.isNull() ? QByteArray("") : content;
}

FilePath TransactionalFileSystem::createTemporaryFileOnDisk(
    const QString& path) const {
  return FilePath();
}

void TransactionalFileSystem::removeFile(const QString& path) {
  mFiles.remove(path);
  mRemovedFiles.insert(path);
}

void TransactionalFileSystem::removeDirRecursively(const QString& path) {
  // TODO
}

/*******************************************************************************
 *  General Methods
 ******************************************************************************/

void TransactionalFileSystem::loadFromDirectory(const FilePath& fp) {
  mFiles.clear();
  mRemovedFiles.clear();
  mOriginFilePath = fp;
  if (!fp.isExistingDir()) {
    throw LogicError(
        __FILE__, __LINE__,
        QString(tr("The directory \"%1\" does not exist.")).arg(fp.toNative()));
  }
  loadDir(fp);
}

void TransactionalFileSystem::saveToDirectory(const FilePath& fp) {
  // always create root directory, even if file system is empty
  FileUtils::makePath(fp);  // can throw

  // remove no longer existent files
  foreach (const QString& filepath, mRemovedFiles) {
    FilePath absFp = toAbsPath(fp, filepath);  // can throw
    if (absFp.isExistingFile()) {
      FileUtils::removeFile(absFp);  // can throw
      // try to remove empty directories
      FilePath parent = absFp.getParentDir();
      while (parent.isLocatedInDir(fp) && parent.isEmptyDir()) {
        QDir().rmdir(parent.toStr());  // ignore failure
        parent = parent.getParentDir();
      }
    }
  }

  // save new or modified files
  foreach (const QString& filepath, mFiles.keys()) {
    // do not save files which were not modified and already exist (for
    // performance reasons)
    if ((fp == mOriginFilePath) && (!mFiles.value(filepath).isNull())) {
      FilePath absFp = toAbsPath(fp, filepath);     // can throw
      FileUtils::writeFile(absFp, read(filepath));  // can throw
    }
  }
}

void TransactionalFileSystem::saveToZip(const FilePath& fp) {
  QuaZip zip(fp.toStr());
  if (!zip.open(QuaZip::mdCreate)) {
    throw RuntimeError(
        __FILE__, __LINE__,
        QString(tr("Failed to create the ZIP file '%1'.")).arg(fp.toNative()));
  }
  QuaZipFile file(&zip);
  foreach (const QString& filepath, mFiles.keys()) {
    const QByteArray& content = read(filepath);  // can throw
    QuaZipNewInfo     newFileInfo(filepath);
    newFileInfo.setPermissions(QFileDevice::ReadOwner | QFileDevice::ReadGroup |
                               QFileDevice::ReadOther |
                               QFileDevice::WriteOwner);
    if (!file.open(QIODevice::WriteOnly, newFileInfo)) {
      throw RuntimeError(__FILE__, __LINE__);
    }
    qint64 bytesWritten = file.write(content);
    file.close();
    if (bytesWritten != content.length()) {
      throw RuntimeError(__FILE__, __LINE__,
                         QString(tr("Failed to write file '%1'."))
                             .arg(toPrettyPath(fp, filepath)));
    }
  }
  zip.close();
}

/*******************************************************************************
 *  Private Methods
 ******************************************************************************/

const QByteArray& TransactionalFileSystem::getFile(const QString& path) const {
  auto it = mFiles.find(path);
  if (it != mFiles.end()) {
    return *it;
  } else {
    throw RuntimeError(__FILE__, __LINE__,
                       QString(tr("File '%1' does not exist."))
                           .arg(toPrettyPath(mOriginFilePath, path)));
  }
}

QByteArray& TransactionalFileSystem::getOrCreateFile(const QString& path) {
  auto it = mFiles.find(path);
  if (it != mFiles.end()) {
    return *it;
  } else {
    // make sure there is no directory or file with that path (search case
    // insensitive to avoid possible issues with case insensitive file systems)
    foreach (const QString& filepath, mFiles.keys()) {
      QString lower = filepath.toLower();
      if (lower == path.toLower() || lower.startsWith(path.toLower() % "/")) {
        throw RuntimeError(__FILE__, __LINE__,
                           QString(tr("Path '%1' exists already."))
                               .arg(toPrettyPath(mOriginFilePath, path)));
      }
    }
    // stop marking file as removed
    foreach (const QString& filepath, mRemovedFiles) {
      if (filepath.toLower() == path.toLower()) {
        mRemovedFiles.remove(filepath);
      }
    }
    return mFiles[path];  // creates a new entry
  }
}

void TransactionalFileSystem::loadDir(const FilePath& dir,
                                      const QString&  prefix) {
  QDir qDir(dir.toStr());
  qDir.setFilter(QDir::Files | QDir::Dirs | QDir::Hidden |
                 QDir::NoDotAndDotDot);
  foreach (const QFileInfo& info, qDir.entryInfoList()) {
    QString  relFp = prefix + info.fileName();
    FilePath absFp = dir.getPathTo(info.fileName());
    if (info.isFile()) {
      mFiles.insert(relFp, QByteArray());  // Null QByteArray -> lazy load
    } else if (info.isDir()) {
      if (!info.fileName().startsWith('.')) {
        // ignore directories starting with a dot (e.g. ".git" or ".autosave")
        loadDir(absFp, relFp + "/");
      }
    } else {
      qCritical() << "Unknown file item:" << absFp.toNative();
    }
  }
}

QString TransactionalFileSystem::toPrettyPath(const FilePath& root,
                                              const QString&  path) noexcept {
  if (root.isValid()) {
    return root.getPathTo(path).toNative();
  } else {
    return QDir::toNativeSeparators(path);
  }
}

FilePath TransactionalFileSystem::toAbsPath(const FilePath& root,
                                            const QString&  path) {
  FilePath fp = root.getPathTo(path);
  if (!fp.isValid()) {
    throw RuntimeError(__FILE__, __LINE__,
                       QString(tr("Path '%1' is invalid.")).arg(path));
  }
  return fp;
}

/*******************************************************************************
 *  End of File
 ******************************************************************************/

}  // namespace librepcb
