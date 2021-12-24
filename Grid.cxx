/*
See VTK examples code for source details
https://kitware.github.io/vtk-examples/site/Cxx
*/

#include <vtkActor.h>
#include <vtkGlyph3DMapper.h>
#include <vtkMath.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>
#include <vtkCubeSource.h>
#include <vtkCylinderSource.h>
#include <vtkStructuredGrid.h>
#include <vtkStructuredGridGeometryFilter.h>
#include <vtkXMLStructuredGridWriter.h>
#include <vtkStructuredGridOutlineFilter.h>


namespace {
/**
 * Convert points to glyphs.
 *
 * @param points - The points to glyph
 * @param scale - The scale, used to determine the size of the glyph
 * representing the point, expressed as a fraction of the largest side of the
 * bounding box surrounding the points. e.g. 0.05
 *
 * @return The actor.
 */
vtkSmartPointer<vtkActor> PointToGlyphOctagon(vtkPoints* points, double const& scale);
vtkSmartPointer<vtkActor> PointToGlyphHexagon(vtkPoints* points, double const& scale);
vtkSmartPointer<vtkActor> PointToGlyphSphere(vtkPoints* points, double const& scale);
vtkSmartPointer<vtkActor> PointToGlyphCube(vtkPoints* points, double const& scale);

} // namespace
int main(int, char*[])
{
  vtkNew<vtkNamedColors> colors;

  // Create a grid
  vtkNew<vtkStructuredGrid> structuredGrid;

  vtkNew<vtkPoints> points;
  unsigned int numi = 3;
  unsigned int numj = 11;
  unsigned int numk = 21;

  for (unsigned int k = 0; k < numk; k++)
  {
    for (unsigned int j = 0; j < numj; j++)
    {
      for (unsigned int i = 0; i < numi; i++)
      {
        points->InsertNextPoint(i, j, k);
      }
    }
  }

  // Specify the dimensions of the grid
  structuredGrid->SetDimensions(numi, numj, numk);
  structuredGrid->SetPoints(points);
  
  structuredGrid->BlankPoint(1);
  structuredGrid->UnBlankPoint(1);
  structuredGrid->BlankPoint(2);
  structuredGrid->UnBlankPoint(2);
  structuredGrid->BlankPoint(3);
  structuredGrid->UnBlankPoint(3);

  for (unsigned int k = 0; k < numk; k++)
  {
	structuredGrid->BlankPoint( k);
  }  
  structuredGrid->Modified();
  
  std::cout << "There are " << structuredGrid->GetNumberOfPoints() << " points."
            << std::endl; // there should be 2*3*2 = 12 points
  std::cout << "There are " << structuredGrid->GetNumberOfCells() << " cells."
            << std::endl; // The 12 points define the corners of 2 cubes/cells
                          // (4 points are shared by both cubes)

  vtkNew<vtkStructuredGridGeometryFilter> geometryFilter;
  geometryFilter->SetInputData(structuredGrid);
  geometryFilter->Update();

  // Create a mapper and actor
  vtkNew<vtkPolyDataMapper> mapper;
  mapper->SetInputConnection(geometryFilter->GetOutputPort());
  vtkNew<vtkActor> actor;
  actor->SetMapper(mapper);
  actor->GetProperty()->SetPointSize(1);

  // Visualize
  // Map the points to spheres
  auto pointGlyphActor = PointToGlyphOctagon(geometryFilter->GetOutput()->GetPoints(), 0.05);
//  auto pointGlyphActor = PointToGlyphHexagon(geometryFilter->GetOutput()->GetPoints(), 0.05);
//  auto pointGlyphActor = PointToGlyphCube(geometryFilter->GetOutput()->GetPoints(), 0.03);
//  auto pointGlyphActor = PointToGlyphSphere(geometryFilter->GetOutput()->GetPoints(), 0.03);
  pointGlyphActor->GetProperty()->SetColor(colors->GetColor3d("Gold").GetData());


  //outline box
  vtkNew<vtkStructuredGridOutlineFilter> outlineFilter;
  outlineFilter->SetInputData(structuredGrid);
  outlineFilter->Update();

  // Create a mapper and actor
  vtkNew<vtkPolyDataMapper> outlineMapper;
  outlineMapper->SetInputConnection(outlineFilter->GetOutputPort());
  vtkNew<vtkActor> outlineActor;
  outlineActor->SetMapper(outlineMapper);
  outlineActor->GetProperty()->SetColor(colors->GetColor3d("Gold").GetData());


  vtkNew<vtkRenderer> renderer;
  vtkNew<vtkRenderWindow> renderWindow;
  renderWindow->AddRenderer(renderer);
  renderWindow->SetWindowName("VisualizeStructuredGrid");
  
  vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
  renderWindowInteractor->SetRenderWindow(renderWindow);

  renderer->AddActor(actor);
  renderer->AddActor(pointGlyphActor);
  renderer->AddActor(outlineActor);

  renderer->SetBackground(colors->GetColor3d("SteelBlue").GetData());

  renderWindow->Render();
  renderWindowInteractor->Start();

  return EXIT_SUCCESS;
}


