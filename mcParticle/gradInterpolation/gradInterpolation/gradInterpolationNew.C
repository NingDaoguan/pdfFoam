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

#include "gradInterpolation.H"
#include "volFields.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

template<class Type>
Foam::autoPtr<Foam::gradInterpolation<Type> >
Foam::gradInterpolation<Type>::New
(
    const word& gradInterpolationType,
    const GeometricField<Type, fvPatchField, volMesh>& psi
)
{
    typename dictionaryConstructorTable::iterator cstrIter =
        dictionaryConstructorTablePtr_->find(gradInterpolationType);

    if (cstrIter == dictionaryConstructorTablePtr_->end())
    {
        FatalErrorIn
        (
            "gradInterpolation::New(const word&, "
            "const GeometricField<Type, fvPatchField, volMesh>&)"
        )   << "Unknown gradInterpolation type " << gradInterpolationType
            << " for field " << psi.name() << nl << nl
            << "Valid gradInterpolation types : " << endl
            << dictionaryConstructorTablePtr_->sortedToc()
            << exit(FatalError);
    }

    return autoPtr<gradInterpolation<Type> >(cstrIter()(psi));
}


template<class Type>
Foam::autoPtr<Foam::gradInterpolation<Type> >
Foam::gradInterpolation<Type>::New
(
    const dictionary& interpolationSchemes,
    const GeometricField<Type, fvPatchField, volMesh>& psi
)
{
    return New(word(interpolationSchemes.lookup(psi.name())), psi);
}


// ************************************************************************* //
