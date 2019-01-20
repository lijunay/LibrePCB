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
#include "ercmsglist.h"

#include "../project.h"
#include "ercmsg.h"
#include "if_ercmsgprovider.h"

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

ErcMsgList::ErcMsgList(Project& project)
  : QObject(&project), mProject(project) {
}

ErcMsgList::~ErcMsgList() noexcept {
  Q_ASSERT(mItems.isEmpty());
}

/*******************************************************************************
 *  General Methods
 ******************************************************************************/

void ErcMsgList::add(ErcMsg* ercMsg) noexcept {
  Q_ASSERT(ercMsg);
  Q_ASSERT(!mItems.contains(ercMsg));
  Q_ASSERT(!ercMsg->isIgnored());
  mItems.append(ercMsg);
  emit ercMsgAdded(ercMsg);
}

void ErcMsgList::remove(ErcMsg* ercMsg) noexcept {
  Q_ASSERT(ercMsg);
  Q_ASSERT(mItems.contains(ercMsg));
  Q_ASSERT(!ercMsg->isIgnored());
  mItems.removeOne(ercMsg);
  emit ercMsgRemoved(ercMsg);
}

void ErcMsgList::update(ErcMsg* ercMsg) noexcept {
  Q_ASSERT(ercMsg);
  Q_ASSERT(mItems.contains(ercMsg));
  Q_ASSERT(ercMsg->isVisible());
  emit ercMsgChanged(ercMsg);
}

void ErcMsgList::restoreIgnoreState() {
  QString fn = "circuit/erc.lp";
  if (mProject.getFileSystem().fileExists(fn)) {
    SExpression root = SExpression::parse(mProject.getFileSystem().read(fn));

    // reset all ignore attributes
    foreach (ErcMsg* ercMsg, mItems)
      ercMsg->setIgnored(false);

    // scan approved items and set ignore attributes
    foreach (const SExpression& node, root.getChildren("approved")) {
      foreach (ErcMsg* ercMsg, mItems) {
        if ((ercMsg->getOwner().getErcMsgOwnerClassName() ==
             node.getValueByPath<QString>("class")) &&
            (ercMsg->getOwnerKey() ==
             node.getValueByPath<QString>("instance")) &&
            (ercMsg->getMsgKey() == node.getValueByPath<QString>("message"))) {
          ercMsg->setIgnored(true);
        }
      }
    }
  }
}

bool ErcMsgList::save(QStringList& errors) noexcept {
  bool success = true;

  // Save "circuit/erc.lp"
  try {
    SExpression doc(serializeToDomElement("librepcb_erc"));
    mProject.getFileSystem().write("circuit/erc.lp", doc.toByteArray());
  } catch (Exception& e) {
    success = false;
    errors.append(e.getMsg());
  }

  return success;
}

/*******************************************************************************
 *  Private Methods
 ******************************************************************************/

void ErcMsgList::serialize(SExpression& root) const {
  foreach (ErcMsg* ercMsg, mItems) {
    if (ercMsg->isIgnored()) {
      SExpression& itemNode = root.appendList("approved", true);
      itemNode.appendChild<QString>(
          "class", ercMsg->getOwner().getErcMsgOwnerClassName(), true);
      itemNode.appendChild("instance", ercMsg->getOwnerKey(), true);
      itemNode.appendChild("message", ercMsg->getMsgKey(), true);
    }
  }
}

/*******************************************************************************
 *  End of File
 ******************************************************************************/

}  // namespace project
}  // namespace librepcb
