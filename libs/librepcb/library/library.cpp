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
#include "library.h"

#include "cat/componentcategory.h"
#include "cat/packagecategory.h"
#include "cmp/component.h"
#include "dev/device.h"
#include "pkg/package.h"
#include "sym/symbol.h"

#include <librepcb/common/fileio/sexpression.h>
#include <librepcb/common/toolbox.h>

#include <QtCore>
#include <QtWidgets>

/*******************************************************************************
 *  Namespace
 ******************************************************************************/
namespace librepcb {
namespace library {

/*******************************************************************************
 *  Constructors / Destructor
 ******************************************************************************/

Library::Library(const FileSystemRef& fileSystem, const Uuid& uuid,
                 const Version& version, const QString& author,
                 const ElementName& name_en_US,
                 const QString&     description_en_US,
                 const QString&     keywords_en_US)
  : LibraryBaseElement(fileSystem, getShortElementName(), getLongElementName(),
                       uuid, version, author, name_en_US, description_en_US,
                       keywords_en_US) {
}

Library::Library(const FileSystemRef& fileSystem)
  : LibraryBaseElement(fileSystem, "lib", "library") {
  // read properties
  try {
    mUrl = mLoadingFileDocument.getValueByPath<QUrl>("url");
  } catch (const Exception& e) {
    qWarning() << e.getMsg();
  }

  // read dependency UUIDs
  foreach (const SExpression& node,
           mLoadingFileDocument.getChildren("dependency")) {
    mDependencies.insert(node.getValueOfFirstChild<Uuid>());
  }

  // load image if available
  if (mFileSystem.fileExists(getIconFileName())) {
    mIcon = mFileSystem.read(getIconFileName());  // can throw
  }

  cleanupAfterLoadingElementFromFile();
}

Library::~Library() noexcept {
}

/*******************************************************************************
 *  Getters
 ******************************************************************************/

template <typename ElementType>
FileSystemRef Library::getElementsDirectory() noexcept {
  return FileSystemRef(mFileSystem, ElementType::getShortElementName());
}

// explicit template instantiations
template FileSystemRef
Library::getElementsDirectory<ComponentCategory>() noexcept;
template FileSystemRef
                       Library::getElementsDirectory<PackageCategory>() noexcept;
template FileSystemRef Library::getElementsDirectory<Symbol>() noexcept;
template FileSystemRef Library::getElementsDirectory<Package>() noexcept;
template FileSystemRef Library::getElementsDirectory<Component>() noexcept;
template FileSystemRef Library::getElementsDirectory<Device>() noexcept;

QPixmap Library::getIconAsPixmap() const noexcept {
  QPixmap p;
  p.loadFromData(mIcon, "png");
  return p;
}

/*******************************************************************************
 *  General Methods
 ******************************************************************************/

void Library::save() {
  LibraryBaseElement::save();  // can throw

  // Save icon.
  if (!mIcon.isEmpty()) {
    mFileSystem.write(getIconFileName(), mIcon);  // can throw
  } else if (mFileSystem.fileExists(getIconFileName())) {
    mFileSystem.removeFile(getIconFileName());  // can throw
  }
}

template <typename ElementType>
QList<FileSystemRef> Library::searchForElements() noexcept {
  QList<FileSystemRef> list;
  FileSystemRef        elementTypeDir =
      mFileSystem.getRefToDir(ElementType::getShortElementName());
  foreach (const QString& dirname, elementTypeDir.getSubDirs()) {
    FileSystemRef elementDir(elementTypeDir, dirname);
    if (isValidElementDirectory<ElementType>(elementDir)) {
      list.append(elementDir);
    } else {
      qWarning() << "Directory is not a valid library element:"
                 << elementDir.getPrettyPath();
    }
  }
  return list;
}

// explicit template instantiations
template QList<FileSystemRef>
Library::searchForElements<ComponentCategory>() noexcept;
template QList<FileSystemRef>
                              Library::searchForElements<PackageCategory>() noexcept;
template QList<FileSystemRef> Library::searchForElements<Symbol>() noexcept;
template QList<FileSystemRef> Library::searchForElements<Package>() noexcept;
template QList<FileSystemRef> Library::searchForElements<Component>() noexcept;
template QList<FileSystemRef> Library::searchForElements<Device>() noexcept;

/*******************************************************************************
 *  Private Methods
 ******************************************************************************/

void Library::serialize(SExpression& root) const {
  LibraryBaseElement::serialize(root);
  root.appendChild("url", mUrl, true);
  foreach (const Uuid& uuid, Toolbox::sortedQSet(mDependencies)) {
    root.appendChild("dependency", uuid, true);
  }
}

/*******************************************************************************
 *  End of File
 ******************************************************************************/

}  // namespace library
}  // namespace librepcb
