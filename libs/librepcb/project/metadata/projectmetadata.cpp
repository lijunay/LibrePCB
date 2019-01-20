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
#include "projectmetadata.h"

#include "../project.h"

#include <librepcb/common/fileio/sexpression.h>
#include <librepcb/common/fileio/transactionalfilesystem.h>

#include <QtCore>

/*******************************************************************************
 *  Namespace
 ******************************************************************************/
namespace librepcb {
namespace project {

/*******************************************************************************
 *  Constructors / Destructor
 ******************************************************************************/

ProjectMetadata::ProjectMetadata(Project& project, bool create)
  : QObject(nullptr),
    mProject(project),
    mUuid(Uuid::createRandom()),
    mName("New Project") {
  qDebug() << "load project metadata...";

  if (create) {
    try {
      mName = mProject.getFilepath().getCompleteBasename();
    } catch (const Exception&) {
      // fall back to default name
    }
    mAuthor  = tr("Unknown");
    mVersion = "v1";
    mCreated = QDateTime::currentDateTime();
  } else {
    QString     fn = "project/metadata.lp";
    QString     fp = mProject.getFileSystem().getPrettyPath(fn);
    SExpression root =
        SExpression::parse(mProject.getFileSystem().readText(fn), fp);

    if (root.getChildByIndex(0)
            .isString()) {  // backward compatibility, remove this some time!
      mUuid = root.getChildByIndex(0).getValue<Uuid>();
    }
    mName    = root.getValueByPath<ElementName>("name");
    mAuthor  = root.getValueByPath<QString>("author");
    mVersion = root.getValueByPath<QString>("version");
    mCreated = root.getValueByPath<QDateTime>("created");
    mAttributes.loadFromDomElement(root);  // can throw
  }

  mLastModified = QDateTime::currentDateTime();

  qDebug() << "metadata successfully loaded!";
}

ProjectMetadata::~ProjectMetadata() noexcept {
}

/*******************************************************************************
 *  Setters
 ******************************************************************************/

void ProjectMetadata::setName(const ElementName& newName) noexcept {
  if (newName != mName) {
    mName = newName;
    emit attributesChanged();
  }
}

void ProjectMetadata::setAuthor(const QString& newAuthor) noexcept {
  if (newAuthor != mAuthor) {
    mAuthor = newAuthor;
    emit attributesChanged();
  }
}

void ProjectMetadata::setVersion(const QString& newVersion) noexcept {
  if (newVersion != mVersion) {
    mVersion = newVersion;
    emit attributesChanged();
  }
}

void ProjectMetadata::setAttributes(
    const AttributeList& newAttributes) noexcept {
  if (newAttributes != mAttributes) {
    mAttributes = newAttributes;
    emit attributesChanged();
  }
}

void ProjectMetadata::updateLastModified() noexcept {
  mLastModified = QDateTime::currentDateTime();
  emit attributesChanged();
}

/*******************************************************************************
 *  General Methods
 ******************************************************************************/

bool ProjectMetadata::save(QStringList& errors) noexcept {
  bool success = true;

  try {
    SExpression doc(serializeToDomElement("librepcb_project_metadata"));
    mProject.getFileSystem().writeText("project/metadata.lp", doc.toString(0));
  } catch (const Exception& e) {
    success = false;
    errors.append(e.getMsg());
  }

  return success;
}

/*******************************************************************************
 *  Private Methods
 ******************************************************************************/

void ProjectMetadata::serialize(SExpression& root) const {
  root.appendChild(mUuid);
  root.appendChild("name", mName, true);
  root.appendChild("author", mAuthor, true);
  root.appendChild("version", mVersion, true);
  root.appendChild("created", mCreated, true);
  mAttributes.serialize(root);
}

/*******************************************************************************
 *  End of File
 ******************************************************************************/

}  // namespace project
}  // namespace librepcb
