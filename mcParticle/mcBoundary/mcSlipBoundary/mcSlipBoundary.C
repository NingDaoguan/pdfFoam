/*---------------------------------------------------------------------------*\
                pdfFoam: General Purpose PDF Solution Algorithm
                   for Reactive Flow Simulations in OpenFOAM

 Copyright (C) 2012 Michael Wild, Heng Xiao, Patrick Jenny,
                    Institute of Fluid Dynamics, ETH Zurich
-------------------------------------------------------------------------------
License
    This file is part of pdfFoam.

    pdfFoam is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) version 3 of the same License.

    pdfFoam is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with pdfFoam.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#include "mcSlipBoundary.H"

#include "addToRunTimeSelectionTable.H"
#include "mcParticle.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{

    defineTypeNameAndDebug(mcSlipBoundary, 0);
    addNamedToRunTimeSelectionTable
    (
        mcBoundary,
        mcSlipBoundary,
        mcBoundary,
        slip
    );

} // namespace Foam

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::mcSlipBoundary::mcSlipBoundary
(
    mcParticleCloud& cloud,
    label patchID,
    const dictionary& dict
)
:
    mcBoundary(cloud, patchID, dict)
{}

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void Foam::mcSlipBoundary::hitPatch
(
    mcParticle& p,
#if FOAM_HEX_VERSION < 0x200
    mcParticle::trackData& td
#else
    mcParticle::trackData& td,
    const label patchI,
    const scalar trackFraction,
    const tetIndices& tetIs
#endif
)
{
    const polyPatch& pp = patch().patch();
    vector nw = pp.faceAreas()[pp.whichFace(p.face())];
    nw /= mag(nw); // Wall normal (outward)
    p.transformProperties(I - 2.0*nw*nw);
    p.reflected() = true;
}


#if FOAM_HEX_VERSION < 0x200
void Foam::mcSlipBoundary::hitPatch
(
    Foam::mcParticle& p,
    int&
)
{}
#endif

// ************************************************************************* //
