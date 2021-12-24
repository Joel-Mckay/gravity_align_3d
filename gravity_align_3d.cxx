/*
* This bug test example is provided under the Apache 2.0 licence
*
* (c) 2021 Joel Mckay 
*
* Except as represented in this agreement,
* all work product by Developer is provided "AS IS". 
* Other than as provided in this agreement, 
* Developer makes no other warranties, express or implied, 
* and hereby disclaims all implied warranties, including any 
* warranty of merchantability and warranty of fitness for a 
* particular purpose.
*/


#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkOBJImporter.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkTexture.h>

#include <vtkActor.h>
#include <vtkCutter.h>
#include <vtkMath.h>
#include <vtkPlane.h>
#include <vtkProperty.h>
#include <vtkSmartPointer.h>
#include <iostream>
#include <string>
#include <sstream>
 
#include <vtkDoubleArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkRectilinearGridGeometryFilter.h>


#define DEBUG_MODE 1
//#define SHOW_OBJECT 1
#define SHOW_BOUNDING_BOX 1

/* show only z-axis slices */
#define SHOW_OBJECT_CONTOURS 1
/* show only xyz-axis slices */
//#define SHOW_OBJECT_CONTOURS 3

/* try to fix object ?  Bug3: causes seg fault with meshlab fixed cat mesh */
//#define CLEAN_OBJ_POLY_DATA 1

/* scaling
* Note: default scale in meters,
* This will auto-scale from meters-to-cm and cm-to-mm if contour lines seem too sparse
*/
#define OBJECT_CONTOUR_LAYER_Z_SPACING 1.0
#define OBJECT_MAX_CONTOUR_LAYER_Z_COUNT 1000.0


/* select scan mode: */
/* scan object max size */
#define SCAN_MODE 0
/* scan object Z-height of bounding box */
//Bug: Using the bounding cube Z-height causes loss of the model nose on test4.sh
//#define SCAN_MODE 1


#if defined(CLEAN_OBJ_POLY_DATA)
	#include <vtkTriangleFilter.h>
	#include <vtkCleanPolyData.h>
#endif

#include <vtkCamera.h>
#include <vtkCubeSource.h>
#define 	BOUNDS_XMIN 0
#define 	BOUNDS_XMAX 1
#define 	BOUNDS_YMIN 2
#define 	BOUNDS_YMAX 3
#define 	BOUNDS_ZMIN 4
#define 	BOUNDS_ZMAX 5

#define 	COORDS_X 0
#define 	COORDS_Y 1
#define 	COORDS_Z 2

