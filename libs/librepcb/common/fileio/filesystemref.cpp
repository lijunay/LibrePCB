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
#include "filesystemref.h"

#include "filesystem.h"

/*******************************************************************************
 *  Namespace
 ******************************************************************************/
namespace librepcb {

/*******************************************************************************
 *  Constructors / Destructor
 ******************************************************************************/

FileSystemRef::FileSystemRef() noexcept : mFileSystem(), mRoot() {
}

FileSystemRef::FileSystemRef(const FileSystemRef& other,
                             const QString&       relpath) noexcept
  : mFileSystem(other.mFileSystem), mRoot(other.getAbsPath(relpath)) {
}

FileSystemRef::FileSystemRef(FileSystem& fs, const QString& root) noexcept
  : mFileSystem(&fs), mRoot(root) {
  Q_ASSERT(!mRoot.endsWith('/'));
}

FileSystemRef::~FileSystemRef() noexcept {
}

/*******************************************************************************
 *  General Methods
 ******************************************************************************/

FileSystemRef FileSystemRef::getRefToDir(const QString& dir) const noexcept {
  Q_ASSERT(!dir.isEmpty());
  return FileSystemRef(*this, dir);
}

/*******************************************************************************
 *  File Operations
 ******************************************************************************/

// QString FileSystemRef::getRootName() const noexcept {
//  if (mRoot.isEmpty() && mFileSystem) {
//    return mFileSystem->getRootName();
//  } else {
//    return mRoot.section('/', -1);
//  }
//}

QString FileSystemRef::getPrettyPath(const QString& path) const noexcept {
  if (mFileSystem) {
    return mFileSystem->getPrettyPath(getAbsPath(path));
  } else {
    qCritical() << "FileSystemRef::getPrettyPath() called without filesystem.";
    return QString();
  }
}

QStringList FileSystemRef::getSubDirs(const QString& path) const noexcept {
  if (mFileSystem) {
    return mFileSystem->getSubDirs(getAbsPath(path));
  } else {
    qCritical() << "FileSystemRef::getSubDirs() called without filesystem.";
    return QStringList();
  }
}

QStringList FileSystemRef::getFilesInDir(QString            dir,
                                         const QStringList& filters) const {
  if (mFileSystem) {
    return mFileSystem->getFilesInDir(getAbsPath(dir), filters);
  } else {
    qCritical() << "FileSystemRef::getFilesInDir() called without filesystem.";
    return QStringList();
  }
}

bool FileSystemRef::fileExists(const QString& path) const noexcept {
  if (mFileSystem) {
    return mFileSystem->fileExists(getAbsPath(path));
  } else {
    qCritical() << "FileSystemRef::fileExists() called without filesystem.";
    return false;
  }
}

QByteArray FileSystemRef::read(const QString& path) const {
  if (mFileSystem) {
    return mFileSystem->read(getAbsPath(path));
  } else {
    qCritical() << "FileSystemRef::readBinary() called without filesystem.";
    return QByteArray();
  }
}

void FileSystemRef::write(const QString& path, const QByteArray& content) {
  if (mFileSystem) {
    mFileSystem->write(getAbsPath(path), content);
  } else {
    qCritical() << "FileSystemRef::writeBinary() called without filesystem.";
  }
}

FilePath FileSystemRef::createTemporaryFileOnDisk(const QString& path) const {
  if (mFileSystem) {
    return mFileSystem->createTemporaryFileOnDisk(getAbsPath(path));
  } else {
    qCritical() << "FileSystemRef::createTemporaryFileOnDisk() called without "
                   "filesystem.";
    return FilePath();
  }
}

void FileSystemRef::removeFile(const QString& path) {
  if (mFileSystem) {
    mFileSystem->removeFile(getAbsPath(path));
  } else {
    qCritical() << "FileSystemRef::removeFile() called without filesystem.";
  }
}

void FileSystemRef::removeDirRecursively(const QString& path) {
  if (mFileSystem) {
    mFileSystem->removeDirRecursively(getAbsPath(path));
  } else {
    qCritical()
        << "FileSystemRef::removeDirRecursively() called without filesystem.";
  }
}

/*******************************************************************************
 *  Operator Overloadings
 ******************************************************************************/

FileSystemRef& FileSystemRef::operator=(const FileSystemRef& rhs) noexcept {
  mFileSystem = rhs.mFileSystem;
  mRoot       = rhs.mRoot;
  return *this;
}

/*******************************************************************************
 *  Private Methods
 ******************************************************************************/

QString FileSystemRef::getAbsPath(const QString& path) const noexcept {
  return mRoot % "/" % path;
}

/*******************************************************************************
 *  End of File
 ******************************************************************************/

}  // namespace librepcb
