// ==============================================================
// ssTIFF2VTK: Simple tool to convert both 8-bit and 16-bit TIFF
// files into VTK ImageData files.
// Developed by Matheus P. Viana - vianamp@gmail.com - 2014.05.29
// Susanne Rafelski Lab, University of California Irvine
// --------------------------------------------------------------
// How to Use:
// -----------
// 1. To convert a multi-paged TIFF file named Cell140502.tif
// 
// -Command-
// ./ssTIFF2VTK -prefix Cell140502 -save ImageData.vtk
//
// where:
// -prefix indicates the name prefix of the TIFF file
// -save   indicates the save that must be used to save the
//         VTK file. If no name is provided, the name
//         ImageData_Original.vtk will be used.
//
// 2. To convert a TIFF image sequence composed by the files
//    im00.tif, im01.tif, im03.tif, ..., im29.tif
//
// -Command-
// ./ssTIFF2VTK -prefix im -n 30 -save ImageData.vtk
//
// where:
// -prefix indicates the name prefix of the TIFF file
// -n      indicates the number of images in the sequence. In
//         this case, we have 30 images (from 0 to 29).
//         NOTE: The index if the first image in the sequence
//         must start with zero.
// -save   indicates the save that must be used to save the
//         VTK file. If no name is provided, the name
//         ImageData_Original.vtk will be used.
//
// ==============================================================

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vtkPointData.h>
#include <vtkDataArray.h>
#include <vtkImageData.h>
#include <vtkTIFFReader.h>
#include <vtkSmartPointer.h>
#include <vtkStructuredPoints.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnsignedShortArray.h>
#include <vtkStructuredPointsWriter.h>

int main(int argc, char *argv[]) {     
    
    int i;
    int _nfiles = 1;
    char _prefix[64];
    char _savenm[64];
    sprintf(_prefix,"im");
    sprintf(_savenm,"ImageData_Original.vtk");

    for (i = 0; i < argc; i++) {
        if (!strcmp(argv[i],"-prefix")) {
            sprintf(_prefix,"%s",argv[i+1]);
        }
        if (!strcmp(argv[i],"-n")) {
            _nfiles = atoi(argv[i+1]);
        }
    }

    // Not multi-paged TIFF file
    vtkSmartPointer<vtkTIFFReader> TIFFReader = vtkSmartPointer<vtkTIFFReader>::New();
    if (_nfiles>1) {
        printf("Running image sequence mode\n");
        TIFFReader -> SetDataExtent(0,0,0,0,0,_nfiles-1);
        TIFFReader -> SetFilePrefix(_prefix);
        TIFFReader -> SetFilePattern("%s%02d.tif");
        TIFFReader -> Update();
    // Multi-paged TIFF file
    } else {
        printf("Running multi-paged mode\n");
        sprintf(_prefix,"%s.tif",_prefix);
        TIFFReader -> SetFileName(_prefix);
    }
    TIFFReader -> Update();

    vtkImageData *Image = TIFFReader -> GetOutput();
    
    // 8-Bit images
    if (Image -> GetScalarType() == VTK_UNSIGNED_CHAR) {

        vtkSmartPointer<vtkStructuredPointsWriter> Writer = vtkSmartPointer<vtkStructuredPointsWriter>::New();
        Writer -> SetFileName(_savenm);
        Writer -> SetFileType(VTK_BINARY);
        #if (VTK_MAJOR_VERSION==5)
            Writer -> SetInput(TIFFReader->GetOutput());
        #else
            Writer -> SetInputData(TIFFReader->GetOutput());
        #endif
        Writer -> Write();

    // 16-Bit images
    } else if (Image -> GetScalarType() == VTK_UNSIGNED_SHORT) {

        printf("Converting from 16-bit to 8-bit...\n");

        vtkSmartPointer<vtkImageData> Image8 = vtkSmartPointer<vtkImageData>::New();
        Image8 -> ShallowCopy(Image);
        //Image8 -> Update();

        vtkDataArray *ScalarsShort = Image -> GetPointData() -> GetScalars();
        unsigned long int N = ScalarsShort -> GetNumberOfTuples();
        double range[2];
        ScalarsShort -> GetRange(range);

        // Scaling of intensities to 8-bit. Same process that is done in
        // ImageJ (http://rsbweb.nih.gov/ij/docs/guide/146-28.html)
        printf("Original intensities range: [%d-%d]\n",(int)range[0],(int)range[1]);

        vtkSmartPointer<vtkUnsignedCharArray> ScalarsChar = vtkSmartPointer<vtkUnsignedCharArray>::New();
        ScalarsChar -> SetNumberOfComponents(1);
        ScalarsChar -> SetNumberOfTuples(N);
        
        double x, y;
        unsigned long int register id;
        for ( id = N; id--; ) {
            x = ScalarsShort -> GetTuple1(id);
            y = 255.0 * (x-range[0]) / (range[1]-range[0]);
            ScalarsChar -> SetTuple1(id,(unsigned char)y);
        }
        ScalarsChar -> Modified();

        Image8 -> GetPointData() -> SetScalars(ScalarsChar);
        //Image8 -> Update();

        // Saving VTK file
        vtkSmartPointer<vtkStructuredPointsWriter> Writer = vtkSmartPointer<vtkStructuredPointsWriter>::New();
        Writer -> SetFileName(_savenm);
        Writer -> SetFileType(VTK_BINARY);
        #if (VTK_MAJOR_VERSION==5)
            Writer -> SetInput(Image8);
        #else
            Writer -> SetInputData(Image8);
        #endif
        Writer -> Write();

    } else {

        printf("Bit Depth Not Supported.\n");

    }

    return 0;
}
