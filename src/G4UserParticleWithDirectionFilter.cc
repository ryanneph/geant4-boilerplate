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
// $Id: G4SDParticleFilter.cc 67992 2013-03-13 10:59:57Z gcosmo $
//
// G4VSensitiveDetector
#include "G4SDParticleFilter.hh"
#include "G4UserParticleWithDirectionFilter.hh"
#include "G4Step.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"

////////////////////////////////////////////////////////////////////////////////
// class description:
//
//  This is the class of a filter to be associated with a
// sensitive detector.
//  This class filters steps by partilce definition.
//
// Created: 2005-11-14  Tsukasa ASO.
// Modified: 2017-05-25 Ryan Neph
//
///////////////////////////////////////////////////////////////////////////////

G4UserParticleWithDirectionFilter::G4UserParticleWithDirectionFilter(G4String name, const G4ThreeVector& dir, G4int polarity)
    :G4SDParticleFilter(name),m_dirVect(dir),m_polarity(polarity)
{}

G4UserParticleWithDirectionFilter::G4UserParticleWithDirectionFilter(G4String name, const G4String& particleName, const G4ThreeVector& dir, G4int polarity)
    :G4SDParticleFilter(name, particleName),m_dirVect(dir),m_polarity(polarity)
{}

G4UserParticleWithDirectionFilter::G4UserParticleWithDirectionFilter(G4String name, const std::vector<G4String>& particleNames, const G4ThreeVector& dir, G4int polarity)
    :G4SDParticleFilter(name, particleNames),m_dirVect(dir),m_polarity(polarity)
{}

G4UserParticleWithDirectionFilter::G4UserParticleWithDirectionFilter(G4String name, const std::vector<G4ParticleDefinition*>& particleDef, const G4ThreeVector& dir, G4int polarity)
    :G4SDParticleFilter(name, particleDef),m_dirVect(dir),m_polarity(polarity)
{}

G4bool G4UserParticleWithDirectionFilter::Accept(const G4Step* aStep) const
{
    // get result of ParticleFilter base class
    if (!G4SDParticleFilter::Accept(aStep)) { return FALSE; }

    // check if particle direction matches requested
    G4ThreeVector particledir = aStep->GetPreStepPoint()->GetMomentumDirection();
    if (m_polarity*particledir.dot(m_dirVect) > 0.0) {
        return TRUE;
    } else {
        return FALSE;
    }
}

void G4UserParticleWithDirectionFilter::show(){
    G4cout << "----G4UserParticleWithDirectionFilter-----" << G4endl;
    G4SDParticleFilter::show();
    G4cout << " Direction: " << m_dirVect  << G4endl;
    G4cout << " Polarity:  " << m_polarity << G4endl;
    G4cout << "------------------------------------------" << G4endl;
}
