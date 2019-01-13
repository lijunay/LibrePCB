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
#include "newelementwizardcontext.h"

#include <librepcb/library/elements.h>
#include <librepcb/workspace/settings/workspacesettings.h>
#include <librepcb/workspace/workspace.h>

/*******************************************************************************
 *  Namespace
 ******************************************************************************/
namespace librepcb {
namespace library {
namespace editor {

/*******************************************************************************
 *  Constructors / Destructor
 ******************************************************************************/

NewElementWizardContext::NewElementWizardContext(
    const workspace::Workspace& ws, Library& lib,
    const IF_GraphicsLayerProvider& lp, QObject* parent) noexcept
  : QObject(parent),
    mWorkspace(ws),
    mLibrary(lib),
    mLayerProvider(lp),
    mElementType(ElementType::None),
    mComponentPrefixes(ComponentPrefix("")) {
  reset();
}

NewElementWizardContext::~NewElementWizardContext() noexcept {
}

/*******************************************************************************
 *  Getters
 ******************************************************************************/

const QStringList& NewElementWizardContext::getLibLocaleOrder() const noexcept {
  return mWorkspace.getSettings().getLibLocaleOrder().getLocaleOrder();
}

/*******************************************************************************
 *  Private Methods
 ******************************************************************************/

void NewElementWizardContext::reset() noexcept {
  // common
  mElementType = ElementType::None;
  mElementName = tl::nullopt;
  mElementDescription.clear();
  mElementKeywords.clear();
  mElementAuthor       = mWorkspace.getSettings().getUser().getName();
  mElementVersion      = Version::fromString("0.1");
  mElementCategoryUuid = tl::nullopt;

  // symbol
  mSymbolPins.clear();
  mSymbolPolygons.clear();
  mSymbolCircles.clear();
  mSymbolTexts.clear();

  // package
  mPackagePads.clear();
  mPackageFootprints.clear();

  // component
  mComponentSchematicOnly = false;
  mComponentAttributes.clear();
  mComponentDefaultValue.clear();
  mComponentPrefixes = NormDependentPrefixMap(ComponentPrefix(""));
  mComponentSignals.clear();
  mComponentSymbolVariants.clear();

  // device
  mDeviceComponentUuid = tl::nullopt;
  mDevicePackageUuid   = tl::nullopt;
}

void NewElementWizardContext::createLibraryElement() {
  QSet<Uuid> categories;
  if (mElementCategoryUuid) {
    categories.insert(*mElementCategoryUuid);
  }

  if (!mElementName) throw LogicError(__FILE__, __LINE__);
  if (!mElementVersion) throw LogicError(__FILE__, __LINE__);

  Uuid uuid = Uuid::createRandom();

  switch (mElementType) {
    case NewElementWizardContext::ElementType::ComponentCategory: {
      mOutputDirectory =
          mLibrary.getElementsDirectory<ComponentCategory>().getRefToDir(
              uuid.toStr());
      ComponentCategory element(mOutputDirectory, uuid, *mElementVersion,
                                mElementAuthor, *mElementName,
                                mElementDescription, mElementKeywords);
      element.setParentUuid(mElementCategoryUuid);
      element.save();
      break;
    }
    case NewElementWizardContext::ElementType::PackageCategory: {
      mOutputDirectory =
          mLibrary.getElementsDirectory<PackageCategory>().getRefToDir(
              uuid.toStr());
      PackageCategory element(mOutputDirectory, uuid, *mElementVersion,
                              mElementAuthor, *mElementName,
                              mElementDescription, mElementKeywords);
      element.setParentUuid(mElementCategoryUuid);
      element.save();
      break;
    }
    case NewElementWizardContext::ElementType::Symbol: {
      mOutputDirectory =
          mLibrary.getElementsDirectory<Symbol>().getRefToDir(uuid.toStr());
      Symbol element(mOutputDirectory, uuid, *mElementVersion, mElementAuthor,
                     *mElementName, mElementDescription, mElementKeywords);
      element.setCategories(categories);
      element.getPins()     = mSymbolPins;
      element.getPolygons() = mSymbolPolygons;
      element.getCircles()  = mSymbolCircles;
      element.getTexts()    = mSymbolTexts;
      element.save();
      break;
    }
    case NewElementWizardContext::ElementType::Package: {
      mOutputDirectory =
          mLibrary.getElementsDirectory<Package>().getRefToDir(uuid.toStr());
      Package element(mOutputDirectory, uuid, *mElementVersion, mElementAuthor,
                      *mElementName, mElementDescription, mElementKeywords);
      element.setCategories(categories);
      element.getPads()       = mPackagePads;
      element.getFootprints() = mPackageFootprints;
      if (element.getFootprints().isEmpty()) {
        element.getFootprints().append(std::make_shared<Footprint>(
            Uuid::createRandom(), ElementName("default"), ""));
      }
      element.save();
      break;
    }
    case NewElementWizardContext::ElementType::Component: {
      mOutputDirectory =
          mLibrary.getElementsDirectory<Component>().getRefToDir(uuid.toStr());
      Component element(mOutputDirectory, uuid, *mElementVersion,
                        mElementAuthor, *mElementName, mElementDescription,
                        mElementKeywords);
      element.setCategories(categories);
      element.setIsSchematicOnly(mComponentSchematicOnly);
      element.setAttributes(mComponentAttributes);
      element.setDefaultValue(mComponentDefaultValue);
      element.setPrefixes(mComponentPrefixes);
      element.getSignals()        = mComponentSignals;
      element.getSymbolVariants() = mComponentSymbolVariants;
      element.save();
      break;
    }
    case NewElementWizardContext::ElementType::Device: {
      if (!mDeviceComponentUuid) throw LogicError(__FILE__, __LINE__);
      if (!mDevicePackageUuid) throw LogicError(__FILE__, __LINE__);
      mOutputDirectory =
          mLibrary.getElementsDirectory<Device>().getRefToDir(uuid.toStr());
      Device element(mOutputDirectory, uuid, *mElementVersion, mElementAuthor,
                     *mElementName, mElementDescription, mElementKeywords,
                     *mDeviceComponentUuid, *mDevicePackageUuid);
      element.setCategories(categories);
      element.getPadSignalMap() = mDevicePadSignalMap;
      element.save();
      break;
    }
    default:
      throw LogicError(__FILE__, __LINE__);
  }
}

/*******************************************************************************
 *  End of File
 ******************************************************************************/

}  // namespace editor
}  // namespace library
}  // namespace librepcb
