/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkImageAppendComponents.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkImageAppendComponents.h"

#include "vtkImageData.h"
#include "vtkImageProgressIterator.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkDataSetAttributes.h"
#include "vtkStreamingDemandDrivenPipeline.h"

vtkCxxRevisionMacro(vtkImageAppendComponents, "1.30");
vtkStandardNewMacro(vtkImageAppendComponents);

//----------------------------------------------------------------------------
// This method tells the ouput it will have more components
int vtkImageAppendComponents::RequestInformation (
  vtkInformation * vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  vtkInformation *inScalarInfo;
  
  int idx1, num;
  num = 0;
  for (idx1 = 0; idx1 < this->GetNumberOfInputConnections(0); ++idx1)
    {
    inScalarInfo = vtkDataObject::GetActiveFieldInformation(
      inputVector[0]->GetInformationObject(idx1), 
      vtkDataObject::FIELD_ASSOCIATION_POINTS, vtkDataSetAttributes::SCALARS);
    if (inScalarInfo && 
      inScalarInfo->Has( vtkDataObject::FIELD_NUMBER_OF_COMPONENTS() ) )
      {
      num += inScalarInfo->Get( vtkDataObject::FIELD_NUMBER_OF_COMPONENTS() );
      }
    }

  vtkDataObject::SetPointDataActiveScalarInfo(outInfo, -1, num);
  return 1;
}

//----------------------------------------------------------------------------
// This templated function executes the filter for any type of data.
template <class T>
void vtkImageAppendComponentsExecute(vtkImageAppendComponents *self,
                                     vtkImageData *inData, 
                                     vtkImageData *outData, 
                                     int outComp,
                                     int outExt[6], int id, T *)
{
  vtkImageIterator<T> inIt(inData, outExt);
  vtkImageProgressIterator<T> outIt(outData, outExt, self, id);
  int numIn = inData->GetNumberOfScalarComponents();
  int numSkip = outData->GetNumberOfScalarComponents() - numIn;
  int i;
  
  // Loop through ouput pixels
  while (!outIt.IsAtEnd())
    {
    T* inSI = inIt.BeginSpan();
    T* outSI = outIt.BeginSpan() + outComp;
    T* outSIEnd = outIt.EndSpan();
    while (outSI < outSIEnd)
      {
      // now process the components
      for (i = 0; i < numIn; ++i)
        {
        *outSI = *inSI;
        ++outSI;
        ++inSI;
        }
      outSI = outSI + numSkip;
      }
    inIt.NextSpan();
    outIt.NextSpan();
    }
}

//----------------------------------------------------------------------------
int vtkImageAppendComponents::FillInputPortInformation(int i, 
                                                       vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_IS_REPEATABLE(), 1);
  return this->Superclass::FillInputPortInformation(i,info);
}

//----------------------------------------------------------------------------
// This method is passed a input and output regions, and executes the filter
// algorithm to fill the output from the inputs.
// It just executes a switch statement to call the correct function for
// the regions data types.
void vtkImageAppendComponents::ThreadedRequestData (  
  vtkInformation * vtkNotUsed( request ), 
  vtkInformationVector** vtkNotUsed( inputVector ), 
  vtkInformationVector * vtkNotUsed( outputVector ),
  vtkImageData ***inData, 
  vtkImageData **outData,
  int outExt[6], int id)
{
  int idx1, outComp;

  outComp = 0;
  for (idx1 = 0; idx1 < this->GetNumberOfInputConnections(0); ++idx1)
    {
    if (inData[0][idx1] != NULL)
      {
      // this filter expects that input is the same type as output.
      if (inData[0][idx1]->GetScalarType() != outData[0]->GetScalarType())
        {
        vtkErrorMacro(<< "Execute: input" << idx1 << " ScalarType (" 
                      << inData[0][idx1]->GetScalarType() 
                      << "), must match output ScalarType (" 
                      << outData[0]->GetScalarType() << ")");
        return;
        }
      switch (inData[0][idx1]->GetScalarType())
        {
        vtkTemplateMacro(
          vtkImageAppendComponentsExecute ( this, 
                                            inData[0][idx1], outData[0], 
                                            outComp, outExt, id, 
                                            static_cast<VTK_TT *>(0)) );
       default:
         vtkErrorMacro(<< "Execute: Unknown ScalarType");
         return;
       }
      outComp += inData[0][idx1]->GetNumberOfScalarComponents();
      }
    }
}

