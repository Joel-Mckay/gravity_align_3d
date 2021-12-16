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
 


#define DEBUG_MODE 1
#define SHOW_OBJECT 1
#define SHOW_OBJECT_CONTOURS 1

/*
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


#define SHOW_BOUNDING_BOX 1
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


int main(int argc, char* argv[])
{
  
  double zLayerSliceThickness = OBJECT_CONTOUR_LAYER_Z_SPACING;
  char flagTextureOK=0;

  if (argc < 4)
  {
    std::cout << "Usage: " << argv[0] << " objfile mtlfile texturepath" << std::endl
			<< "e.g. "<< argv[0] <<" doorman.obj doorman.mtl doorman" << std::endl;
    return EXIT_FAILURE;
  }
  
 vtkNew<vtkOBJImporter> importer;
  
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

  vtkNew<vtkNamedColors> colors;

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

	vtkPolyData* inputPolyData = dynamic_cast<vtkPolyData*>(actor->GetMapper()->GetInput());
	inputPolyData->ComputeBounds();

	vtkPolyDataMapper* inputMapper = dynamic_cast<vtkPolyDataMapper*>(actor->GetMapper());
	inputMapper->SetInputData(inputPolyData);

	// Create a plane to cut
	vtkNew<vtkPlane> plane;
	plane->SetOrigin(inputPolyData->GetCenter());
	plane->SetNormal(0, 0, 1);	//angle of the slice from XY plan

	//double* vtkDataSet::GetBounds() 	
	//Returns a pointer to the geometry bounding box in the form (xmin,xmax, ymin,ymax, zmin,zmax). 
	double allBounds[6];
	allBounds[BOUNDS_XMIN] = inputPolyData->GetBounds()[BOUNDS_XMIN];
	allBounds[BOUNDS_XMAX] = inputPolyData->GetBounds()[BOUNDS_XMAX];
	allBounds[BOUNDS_YMIN] = inputPolyData->GetBounds()[BOUNDS_YMIN];
	allBounds[BOUNDS_YMAX] = inputPolyData->GetBounds()[BOUNDS_YMAX];
	allBounds[BOUNDS_ZMIN] = inputPolyData->GetBounds()[BOUNDS_ZMIN];
	allBounds[BOUNDS_ZMAX] = inputPolyData->GetBounds()[BOUNDS_ZMAX];
	
	double minBound[3];
	minBound[COORDS_X] =allBounds[BOUNDS_XMIN];
	minBound[COORDS_Y] = allBounds[BOUNDS_YMIN];
	minBound[COORDS_Z] = allBounds[BOUNDS_ZMIN];

	double maxBound[3];
	maxBound[COORDS_X] = allBounds[BOUNDS_XMAX];
	maxBound[COORDS_Y] = allBounds[BOUNDS_YMAX];
	maxBound[COORDS_Z] = allBounds[BOUNDS_ZMAX];
 
	double center[3];
	center[COORDS_X] = inputPolyData->GetCenter()[COORDS_X];
	center[COORDS_Y] = inputPolyData->GetCenter()[COORDS_Y];
	center[COORDS_Z] = inputPolyData->GetCenter()[COORDS_Z];

	double distanceMin = sqrt(vtkMath::Distance2BetweenPoints(minBound, center));   //better size estimate
	double distanceMax = sqrt(vtkMath::Distance2BetweenPoints(maxBound, center));
	
	//check object model is of sane design
	if( (std::isinf(distanceMin) == true) || (std::isinf(distanceMax) == true) || (flagTextureOK==0))
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

		//make oversized by one layer if lattice violation, as milling to size is easier than adding material
		zLayerSliceThickness = OBJECT_CONTOUR_LAYER_Z_SPACING;	//default to normal mm scale
		#if SCAN_MODE == 0
			int numOfSlices = vtkMath::Ceil((distanceMin + distanceMax)/zLayerSliceThickness);
		#elif SCAN_MODE == 1
			int numOfSlices = vtkMath::Ceil( ( std::fabs(allBounds[BOUNDS_ZMIN])  + std::fabs(allBounds[BOUNDS_ZMAX]) )/zLayerSliceThickness);
		#endif
		
		//contours too few?
		if(numOfSlices < 99)
		{
			zLayerSliceThickness = OBJECT_CONTOUR_LAYER_Z_SPACING/10.0; //convert meter scale to dm
			
			#if SCAN_MODE == 0
				numOfSlices = vtkMath::Ceil((distanceMin + distanceMax)/zLayerSliceThickness);
			#elif SCAN_MODE == 1
				numOfSlices = vtkMath::Ceil( ( std::fabs(allBounds[BOUNDS_ZMIN])  + std::fabs(allBounds[BOUNDS_ZMAX]) )/zLayerSliceThickness);
			#endif
			
			#if defined(DEBUG_MODE)
				std::cout << "\nWarning: scaled up... now in decimeter \n";
			#endif
		}
		
		//contours too few?
		if(numOfSlices < 99)
		{
			zLayerSliceThickness = OBJECT_CONTOUR_LAYER_Z_SPACING/100.0; //convert meter scale to cm
			
			#if SCAN_MODE == 0
				numOfSlices = vtkMath::Ceil((distanceMin + distanceMax)/zLayerSliceThickness);
			#elif SCAN_MODE == 1
				numOfSlices = vtkMath::Ceil( ( std::fabs(allBounds[BOUNDS_ZMIN])  + std::fabs(allBounds[BOUNDS_ZMAX]) )/zLayerSliceThickness);
			#endif
			
			#if defined(DEBUG_MODE)
				std::cout << "\nWarning: scaled up... now in centimeter \n";
			#endif
		}
		
		//contours still too few?
		if(numOfSlices < 99)
		{
			zLayerSliceThickness = OBJECT_CONTOUR_LAYER_Z_SPACING/1000.0; //convert meter scale to mm
			
			#if SCAN_MODE == 0
				numOfSlices = vtkMath::Ceil((distanceMin + distanceMax)/zLayerSliceThickness);
			#elif SCAN_MODE == 1
				numOfSlices = vtkMath::Ceil( ( std::fabs(allBounds[BOUNDS_ZMIN])  + std::fabs(allBounds[BOUNDS_ZMAX]) )/zLayerSliceThickness);
			#endif
			
			#if defined(DEBUG_MODE)
				std::cout << "\nWarning: scaled up... now in millimeter \n";
			#endif
		}
		
		//contours still too Many?
		if(numOfSlices >= OBJECT_MAX_CONTOUR_LAYER_Z_COUNT)
		{
			zLayerSliceThickness = OBJECT_CONTOUR_LAYER_Z_SPACING/OBJECT_MAX_CONTOUR_LAYER_Z_COUNT; //convert unknown scale to fit
			
			numOfSlices = vtkMath::Floor(OBJECT_MAX_CONTOUR_LAYER_Z_COUNT);
			
			#if defined(DEBUG_MODE)
				std::cout << "\nWarning: scaled down to fit maximum area... \n";
			#endif
		}
		
		//still bad slice?
		if(numOfSlices < 4){
			numOfSlices=3;
			#if defined(DEBUG_MODE)
					std::cout << "\nWarning model cutter defaults to  " << numOfSlices << " slice(s)\n";
			#endif
		}
		
		
		#if defined(DEBUG_MODE)
			#if SCAN_MODE == 0
				std::cout << "\nFrom distanceZ=["<< -distanceMin << ", " << distanceMax << "] has about " << numOfSlices << " slices\n";
			#elif SCAN_MODE == 1
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
			cutter->GenerateValues(numOfSlices, -distanceMin, distanceMax);
		#elif SCAN_MODE == 1
			//Bug: Using the bounding cube Z-height causes loss of the model nose on test4.sh
			cutter->GenerateValues(numOfSlices,  allBounds[BOUNDS_ZMIN], allBounds[BOUNDS_ZMAX]);
		#endif
		
		vtkNew<vtkPolyDataMapper> cutterMapper;
		cutterMapper->SetInputConnection(cutter->GetOutputPort());
		
		#if defined(DEBUG_MODE)
			std::cout  << std::endl << "cutterMapper->GetNumberOfPieces has " << cutterMapper->GetNumberOfPieces() << std::endl;
			std::cout  << std::endl << "cutter->GetNumberOfContours has " << cutter->GetNumberOfContours()  << std::endl;
		#endif
		
		cutterMapper->ScalarVisibilityOff();	//Turn off flag to control whether scalar data is used to color objects. 
									//todo: BUG 1 for cat test object, the blue bum bug arrises if off
		
		cutterMapper->SetResolveCoincidentTopologyToPolygonOffset(); //todo: performance drops a bit
	#endif
	

	#if defined(SHOW_OBJECT_CONTOURS)
		// Create plane actor
		vtkNew<vtkActor> planeActor;
		planeActor->GetProperty()->SetColor(colors->GetColor3d("Deep_pink").GetData());
		planeActor->GetProperty()->SetLineWidth(zSpacingPx);
		planeActor->SetMapper(cutterMapper);
		renderer->AddActor(planeActor); // display the rectangle resulting from the cut
	#endif
	
	#if defined(SHOW_BOUNDING_BOX)
		 // Create cube actor
		vtkNew<vtkActor> cubeActor;
		cubeActor->GetProperty()->SetColor( colors->GetColor3d("Aquamarine").GetData());
		cubeActor->GetProperty()->SetOpacity(0.2);
		cubeActor->SetMapper(cubeMapper);
		renderer->AddActor(cubeActor); // display 
	#endif

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
