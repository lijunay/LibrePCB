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
#include "diskfilesystem.h"

#include "fileutils.h"

/*******************************************************************************
 *  Namespace
 ******************************************************************************/
namespace librepcb {

/*******************************************************************************
 *  Constructors / Destructor
 ******************************************************************************/

DiskFileSystem::DiskFileSystem(const FilePath& root, bool readOnly,
                               QObject* parent) noexcept
  : FileSystem(parent), mRoot(root), mReadOnly(readOnly) {
}

DiskFileSystem::~DiskFileSystem() noexcept {
}

/*******************************************************************************
 *  File Operations
 ******************************************************************************/

// QString DiskFileSystem::getRootName() const noexcept {
//  return mRoot.getFilename();
//}

QString DiskFileSystem::getPrettyPath(const QString& path) const noexcept {
  return mRoot.getPathTo(path).toNative();
}

QStringList DiskFileSystem::getSubDirs(const QString& path) const noexcept {
  return QDir(mRoot.getPathTo(path).toStr())
      .entryList(QDir::Dirs | QDir::Hidden | QDir::NoDotAndDotDot);
}

QStringList DiskFileSystem::getFilesInDir(QString            dir,
                                          const QStringList& filters) const {
  QList<FilePath> absPaths =
      FileUtils::getFilesInDirectory(mRoot.getPathTo(dir), filters);
  QStringList filenames;
  foreach (const FilePath& fp, absPaths) { filenames.append(fp.getFilename()); }
  return filenames;
}

bool DiskFileSystem::fileExists(const QString& path) const noexcept {
  return mRoot.getPathTo(path).isExistingFile();
}

QByteArray DiskFileSystem::read(const QString& path) const {
  return FileUtils::readFile(mRoot.getPathTo(path));
}

void DiskFileSystem::write(const QString& path, const QByteArray& content) {
  FileUtils::writeFile(mRoot.getPathTo(path), content);
}

FilePath DiskFileSystem::createTemporaryFileOnDisk(const QString& path) const {
  return mRoot.getPathTo(path);
}

void DiskFileSystem::removeFile(const QString& path) {
  FileUtils::removeFile(mRoot.getPathTo(path));
}

void DiskFileSystem::removeDirRecursively(const QString& path) {
  FileUtils::removeDirRecursively(mRoot.getPathTo(path));
}

/*******************************************************************************
 *  General Methods
 ******************************************************************************/

/*******************************************************************************
 *  Private Methods
 ******************************************************************************/

/*******************************************************************************
 *  End of File
 ******************************************************************************/

}  // namespace librepcb
