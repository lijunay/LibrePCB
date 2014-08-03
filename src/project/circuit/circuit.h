/*
 * EDA4U - Professional EDA for everyone!
 * Copyright (C) 2013 Urban Bruhin
 * http://eda4u.ubruhin.ch/
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

#ifndef PROJECT_CIRCUIT_H
#define PROJECT_CIRCUIT_H

/*****************************************************************************************
 *  Includes
 ****************************************************************************************/

#include <QtCore>

/*****************************************************************************************
 *  Forward Declarations
 ****************************************************************************************/

class QDomDocument;
class QDomElement;

class Workspace;

namespace project {
class Project;
}

/*****************************************************************************************
 *  Class SchematicEditor
 ****************************************************************************************/

namespace project {

class Circuit final : public QObject
{
        Q_OBJECT

    public:

        // Constructors / Destructor
        explicit Circuit(Workspace* workspace, Project* project);
        ~Circuit();

        // General Methods
        void save();

    private:

        // make some methods inaccessible...
        Circuit();
        Circuit(const Circuit& other);
        Circuit& operator=(const Circuit& rhs);

        // General
        Workspace* mWorkspace; ///< A pointer to the Workspace object (from the constructor)
        Project* mProject; ///< A pointer to the Project object (from the constructor)

        // File "core/circuit.xml"
        QString mXmlFilepath;
        QDomDocument* mDomDocument;
        QDomElement* mRootDomElement;

        // Circuit Attributes from the XML file
        QUuid mUuid;

};

} // namespace project

#endif // PROJECT_CIRCUIT_H