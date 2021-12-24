
cmake_minimum_required(VERSION 3.3 FATAL_ERROR)

project(gravity_align_3d)

find_package(VTK COMPONENTS 
  vtkCommonDataModel
  vtkIOImport
  vtkInteractionStyle
  vtkRenderingContextOpenGL2
  vtkRenderingCore
  vtkRenderingFreeType
  vtkRenderingGL2PSOpenGL2
  vtkRenderingOpenGL2
  vtkFiltersSources
  vtkRenderingVolumeOpenGL2
  vtkCommonColor
    vtkCommonCore
    vtkCommonDataModel
    vtkCommonExecutionModel
    vtkCommonTransforms
    vtkFiltersCore
    vtkFiltersGeneral
    vtkFiltersGeometry
    vtkFiltersHybrid
    vtkFiltersSources
    vtkIOImage
    vtkIOXML
    vtkImagingColor
    vtkImagingCore
    vtkImagingFourier
    vtkImagingGeneral
    vtkImagingMath
    vtkImagingMorphological
    vtkImagingSources
    vtkImagingStatistics
    vtkImagingStencil
    vtkInteractionImage
    vtkInteractionStyle
    vtkInteractionWidgets
    vtkRenderingAnnotation
    vtkRenderingCore
    vtkRenderingFreeType
    vtkRenderingImage
    vtkRenderingOpenGL2
  QUIET
)



if (NOT VTK_FOUND)
  message("Skipping OBJImporter: ${VTK_NOT_FOUND_MESSAGE}")
  return()
endif()

message (STATUS "VTK_VERSION: ${VTK_VERSION}")
if (VTK_VERSION VERSION_LESS "8.90.0")
  # old system
  include(${VTK_USE_FILE})
  
  add_executable(gravity_align_3d MACOSX_BUNDLE gravity_align_3d.cxx )
  target_link_libraries(gravity_align_3d PRIVATE ${VTK_LIBRARIES})
  
  add_executable(gridcells MACOSX_BUNDLE GridCells.cxx )
  target_link_libraries(gridcells PRIVATE ${VTK_LIBRARIES})
  
  add_executable(grid MACOSX_BUNDLE Grid.cxx )
  target_link_libraries(grid PRIVATE ${VTK_LIBRARIES})
  
else()
  # Prevent a "command line is too long" failure in Windows.
  set(CMAKE_NINJA_FORCE_RESPONSE_FILE "ON" CACHE BOOL "Force Ninja to use response files.")
  
  add_executable(gravity_align_3d MACOSX_BUNDLE gravity_align_3d.cxx )
  target_link_libraries(gravity_align_3d PRIVATE ${VTK_LIBRARIES})
  
  add_executable(gridcells MACOSX_BUNDLE GridCells.cxx )
  target_link_libraries(gridcells PRIVATE ${VTK_LIBRARIES})
    
  add_executable(grid MACOSX_BUNDLE Grid.cxx )
  target_link_libraries(grid PRIVATE ${VTK_LIBRARIES})
  
  # vtk_module_autoinit is needed
  vtk_module_autoinit(
    TARGETS gravity_align_3d
    MODULES ${VTK_LIBRARIES}
    )
    vtk_module_autoinit(
    TARGETS gridcells
    MODULES ${VTK_LIBRARIES}
    )
    vtk_module_autoinit(
    TARGETS grid
    MODULES ${VTK_LIBRARIES}
    )
endif()