namespace {

vtkSmartPointer<vtkActor> PointToGlyphSphere(vtkPoints* points, double const& scale)
{
  auto bounds = points->GetBounds();
  double maxLen = 0;
  for (int i = 1; i < 3; ++i)
  {
    maxLen = std::max(bounds[i + 1] - bounds[i], maxLen);
  }

  vtkNew<vtkSphereSource> sphereSource;
  sphereSource->SetRadius(scale * maxLen);

  vtkNew<vtkPolyData> pd;
  pd->SetPoints(points);

  vtkNew<vtkGlyph3DMapper> mapper;
  mapper->SetInputData(pd);
  mapper->SetSourceConnection(sphereSource->GetOutputPort());
  mapper->ScalarVisibilityOff();
  mapper->ScalingOff();

  vtkNew<vtkActor> actor;
  actor->SetMapper(mapper);

  return actor;
}


vtkSmartPointer<vtkActor> PointToGlyphCube(vtkPoints* points, double const& scale)
{
  auto bounds = points->GetBounds();
  double maxLen = 0;
  for (int i = 1; i < 3; ++i)
  {
    maxLen = std::max(bounds[i + 1] - bounds[i], maxLen);
  }

  vtkNew<vtkCubeSource> cubeSource;
  double wallSz=(scale * maxLen)/2.0;
 //cubeSource->SetBounds (double xMin, double xMax, double yMin, double yMax, double zMin, double zMax)
 cubeSource->SetBounds(-wallSz, wallSz, -wallSz, wallSz,-wallSz, wallSz);
  
  vtkNew<vtkPolyData> pd;
  pd->SetPoints(points);

  vtkNew<vtkGlyph3DMapper> mapper;
  mapper->SetInputData(pd);
  mapper->SetSourceConnection(cubeSource->GetOutputPort());
  mapper->ScalarVisibilityOff();
  mapper->ScalingOff();

  vtkNew<vtkActor> actor;
  actor->SetMapper(mapper);
   //actor->GetProperty()->SetColor(colors->GetColor3d("Banana").GetData())
  
  return actor;
}


vtkSmartPointer<vtkActor> PointToGlyphHexagon(vtkPoints* points, double const& scale)
{
  auto bounds = points->GetBounds();
  double maxLen = 0;
  for (int i = 1; i < 3; ++i)
  {
    maxLen = std::max(bounds[i + 1] - bounds[i], maxLen);
  }

  vtkNew<vtkCylinderSource> hexSource;
  double wallSz=(scale * maxLen);
 hexSource->SetHeight(wallSz);
 hexSource->SetRadius(wallSz);
 hexSource->SetResolution(6);
  
  vtkNew<vtkPolyData> pd;
  pd->SetPoints(points);

  vtkNew<vtkGlyph3DMapper> mapper;
  mapper->SetInputData(pd);
  mapper->SetSourceConnection(hexSource->GetOutputPort());
  mapper->ScalarVisibilityOff();
  mapper->ScalingOff();

  vtkNew<vtkActor> actor;
  actor->SetMapper(mapper);
  //todo: find proper way to tile the plane
 // actor->RotateX(90.0);
 // actor->RotateY(-45.0);
   //actor->GetProperty()->SetColor(colors->GetColor3d("Banana").GetData())
  
  return actor;
}


vtkSmartPointer<vtkActor> PointToGlyphOctagon(vtkPoints* points, double const& scale)
{
  auto bounds = points->GetBounds();
  double maxLen = 0;
  for (int i = 1; i < 3; ++i)
  {
    maxLen = std::max(bounds[i + 1] - bounds[i], maxLen);
  }

  vtkNew<vtkCylinderSource> octaSource;
  double wallSz=(scale * maxLen);
 octaSource->SetHeight(wallSz);
 octaSource->SetRadius(wallSz);
 octaSource->SetResolution(8);
  
  vtkNew<vtkPolyData> pd;
  pd->SetPoints(points);

  vtkNew<vtkGlyph3DMapper> mapper;
  mapper->SetInputData(pd);
  mapper->SetSourceConnection(octaSource->GetOutputPort());
  mapper->ScalarVisibilityOff();
  mapper->ScalingOff();

  vtkNew<vtkActor> actor;
  actor->SetMapper(mapper);
  //todo: find proper way to tile the plane
  //actor->RotateX(90.0);
  //actor->RotateZ(-45.0);
  
   //actor->GetProperty()->SetColor(colors->GetColor3d("Banana").GetData())
  
  return actor;
}



} // namespace
