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
#include "librarybaseelement.h"

#include "librarybaseelementcheck.h"

#include <librepcb/common/application.h>
#include <librepcb/common/fileio/sexpression.h>
#include <librepcb/common/fileio/versionfile.h>

#include <QtCore>

/*******************************************************************************
 *  Namespace
 ******************************************************************************/
namespace librepcb {
namespace library {

/*******************************************************************************
 *  Constructors / Destructor
 ******************************************************************************/

LibraryBaseElement::LibraryBaseElement(
    const FileSystemRef& fileSystem, const QString& shortElementName,
    const QString& longElementName, const Uuid& uuid, const Version& version,
    const QString& author, const ElementName& name_en_US,
    const QString& description_en_US, const QString& keywords_en_US)
  : QObject(nullptr),
    mFileSystem(fileSystem),
    mShortElementName(shortElementName),
    mLongElementName(longElementName),
    mUuid(uuid),
    mVersion(version),
    mAuthor(author),
    mCreated(QDateTime::currentDateTime()),
    mIsDeprecated(false),
    mNames(name_en_US),
    mDescriptions(description_en_US),
    mKeywords(keywords_en_US) {
}

LibraryBaseElement::LibraryBaseElement(const FileSystemRef& fileSystem,
                                       const QString&       shortElementName,
                                       const QString&       longElementName)
  : QObject(nullptr),
    mFileSystem(fileSystem),
    mShortElementName(shortElementName),
    mLongElementName(longElementName),
    mUuid(Uuid::createRandom()),  // just for initialization, will be
                                  // overwritten
    mVersion(Version::fromString(
        "0.1")),  // just for initialization, will be overwritten
    mNames(ElementName(
        "unknown")),  // just for initialization, will be overwritten
    mDescriptions(""),
    mKeywords("") {
  // determine the filepath to the version file
  QString versionFileName = ".librepcb-" % mShortElementName;

  // check if the directory is a library element
  if (!mFileSystem.fileExists(versionFileName)) {
    throw RuntimeError(
        __FILE__, __LINE__,
        QString(tr("Directory is not a library element of type %1: \"%2\""))
            .arg(mLongElementName, mFileSystem.getPrettyPath("")));
  }

  // read version number from version file
  VersionFile versionFile =
      VersionFile::fromByteArray(mFileSystem.readBinary(versionFileName));
  if (versionFile.getVersion() > qApp->getAppVersion()) {
    throw RuntimeError(
        __FILE__, __LINE__,
        QString(
            tr("The library element %1 was created with a newer application "
               "version. You need at least LibrePCB version %2 to open it."))
            .arg(mFileSystem.getPrettyPath(""))
            .arg(versionFile.getVersion().toPrettyStr(3)));
  }

  // open main file
  QString sexprFileName = mLongElementName % ".lp";
  QString sexprFilePath = mFileSystem.getPrettyPath(sexprFileName);
  mLoadingFileDocument =
      SExpression::parse(mFileSystem.readText(sexprFileName), sexprFilePath);

  // read attributes
  if (mLoadingFileDocument.getChildByIndex(0).isString()) {
    mUuid = mLoadingFileDocument.getChildByIndex(0).getValue<Uuid>();
  } else {
    // backward compatibility, remove this some time!
    mUuid = mLoadingFileDocument.getValueByPath<Uuid>("uuid");
  }
  mVersion      = mLoadingFileDocument.getValueByPath<Version>("version");
  mAuthor       = mLoadingFileDocument.getValueByPath<QString>("author");
  mCreated      = mLoadingFileDocument.getValueByPath<QDateTime>("created");
  mIsDeprecated = mLoadingFileDocument.getValueByPath<bool>("deprecated");

  // read names, descriptions and keywords in all available languages
  mNames        = LocalizedNameMap(mLoadingFileDocument);
  mDescriptions = LocalizedDescriptionMap(mLoadingFileDocument);
  mKeywords     = LocalizedKeywordsMap(mLoadingFileDocument);
}

LibraryBaseElement::~LibraryBaseElement() noexcept {
}

/*******************************************************************************
 *  Getters
 ******************************************************************************/

QStringList LibraryBaseElement::getAllAvailableLocales() const noexcept {
  QStringList list;
  list.append(mNames.keys());
  list.append(mDescriptions.keys());
  list.append(mKeywords.keys());
  list.removeDuplicates();
  list.sort(Qt::CaseSensitive);
  return list;
}

/*******************************************************************************
 *  General Methods
 ******************************************************************************/

LibraryElementCheckMessageList LibraryBaseElement::runChecks() const {
  LibraryBaseElementCheck check(*this);
  return check.runChecks();  // can throw
}

void LibraryBaseElement::save() {
  // save S-Expressions file
  QString     sexprFileName = mLongElementName % ".lp";
  SExpression root(serializeToDomElement("librepcb_" % mLongElementName));
  mFileSystem.writeText(sexprFileName, root.toString(0));

  // save version number file
  QString     versionFileName = ".librepcb-" % mShortElementName;
  VersionFile versionFile(qApp->getFileFormatVersion());
  mFileSystem.writeBinary(versionFileName, versionFile.toByteArray());
}

/*******************************************************************************
 *  Protected Methods
 ******************************************************************************/

void LibraryBaseElement::cleanupAfterLoadingElementFromFile() noexcept {
  mLoadingFileDocument = SExpression();  // destroy the whole DOM tree
}

void LibraryBaseElement::serialize(SExpression& root) const {
  root.appendChild(mUuid);
  mNames.serialize(root);
  mDescriptions.serialize(root);
  mKeywords.serialize(root);
  root.appendChild("author", mAuthor, true);
  root.appendChild("version", mVersion, true);
  root.appendChild("created", mCreated, true);
  root.appendChild("deprecated", mIsDeprecated, true);
}

/*******************************************************************************
 *  End of File
 ******************************************************************************/

}  // namespace library
}  // namespace librepcb