#define GRID_X_AXIS_MARKS 101
#define GRID_Y_AXIS_MARKS 101
#define GRID_Z_AXIS_MARKS 101
  
  
int main(int argc, char* argv[])
{
  
  double zLayerSliceThickness = OBJECT_CONTOUR_LAYER_Z_SPACING;
  char flagTextureOK=0;

  int ii;
  int xx;
  int yy;
  double tmpIncVal;
  static double gridX[GRID_X_AXIS_MARKS];
  static double gridY[GRID_Y_AXIS_MARKS];
  static double gridZ[GRID_Z_AXIS_MARKS];

  if (argc < 4)
  {
    std::cout << "Usage: " << argv[0] << " objfile mtlfile texturepath" << std::endl
			<< "e.g. "<< argv[0] <<" doorman.obj doorman.mtl doorman" << std::endl;
    return EXIT_FAILURE;
  }
  
 vtkNew<vtkOBJImporter> importer;
 vtkNew<vtkNamedColors> colors;
  
  #if defined(DEBUG_MODE)
	std::cout  << std::endl << "importer->SetFileName" << std::endl;
  #endif
  importer->SetFileName(argv[1]);

  //note: if Name.txt in same path as Name.obj a cutter error causes it to loop
  
  #if defined(DEBUG_MODE)
	std::cout  << std::endl << "importer->SetTexturePath" << std::endl;
  #endif
  importer->SetTexturePath(argv[3]);
  
  #if defined(DEBUG_MODE)
	std::cout  << std::endl << "importer->SetFileNameMTL" << std::endl;
  #endif
  importer->SetFileNameMTL(argv[2]);



  
  
  /*
  //https://kitware.github.io/vtk-examples/site/Cxx/PolyData/BooleanOperationPolyDataFilter/
      auto poly1 = ReadPolyData(argv[1]);
    vtkNew<vtkTriangleFilter> tri1;
    tri1->SetInputData(poly1);
    vtkNew<vtkCleanPolyData> clean1;
    clean1->SetInputConnection(tri1->GetOutputPort());
    clean1->Update();
    input1 = clean1->GetOutput();

    auto poly2 = ReadPolyData(argv[3]);
    vtkNew<vtkTriangleFilter> tri2;
    tri2->SetInputData(poly2);
    tri2->Update();
    vtkNew<vtkCleanPolyData> clean2;
    clean2->SetInputConnection(tri2->GetOutputPort());
    clean2->Update();
    input2 = clean2->GetOutput();
    operation = argv[2];
  */
  
  vtkNew<vtkRenderer> renderer;
  vtkNew<vtkRenderWindow> renWin;
  vtkNew<vtkRenderWindowInteractor> iren;
  
  //todo: find better way to keep model and contour layers separated
  #if !defined(SHOW_OBJECT)
	vtkNew<vtkRenderer> rendererBuffer;
	vtkNew<vtkRenderWindow> renWinBuffer;
	rendererBuffer->SetBackground(colors->GetColor3d("Black").GetData());
	rendererBuffer->GradientBackgroundOff();
	rendererBuffer->UseHiddenLineRemovalOn();
	renWinBuffer->AddRenderer(rendererBuffer);
	renWinBuffer->SetSize(640, 480);
	renWinBuffer->SetWindowName("Hidden Buffer");
  #endif
  
  renderer->SetBackground2(colors->GetColor3d("Blue").GetData());
  renderer->SetBackground(colors->GetColor3d("Green").GetData());
  renderer->GradientBackgroundOn();
  renWin->AddRenderer(renderer);
  renderer->UseHiddenLineRemovalOn();
  renWin->AddRenderer(renderer);
  renWin->SetSize(640, 480);
  renWin->SetWindowName("Scanning file....");

  iren->SetRenderWindow(renWin);
  
  #if defined(DEBUG_MODE)
	std::cout  << std::endl << "renderer->GetActors" << std::endl;
  #endif
  
  #if defined(SHOW_OBJECT)
	importer->SetRenderWindow(renWin);
	importer->Update();	//crashes on larger files with manifold errors
	auto actors = renderer->GetActors();
  #else
	importer->SetRenderWindow(renWinBuffer);
	importer->Update();	//crashes on larger files with manifold errors
	auto actors = rendererBuffer->GetActors();
  #endif
  
  actors->InitTraversal();
  #if defined(DEBUG_MODE)
	std::cout << "There are " << actors->GetNumberOfItems() << " actors" << std::endl;
  #endif
  
  //scan scene objects
 // for (vtkIdType a = 0; (a < actors->GetNumberOfItems()) && (flagTextureOK == 0); ++a)  //stop after first textured actor found
  for (vtkIdType a = 0; (a < actors->GetNumberOfItems()); ++a)  //stop after first textured actor found
  {
	std::cout << importer->GetOutputDescription(a) << std::endl;

	vtkActor* actor = actors->GetNextActor();

	//Note OBJImporter turns texture interpolation off
	if (actor->GetTexture())
	{
		#if defined(DEBUG_MODE)
			std::cout << "Has texture\n";
		#endif
		actor->GetTexture()->InterpolateOn();
		flagTextureOK=1;
	}else{
		flagTextureOK=0;
	}

	
	#if defined(CLEAN_OBJ_POLY_DATA)
		// merge duplicate points, and/or remove unused points and/or remove degenerate cells 
		vtkNew<vtkTriangleFilter> inputPolyDataTri;
		inputPolyDataTri->SetInputData(dynamic_cast<vtkPolyData*>(actor->GetMapper()->GetInput()));
		vtkNew<vtkCleanPolyData> inputPolyDataCleaner;
		inputPolyDataCleaner->SetInputConnection(inputPolyDataTri->GetOutputPort());
		inputPolyDataCleaner->Update();
		vtkPolyData* inputPolyData = dynamic_cast<vtkPolyData*>(inputPolyDataCleaner->GetOutput());
	#else
		vtkPolyData* inputPolyData = dynamic_cast<vtkPolyData*>(actor->GetMapper()->GetInput());
	#endif
	inputPolyData->ComputeBounds();
	

	vtkPolyDataMapper* inputMapper = dynamic_cast<vtkPolyDataMapper*>(actor->GetMapper());
	inputMapper->SetInputData(inputPolyData);

	// Create a plane to cut
	vtkNew<vtkPlane> plane;
	plane->SetOrigin(inputPolyData->GetCenter());
	plane->SetNormal(0, 0, 1);	//angle of the slice from XY plane
	
	#if SHOW_OBJECT_CONTOURS == 3
		vtkNew<vtkPlane> planeWidth;
		planeWidth->SetOrigin(inputPolyData->GetCenter());
		planeWidth->SetNormal(1, 0, 0);	//angle of the slice from ZX plane
		
		vtkNew<vtkPlane> planeThickness;
		planeThickness->SetOrigin(inputPolyData->GetCenter());
		planeThickness->SetNormal(0, 1, 0);	//angle of the slice from ZY plane
	#endif
	
	//double* vtkDataSet::GetBounds() 	
	//Returns a pointer to the geometry bounding box in the form (xmin,xmax, ymin,ymax, zmin,zmax). 
	double allBounds[6];
	allBounds[BOUNDS_XMIN] = inputPolyData->GetBounds()[BOUNDS_XMIN];
	allBounds[BOUNDS_XMAX] = inputPolyData->GetBounds()[BOUNDS_XMAX];
	allBounds[BOUNDS_YMIN] = inputPolyData->GetBounds()[BOUNDS_YMIN];
	allBounds[BOUNDS_YMAX] = inputPolyData->GetBounds()[BOUNDS_YMAX];
	allBounds[BOUNDS_ZMIN] = inputPolyData->GetBounds()[BOUNDS_ZMIN];
	allBounds[BOUNDS_ZMAX] = inputPolyData->GetBounds()[BOUNDS_ZMAX];
	
	double minBound[3]; //assume - z-axis
	minBound[COORDS_X] =allBounds[BOUNDS_XMIN];
	minBound[COORDS_Y] = allBounds[BOUNDS_YMIN];
	minBound[COORDS_Z] = allBounds[BOUNDS_ZMIN];

	double maxBound[3]; //assume z-axis
	maxBound[COORDS_X] = allBounds[BOUNDS_XMAX];
	maxBound[COORDS_Y] = allBounds[BOUNDS_YMAX];
	maxBound[COORDS_Z] = allBounds[BOUNDS_ZMAX];
 
	double center[3];
	center[COORDS_X] = inputPolyData->GetCenter()[COORDS_X];
	center[COORDS_Y] = inputPolyData->GetCenter()[COORDS_Y];
	center[COORDS_Z] = inputPolyData->GetCenter()[COORDS_Z];

	double minWidth[3]; 
	minWidth[COORDS_X] =allBounds[BOUNDS_XMIN];
	minWidth[COORDS_Y] = center[COORDS_X];
	minWidth[COORDS_Z] = center[COORDS_Z];

	double maxWidth[3];
	maxWidth[COORDS_X] = allBounds[BOUNDS_XMAX];
	maxWidth[COORDS_Y] = center[COORDS_X];
	maxWidth[COORDS_Z] = center[COORDS_Z];
		
	double minThickness[3]; 
	minThickness[COORDS_X] =center[COORDS_Y];
	minThickness[COORDS_Y] = allBounds[BOUNDS_YMIN];
	minThickness[COORDS_Z] = center[COORDS_Z];

	double maxThickness[3];
	maxThickness[COORDS_X] = center[COORDS_Y];
	maxThickness[COORDS_Y] = allBounds[BOUNDS_YMAX];
	maxThickness[COORDS_Z] = center[COORDS_Z];
	
	double widthMin = sqrt(vtkMath::Distance2BetweenPoints(minWidth, center));   
	double widthMax = sqrt(vtkMath::Distance2BetweenPoints(maxWidth, center));
	double thicknessMin = sqrt(vtkMath::Distance2BetweenPoints(minThickness, center));  
	double thicknessMax = sqrt(vtkMath::Distance2BetweenPoints(maxThickness, center));
	double distanceMin = sqrt(vtkMath::Distance2BetweenPoints(minBound, center));   //better size estimate
	double distanceMax = sqrt(vtkMath::Distance2BetweenPoints(maxBound, center));
	
	double zLayerScaleFactor=1.0; //convert meter scale factor to meters default
	
	
	//check object model is of sane design
	if( (std::isinf(widthMin) == true) || (std::isinf(widthMax) == true) ||
		(std::isinf(thicknessMin) == true) || (std::isinf(thicknessMax) == true) ||
			(std::isinf(distanceMin) == true) || (std::isinf(distanceMax) == true) || (flagTextureOK==0))
	{	
		std::cout  << std::endl << "Skipped malformed 3D actor" << std::endl;
		flagTextureOK=0;	//force scan to continue
	}else{
	
	#if defined(SHOW_BOUNDING_BOX)
		vtkNew<vtkCubeSource> cubee;
		cubee->SetBounds(allBounds);
		cubee->SetCenter(center);
		vtkNew<vtkPolyDataMapper> cubeMapper;
		cubeMapper->SetInputConnection(cubee->GetOutputPort());
	#endif
	
	#if defined(SHOW_OBJECT_CONTOURS)
		// Create cutter
		vtkNew<vtkCutter> cutter;
		cutter->SetCutFunction(plane);	
		//cutter->SetInputConnection(inputPolyData->GetOutputPort());
		//cutter->Update();
		cutter->SetInputData(inputPolyData);
		//cutter->GenerateTrianglesOn();
		//cutter->GenerateTrianglesOff();

		
		#if SHOW_OBJECT_CONTOURS == 3
			vtkNew<vtkCutter> cutterWidth;
			cutterWidth->SetCutFunction(planeWidth);	
			cutterWidth->SetInputData(inputPolyData);

			vtkNew<vtkCutter> cutterThickness;
			cutterThickness->SetCutFunction(planeThickness);	
			cutterThickness->SetInputData(inputPolyData);
		#endif
		
		//make oversized by one layer if lattice violation, as milling to size is easier than adding material
		zLayerSliceThickness = OBJECT_CONTOUR_LAYER_Z_SPACING;	//default to normal mm scale
		#if SCAN_MODE == 0
			int numOfSlices = vtkMath::Ceil((distanceMin + distanceMax)/zLayerSliceThickness);
			int numOfWidthSlices = vtkMath::Ceil((widthMin + widthMax)/zLayerSliceThickness);
			int numOfThicknessSlices = vtkMath::Ceil((thicknessMin + thicknessMax)/zLayerSliceThickness);
		#elif SCAN_MODE == 1
			int numOfSlices = vtkMath::Ceil( ( std::fabs(allBounds[BOUNDS_ZMIN])  + std::fabs(allBounds[BOUNDS_ZMAX]) )/zLayerSliceThickness);
			int numOfWidthSlices = vtkMath::Ceil( ( std::fabs(allBounds[BOUNDS_XMIN])  + std::fabs(allBounds[BOUNDS_XMAX]) )/zLayerSliceThickness);
			int numOfThicknessSlices = vtkMath::Ceil( ( std::fabs(allBounds[BOUNDS_YMIN])  + std::fabs(allBounds[BOUNDS_YMAX]) )/zLayerSliceThickness);
		#endif
		
		//contours too few?
		if((numOfSlices < 99) || (numOfWidthSlices < 99) || (numOfThicknessSlices < 99))
		{
			zLayerSliceThickness = OBJECT_CONTOUR_LAYER_Z_SPACING/10.0; //convert meter scale to dm
			zLayerScaleFactor=10.0; //convert meter scale to dm
			
			#if SCAN_MODE == 0
				numOfSlices = vtkMath::Ceil((distanceMin + distanceMax)/zLayerSliceThickness);
				numOfWidthSlices = vtkMath::Ceil((widthMin + widthMax)/zLayerSliceThickness);
				numOfThicknessSlices = vtkMath::Ceil((thicknessMin + thicknessMax)/zLayerSliceThickness);
			#elif SCAN_MODE == 1
				numOfSlices = vtkMath::Ceil( ( std::fabs(allBounds[BOUNDS_ZMIN])  + std::fabs(allBounds[BOUNDS_ZMAX]) )/zLayerSliceThickness);		
				numOfWidthSlices = vtkMath::Ceil( ( std::fabs(allBounds[BOUNDS_XMIN])  + std::fabs(allBounds[BOUNDS_XMAX]) )/zLayerSliceThickness);
				numOfThicknessSlices = vtkMath::Ceil( ( std::fabs(allBounds[BOUNDS_YMIN])  + std::fabs(allBounds[BOUNDS_YMAX]) )/zLayerSliceThickness);
			#endif
			
			#if defined(DEBUG_MODE)
				std::cout << "\nWarning: scaled up... now in decimeter \n";
			#endif
		}
		
		//contours too few?
		if((numOfSlices < 99) || (numOfWidthSlices < 99) || (numOfThicknessSlices < 99))
		{
			zLayerSliceThickness = OBJECT_CONTOUR_LAYER_Z_SPACING/100.0; //convert meter scale to cm
			zLayerScaleFactor=100.0; //convert meter scale to dm
			
			#if SCAN_MODE == 0
				numOfSlices = vtkMath::Ceil((distanceMin + distanceMax)/zLayerSliceThickness);	
				numOfWidthSlices = vtkMath::Ceil((widthMin + widthMax)/zLayerSliceThickness);
				numOfThicknessSlices = vtkMath::Ceil((thicknessMin + thicknessMax)/zLayerSliceThickness);
			#elif SCAN_MODE == 1
				numOfSlices = vtkMath::Ceil( ( std::fabs(allBounds[BOUNDS_ZMIN])  + std::fabs(allBounds[BOUNDS_ZMAX]) )/zLayerSliceThickness);		
				numOfWidthSlices = vtkMath::Ceil( ( std::fabs(allBounds[BOUNDS_XMIN])  + std::fabs(allBounds[BOUNDS_XMAX]) )/zLayerSliceThickness);
				numOfThicknessSlices = vtkMath::Ceil( ( std::fabs(allBounds[BOUNDS_YMIN])  + std::fabs(allBounds[BOUNDS_YMAX]) )/zLayerSliceThickness);
			#endif
			
			#if defined(DEBUG_MODE)
				std::cout << "\nWarning: scaled up... now in centimeter \n";
			#endif
		}
		
		//contours still too few?
		if((numOfSlices < 99) || (numOfWidthSlices < 99) || (numOfThicknessSlices < 99))
		{
			zLayerSliceThickness = OBJECT_CONTOUR_LAYER_Z_SPACING/1000.0; //convert meter scale to mm
			zLayerScaleFactor=1000.0; //convert meter scale to dm
			
			#if SCAN_MODE == 0
				numOfSlices = vtkMath::Ceil((distanceMin + distanceMax)/zLayerSliceThickness);
				numOfWidthSlices = vtkMath::Ceil((widthMin + widthMax)/zLayerSliceThickness);
				numOfThicknessSlices = vtkMath::Ceil((thicknessMin + thicknessMax)/zLayerSliceThickness);
			#elif SCAN_MODE == 1
				numOfSlices = vtkMath::Ceil( ( std::fabs(allBounds[BOUNDS_ZMIN])  + std::fabs(allBounds[BOUNDS_ZMAX]) )/zLayerSliceThickness);		
				numOfWidthSlices = vtkMath::Ceil( ( std::fabs(allBounds[BOUNDS_XMIN])  + std::fabs(allBounds[BOUNDS_XMAX]) )/zLayerSliceThickness);
				numOfThicknessSlices = vtkMath::Ceil( ( std::fabs(allBounds[BOUNDS_YMIN])  + std::fabs(allBounds[BOUNDS_YMAX]) )/zLayerSliceThickness);
			#endif
			
			#if defined(DEBUG_MODE)
				std::cout << "\nWarning: scaled up... now in millimeter \n";
			#endif
		}
		
		//contours still too Many?
		if(numOfSlices >= OBJECT_MAX_CONTOUR_LAYER_Z_COUNT)
		{
			zLayerSliceThickness = OBJECT_CONTOUR_LAYER_Z_SPACING/OBJECT_MAX_CONTOUR_LAYER_Z_COUNT; //convert unknown scale to fit
			numOfWidthSlices = vtkMath::Floor(numOfWidthSlices*(OBJECT_MAX_CONTOUR_LAYER_Z_COUNT/numOfSlices));
			numOfThicknessSlices = vtkMath::Floor(numOfThicknessSlices*(OBJECT_MAX_CONTOUR_LAYER_Z_COUNT/numOfSlices));
			numOfSlices = vtkMath::Floor(OBJECT_MAX_CONTOUR_LAYER_Z_COUNT);
			
			zLayerScaleFactor=OBJECT_MAX_CONTOUR_LAYER_Z_COUNT; //convert meter scale to fit build volume
			
			#if defined(DEBUG_MODE)
				std::cout << "\nWarning: scaled down to fit maximum area... \n";
			#endif
		}
		
		//still bad slice?
		if((numOfSlices < 4)||(numOfSlices < 4)||(numOfSlices < 4))
			{
			numOfSlices=3;
			numOfWidthSlices=3;
			numOfThicknessSlices=3;
			zLayerScaleFactor=OBJECT_MAX_CONTOUR_LAYER_Z_COUNT; //convert meter scale to unknown ???
				
			#if defined(DEBUG_MODE)
					std::cout << "\nWarning model Z cutter defaults to  " << numOfSlices << " slice(s)\n";
			#endif
		}
		
		
		#if defined(DEBUG_MODE)
			#if SCAN_MODE == 0
				std::cout << "\nFrom distanceX=["<< -widthMin << ", " << widthMax << "] has about " << numOfWidthSlices << " slices\n";
				std::cout << "\nFrom distanceY=["<< -thicknessMin << ", " << thicknessMax << "] has about " << numOfThicknessSlices << " slices\n";
				std::cout << "\nFrom distanceZ=["<< -distanceMin << ", " << distanceMax << "] has about " << numOfSlices << " slices\n";
			#elif SCAN_MODE == 1
				std::cout << "\nFrom X=["<< allBounds[BOUNDS_XMIN] << ", " <<  allBounds[BOUNDS_XMAX] << "] has about " << numOfWidthSlices << " slices\n";
				std::cout << "\nFrom Y=["<< allBounds[BOUNDS_YMIN] << ", " <<  allBounds[BOUNDS_YMAX] << "] has about " << numOfThicknessSlices << " slices\n";
				std::cout << "\nFrom Z=["<< allBounds[BOUNDS_ZMIN] << ", " <<  allBounds[BOUNDS_ZMAX] << "] has about " << numOfSlices << " slices\n";
			#endif
		#endif
		
		
		//set line thickness to render if really big object
		double zSpacingOfSlicesPx = (zLayerSliceThickness/2.0)*10.0; 
		if(zSpacingOfSlicesPx < 10.0){
			zSpacingOfSlicesPx=2.0;
		}
		int zSpacingPx = vtkMath::Floor(zSpacingOfSlicesPx); 
		
		#if SCAN_MODE == 0
			#if SHOW_OBJECT_CONTOURS == 3
				cutterWidth->GenerateValues(numOfWidthSlices, -widthMin, widthMax);
				cutterThickness->GenerateValues(numOfThicknessSlices, -thicknessMin, thicknessMax);
			#endif
			cutter->GenerateValues(numOfSlices, -distanceMin, distanceMax);
		#elif SCAN_MODE == 1
			//Bug: Using the bounding cube Z-height causes loss of the model nose on test4.sh
			#if SHOW_OBJECT_CONTOURS == 3
				cutterWidth->GenerateValues(numOfWidthSlices,  allBounds[BOUNDS_XMIN], allBounds[BOUNDS_XMAX]);
				cutterThickness->GenerateValues(numOfThicknessSlices,  allBounds[BOUNDS_YMIN], allBounds[BOUNDS_YMAX]);
			#endif
			cutter->GenerateValues(numOfSlices,  allBounds[BOUNDS_ZMIN], allBounds[BOUNDS_ZMAX]);
		#endif
		
		#if SHOW_OBJECT_CONTOURS == 3
			vtkNew<vtkPolyDataMapper> cutterMapperWidth;
			cutterMapperWidth->SetInputConnection(cutterWidth->GetOutputPort());
			vtkNew<vtkPolyDataMapper> cutterMapperThickness;
			cutterMapperThickness->SetInputConnection(cutterThickness->GetOutputPort());
		#endif
		vtkNew<vtkPolyDataMapper> cutterMapper;
		cutterMapper->SetInputConnection(cutter->GetOutputPort());
		
		#if defined(DEBUG_MODE)
			#if SHOW_OBJECT_CONTOURS == 3
				std::cout  << std::endl << "cutterMapperWidth->GetNumberOfPieces has " << cutterMapperWidth->GetNumberOfPieces() << std::endl;
				std::cout   << "cutterWidth->GetNumberOfContours has " << cutterWidth->GetNumberOfContours()  << std::endl;
				std::cout  << "cutterMapperThickness->GetNumberOfPieces has " << cutterMapperThickness->GetNumberOfPieces() << std::endl;
				std::cout   << "cutterThickness->GetNumberOfContours has " << cutterThickness->GetNumberOfContours()  << std::endl;
			#endif
			std::cout  << "cutterMapper->GetNumberOfPieces has " << cutterMapper->GetNumberOfPieces() << std::endl;
			std::cout  << "cutter->GetNumberOfContours has " << cutter->GetNumberOfContours()  << std::endl;
		#endif
		
		#if SHOW_OBJECT_CONTOURS == 3
			cutterMapperWidth->ScalarVisibilityOff();
			cutterMapperThickness->ScalarVisibilityOff();
		#endif
		cutterMapper->ScalarVisibilityOff();	//Turn off flag to control whether scalar data is used to color objects. 
									//todo: BUG 1 for cat test object, the blue bum bug arises if off
		
		#if SHOW_OBJECT_CONTOURS == 3
			cutterMapperWidth->SetResolveCoincidentTopologyToPolygonOffset(); //todo: performance drops a bit
			cutterMapperThickness->SetResolveCoincidentTopologyToPolygonOffset(); //todo: performance drops a bit
		#endif
		cutterMapper->SetResolveCoincidentTopologyToPolygonOffset(); //todo: performance drops a bit
	#endif
	

	#if defined(SHOW_OBJECT_CONTOURS)
		// Create plane actors
		#if SHOW_OBJECT_CONTOURS == 3
			vtkNew<vtkActor> planeActorWidth;
			planeActorWidth->GetProperty()->SetColor(colors->GetColor3d("Yellow").GetData());
			planeActorWidth->GetProperty()->SetLineWidth(zSpacingPx);
			planeActorWidth->SetMapper(cutterMapperWidth);
			renderer->AddActor(planeActorWidth); 
			
			vtkNew<vtkActor> planeActorThickness;
			planeActorThickness->GetProperty()->SetColor(colors->GetColor3d("Red").GetData());
			planeActorThickness->GetProperty()->SetLineWidth(zSpacingPx);
			planeActorThickness->SetMapper(cutterMapperThickness);
			renderer->AddActor(planeActorThickness);
		#endif
		
		vtkNew<vtkActor> planeActor;
		planeActor->GetProperty()->SetColor(colors->GetColor3d("Deep_pink").GetData());
		planeActor->GetProperty()->SetLineWidth(zSpacingPx);
		planeActor->SetMapper(cutterMapper);
		renderer->AddActor(planeActor); // display the contours resulting from the cut
	#endif
	
	#if defined(SHOW_BOUNDING_BOX)
		 // Create cube actor
		vtkNew<vtkActor> cubeActor;
		cubeActor->GetProperty()->SetColor( colors->GetColor3d("Aquamarine").GetData());
		cubeActor->GetProperty()->SetOpacity(0.1);
		cubeActor->SetMapper(cubeMapper);
		renderer->AddActor(cubeActor); // display 
	#endif


//////////////////////////////////////////////////////////////////////////
//add grid plane
  //Create a rectilinear grid by defining three arrays specifying the coordinates in the x-y-z directions.
  vtkNew<vtkDoubleArray> gridXCoords;
  vtkNew<vtkDoubleArray> gridYCoords;
  vtkNew<vtkDoubleArray> gridZCoords;

	std::cout << "zLayerScaleFactor " << zLayerScaleFactor << "\n";
	std::cout << "From X=["<< allBounds[BOUNDS_XMIN] << ", " <<  allBounds[BOUNDS_XMAX] << "] has about " << numOfWidthSlices << " slices\n";
	std::cout << "From Y=["<< allBounds[BOUNDS_YMIN] << ", " <<  allBounds[BOUNDS_YMAX] << "] has about " << numOfThicknessSlices << " slices\n";
	std::cout << "From Z=["<< allBounds[BOUNDS_ZMIN] << ", " <<  allBounds[BOUNDS_ZMAX] << "] has about " << numOfSlices << " slices\n";

//select grid mesg size
if(numOfWidthSlices >= numOfThicknessSlices)
{
	tmpIncVal=(-allBounds[BOUNDS_XMIN]+allBounds[BOUNDS_XMAX])/((numOfWidthSlices/zLayerScaleFactor));
	
}else{
	tmpIncVal=(-allBounds[BOUNDS_YMIN]+allBounds[BOUNDS_YMAX])/((numOfThicknessSlices/zLayerScaleFactor));
	
}

  gridX[0] = allBounds[BOUNDS_XMIN];
  for (xx = 1; (xx < GRID_X_AXIS_MARKS) && (gridX[xx-1] <= vtkMath::Ceil(allBounds[BOUNDS_XMAX])); xx++)
  {
	gridX[xx]=gridX[xx-1]+tmpIncVal;
	  
	//gridX[ii]=gridX[ii-1]+((-allBounds[BOUNDS_XMIN]+allBounds[BOUNDS_XMAX])/(GRID_X_AXIS_MARKS));
	//gridX[ii]=gridX[ii-1]+(((-allBounds[BOUNDS_XMIN]+allBounds[BOUNDS_XMAX])/(numOfWidthSlices))*(numOfThicknessSlices/GRID_Y_AXIS_MARKS));
  }
  for (ii = 0; (ii < GRID_X_AXIS_MARKS) && (ii < xx); ii++)
  {
    gridXCoords->InsertNextValue(gridX[ii]);
  }
  
  
  gridY[0] = allBounds[BOUNDS_YMIN] ;
  for (yy = 1; (yy < GRID_Y_AXIS_MARKS) && (gridY[yy-1] <= vtkMath::Ceil(allBounds[BOUNDS_YMAX])); yy++)
  {
	gridY[yy]=gridY[yy-1]+tmpIncVal;
	
	//gridY[ii]=gridY[ii-1]+((-allBounds[BOUNDS_YMIN]+allBounds[BOUNDS_YMAX])/(GRID_Y_AXIS_MARKS));
	//gridY[ii]=gridY[ii-1]+(((-allBounds[BOUNDS_YMIN]+allBounds[BOUNDS_YMAX])/(numOfThicknessSlices))*(numOfThicknessSlices/GRID_Y_AXIS_MARKS));
  }
  for (ii = 0; (ii < GRID_Y_AXIS_MARKS) && (ii < yy); ii++)
  {
    gridYCoords->InsertNextValue(gridY[ii]);
  }
  
  //we don't  need the full 3D ccords
 // gridZ[0] = allBounds[BOUNDS_ZMIN]*zLayerThicknessScale ;
 // for (ii = 1; ii < GRID_Z_AXIS_MARKS; ii++)
 // {
 //	gridZ[ii]=gridZ[ii-1]+1.0/((-allBounds[BOUNDS_ZMIN]+allBounds[BOUNDS_ZMAX] )/(GRID_Z_AXIS_MARKS));
 // }
  for (ii = 0; ii < GRID_Z_AXIS_MARKS; ii++)
  {
    //gridZCoords->InsertNextValue(gridZ[ii]);
    gridZCoords->InsertNextValue(allBounds[BOUNDS_ZMIN]);
  }

  
  // The coordinates are assigned to the rectilinear grid. Make sure that
  // the number of values in each of the XCoordinates, YCoordinates,
  // and ZCoordinates is equal to what is defined in SetDimensions().
  //
  vtkNew<vtkRectilinearGrid> rgrid;
 // rgrid->SetDimensions(GRID_X_AXIS_MARKS, GRID_Y_AXIS_MARKS, GRID_Z_AXIS_MARKS);
  rgrid->SetDimensions(xx, yy, GRID_Z_AXIS_MARKS);
  rgrid->SetXCoordinates(gridXCoords);
  rgrid->SetYCoordinates(gridYCoords);
  rgrid->SetZCoordinates(gridZCoords);

  // Extract a plane from the grid to see what we've got.
  vtkNew<vtkRectilinearGridGeometryFilter> rgridPlane;
  // int xMin, int xMax, int yMin, int yMax, int zMin, int zMax 
  //rgridPlane->SetExtent(0, GRID_X_AXIS_MARKS-1, 0, GRID_Y_AXIS_MARKS-1, 0, GRID_Z_AXIS_MARKS-1 ); //(imin,imax, jmin,jmax, kmin,kmax) indices
  //rgridPlane->SetExtent(0, GRID_X_AXIS_MARKS-1, 0, GRID_Y_AXIS_MARKS-1, 0, 0 ); //(imin,imax, jmin,jmax, kmin,kmax) indices
  rgridPlane->SetExtent(0, xx-1, 0, yy-1, 0, 0 ); //(imin,imax, jmin,jmax, kmin,kmax) indices
  rgridPlane->SetInputData(rgrid);
  
  #if defined(DEBUG_MODE)
	std::cout << "gridX["<< gridX[0]<<", "<< gridX[xx-1] <<"], gridY["<< gridY[0] <<", "<< gridY[yy-1] <<"], gridZ["<< gridZ[0] <<", "<< gridZ[GRID_Z_AXIS_MARKS-1] <<"]" << std::endl;
  #endif
  
  //creat grid plane
  vtkNew<vtkPolyDataMapper> rgridMapper;
  rgridMapper->SetInputConnection(rgridPlane->GetOutputPort());

  vtkNew<vtkActor> rgridWireActor;
  rgridWireActor->SetMapper(rgridMapper);
  rgridWireActor->GetProperty()->SetColor(colors->GetColor3d("Banana").GetData());
  rgridWireActor->GetProperty()->EdgeVisibilityOn();
	
	renderer->AddActor(rgridWireActor);
  //////////////////////////////////////////////////////////////////////////
  
  
	std::string newWinName = "";
	std::ostringstream sprintfbuf;
	#if defined(SHOW_OBJECT_CONTOURS)
		sprintfbuf << "Decomposed into "<< cutter->GetNumberOfContours()  << " object contours";
	#else
		sprintfbuf << "Decomposed object";
	#endif
	newWinName = sprintfbuf.str(); 
	renWin->SetWindowName(newWinName.c_str());
		
	}
  }
  renWin->Render();
  iren->Start();

  return EXIT_SUCCESS;
}
