//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
//
// $Id: G4SDParticleFilter.hh 67992 2013-03-13 10:59:57Z gcosmo $
//

#ifndef G4UserParticleFilter_h
#define G4UserParticleFilter_h 1

class G4Step;
class G4ParticleDefinition;
#include "globals.hh"
#include "G4ThreeVector.hh"
#include "G4VSDFilter.hh"
#include "G4SDParticleFilter.hh"

#include <vector>

////////////////////////////////////////////////////////////////////////////////
// class description:
//
//  This is the class of a filter to be associated with a
// sensitive detector.
//  This class filters steps by partilce definition.
// The particles are given at constructor or add() method.
//
// Created: 2005-11-14  Tsukasa ASO.
// 2010-07-22 T.Aso Filter for Ions
// Modified: 2017-05-25 Ryan Neph
//
///////////////////////////////////////////////////////////////////////////////

class G4UserParticleWithDirectionFilter : public G4SDParticleFilter
{

    public: // with description
        G4UserParticleWithDirectionFilter(G4String name, const G4ThreeVector& dir, G4int polarity=1);
        G4UserParticleWithDirectionFilter(G4String name, const G4String& particleName, const G4ThreeVector& dir, G4int polarity=1);
        G4UserParticleWithDirectionFilter(G4String name, const std::vector<G4String>&  particleNames, const G4ThreeVector& dir, G4int polarity=1);
        G4UserParticleWithDirectionFilter(G4String name, const std::vector<G4ParticleDefinition*>& particleDef, const G4ThreeVector& dir, G4int polarity=1);

    public: // with description
        virtual G4bool Accept(const G4Step*) const;
        void show();

    private:
        G4ThreeVector m_dirVect;
        G4int         m_polarity;
};

#endif

