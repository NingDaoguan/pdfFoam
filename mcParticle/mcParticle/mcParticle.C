/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 1991-2010 OpenCFD Ltd.
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#include "mcParticleCloud.H"

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void Foam::mcParticle::updateThermo()
{
    // TODO move to reaction model
    //rho_ = (1. - 3.2*z_*(1.-z_));
}

bool Foam::mcParticle::move(mcParticle::trackData& td)
{
  // SLM constant, temperarily put here C0 = 2.1

    scalar C0 = 2.1;
    // TODO move mixing constants to mixing model
    //scalar Cz = 2.0;

    td.switchProcessor = false;
    td.keepParticle = true;

    const polyMesh& mesh = cloud().pMesh();
    const polyBoundaryMesh& pbMesh = mesh.boundaryMesh();

    scalar deltaT = mesh.time().deltaT().value();
    scalar tEnd = (1.0 - stepFraction())*deltaT;
    scalar dtMax = tEnd;


    while (td.keepParticle && !td.switchProcessor && tEnd > SMALL)
    {
        if (debug)
        {
           Info<< "Time = " << mesh.time().timeName()
                << " deltaT = " << deltaT
                << " tEnd = " << tEnd
                << " steptFraction() = " << stepFraction() << endl;
        }

        // set the lagrangian time-step
        scalar dt = min(dtMax, tEnd);

        // remember which cell the particle is in
        // since this will change if a face is hit
        label celli = cell();

        // Particle does not move with its actual velocity, but with
        // FV interpolated velocity plus particle fluctuation
        // velocity. A particle number correction flux is needed as
        // well (to ensure consistency with FV density field).

        vector correctedUp = UParticle_ - Updf_ + UFap_;
        meshTools::constrainDirection(mesh, mesh.solutionD(), correctedUp);

        cellPointWeight cpwx(mesh, position(), celli, face());
        vector gradRhoFap = td.gradRhoInterp().interpolate(cpwx);

        vector prevPos = position();
        vector destPos = prevPos + dt * (correctedUp - gradRhoFap);
        dt *= trackToFace(destPos, td);

        tEnd -= dt;
        stepFraction() = 1.0 - tEnd/deltaT;

        cellPointWeight cpw(mesh, position(), celli, face());

        // fluid quantities @ particle position
        vector gradPFap = td.gradPInterp().interpolate(cpw);
        scalar kFap = td.kInterp().interpolate(cpw);
        scalar epsilonFap = td.epsilonInterp().interpolate(cpw);
        scalarField Phiap(Phi_.size());
        const PtrList<interpolationCellPoint<scalar> >& pintl = td.PhiInterp();
        forAll(Phi_, PhiI)
        {
            Phiap[PhiI] = pintl[PhiI].interpolate(cpw);
        }
        vector diffUap = td.diffUInterp().interpolate(cpw);

        // interpolate fluid velocity to particle location This
        // quantity is a data member the class.
        // Note: if would be the best if UInterp is interpolating
        // velocities based on faces to get UFap instead of cell
        // center values. Will implemente later.
        UFap_ = td.UInterp().interpolate(cpw);

        //Wiener process (question mark)
        vector dW = sqrt(dt) * vector
            (
                td.mcpc().random().GaussNormal(),
                td.mcpc().random().GaussNormal(),
                td.mcpc().random().GaussNormal()
            );

        // Update velocity
        UParticle_ += - gradPFap/rho_ * dt
            - (0.5 + 0.75 * C0) * epsilonFap / kFap * (UParticle_- Updf_) * dt
            + sqrt(C0 * epsilonFap) * dW
            + diffUap * dt;

        // Scale to ensure consistency on TKE
        mcParticleCloud& mcpc = td.mcpc();
        UParticle_ +=
              (UParticle_ - Updf_)  * dt / mcpc.kRelaxTime().value()
            * (sqrt(mcpc.kfv()()[celli]/mcpc.kcPdf()[celli]) - 1.0);

        // TODO Move mixing to mixing model
        //z_ += -0.5 * Cz *  epsilonFap / kFap * (z_ - zCap) * dt;

        updateThermo();

        if (onBoundary() && td.keepParticle)
        {
            if (isA<processorPolyPatch>(pbMesh[patch(face())]))
            {
                td.switchProcessor = true;
            }
        }
    }

    return td.keepParticle;
}



// Pre-action before hitting patches
bool Foam::mcParticle::hitPatch
(
    const polyPatch& patch,
    mcParticle::trackData& td,
    const label Lb
)
{
    return false;
}


bool Foam::mcParticle::hitPatch
(
    const polyPatch&,
    int&,
    const label
)
{
  return false;
}


void Foam::mcParticle::hitProcessorPatch
(
    const processorPolyPatch&,
    mcParticle::trackData& td
)
{
    td.switchProcessor = true;
}


void Foam::mcParticle::hitProcessorPatch
(
    const processorPolyPatch&,
    int&
)
{}


void Foam::mcParticle::hitWallPatch
(
    const wallPolyPatch& wpp,
    mcParticle::trackData& td
)
{}


void Foam::mcParticle::hitWallPatch
(
    const wallPolyPatch&,
    int&
)
{}


void Foam::mcParticle::hitPatch
(
    const polyPatch&,
    mcParticle::trackData& td
)
{
    td.keepParticle = false;
}


void Foam::mcParticle::hitPatch
(
    const polyPatch&,
    int&
)
{}


void Foam::mcParticle::transformProperties (const tensor& T)
{
    Particle<mcParticle>::transformProperties(T);
    UParticle_ = transform(T, UParticle_);
}


void Foam::mcParticle::transformProperties(const vector& separation)
{
    Particle<mcParticle>::transformProperties(separation);
}


// ************************************************************************* //