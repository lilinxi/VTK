vtk_module(vtkAMRCore
  GROUPS
    StandAlone
  DEPENDS
    vtkParallelCore
    vtkFiltersGeneral
    vtkhdf5
    vtkIOXML
  TEST_DEPENDS
    vtkTestingCore
    vtkTestingRendering
  )
