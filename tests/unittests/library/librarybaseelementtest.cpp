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
#include <gtest/gtest.h>
#include <librepcb/common/fileio/diskfilesystem.h>
#include <librepcb/common/fileio/fileutils.h>
#include <librepcb/library/librarybaseelement.h>

#include <QtCore>

/*******************************************************************************
 *  Namespace
 ******************************************************************************/
namespace librepcb {
namespace library {
namespace tests {

/*******************************************************************************
 *  Test Class
 ******************************************************************************/

class LibraryBaseElementTest : public ::testing::Test {
protected:
  FilePath                           mTempDir;
  QScopedPointer<DiskFileSystem>     mFileSystem;
  QScopedPointer<LibraryBaseElement> mNewElement;

  LibraryBaseElementTest() {
    mTempDir = FilePath::getRandomTempPath();
    mFileSystem.reset(new DiskFileSystem(mTempDir, false));
    mNewElement.reset(new LibraryBaseElement(
        *mFileSystem, "sym", "symbol", Uuid::createRandom(),
        Version::fromString("1.0"), "test", ElementName("Test"), "", ""));
  }

  virtual ~LibraryBaseElementTest() {
    QDir(mTempDir.toStr()).removeRecursively();
  }
};

/*******************************************************************************
 *  Test Methods
 ******************************************************************************/

TEST_F(LibraryBaseElementTest, testSave) {
  mNewElement->save();
}

TEST_F(LibraryBaseElementTest, testSaveToNonExistingDirectory) {
  ASSERT_FALSE(mTempDir.isExistingDir());
  mNewElement->save();
  EXPECT_TRUE(mTempDir.getPathTo("symbol.lp").isExistingFile());
}

TEST_F(LibraryBaseElementTest, testSaveToEmptyDirectory) {
  // Saving into empty destination directory must work because empty directories
  // are sometimes created "accidentally" (for example by Git operations which
  // remove files, but not their parent directories). So we handle empty
  // directories like they are not existent...
  FileUtils::makePath(mTempDir);
  ASSERT_TRUE(mTempDir.isExistingDir());
  mNewElement->save();
  EXPECT_TRUE(mTempDir.getPathTo("symbol.lp").isExistingFile());
}

/*******************************************************************************
 *  End of File
 ******************************************************************************/

}  // namespace tests
}  // namespace library
}  // namespace librepcb
