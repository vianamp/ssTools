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
    int _nfiles = 0;
    char _prefix[64];
    sprintf(_prefix,"im");

    for (i = 0; i < argc; i++) {
        if (!strcmp(argv[i],"-prefix")) {
            sprintf(_prefix,"%s",argv[i+1]);
        }
        if (!strcmp(argv[i],"-n")) {
            _nfiles = atoi(argv[i+1]);
        }
    }

    if (!_nfiles) {
        printf("The number of TIF files must be specified!\n");
        return -1;
    }

    vtkSmartPointer<vtkTIFFReader> TIFFReader = vtkSmartPointer<vtkTIFFReader>::New();
    TIFFReader -> SetDataExtent(0,0,0,0,0,_nfiles-1);
    TIFFReader -> SetFilePrefix(_prefix);
    TIFFReader -> SetFilePattern("%s%02d.tif");
    TIFFReader -> Update();

    vtkImageData *Image = TIFFReader -> GetOutput();
    
    if (Image -> GetScalarType() == VTK_UNSIGNED_CHAR) {

        vtkSmartPointer<vtkStructuredPointsWriter> Writer = vtkSmartPointer<vtkStructuredPointsWriter>::New();
        Writer -> SetFileName("temp.vtk");
        Writer -> SetFileType(VTK_BINARY);
        Writer -> SetInput(TIFFReader->GetOutput());
        Writer -> Write();

    } else if (Image -> GetScalarType() == VTK_UNSIGNED_SHORT) {

        printf("Converting from 16-bit to 8-bit...\n");

        vtkSmartPointer<vtkImageData> Image8 = vtkSmartPointer<vtkImageData>::New();
        Image8 -> ShallowCopy(Image);
        Image8 -> Update();

        vtkDataArray *ScalarsShort = Image -> GetPointData() -> GetScalars();
        unsigned long int N = ScalarsShort -> GetNumberOfTuples();
        double range[2];
        ScalarsShort -> GetRange(range);

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
        Image8 -> Update();

        vtkSmartPointer<vtkStructuredPointsWriter> Writer = vtkSmartPointer<vtkStructuredPointsWriter>::New();
        Writer -> SetFileName("temp.vtk");
        Writer -> SetFileType(VTK_BINARY);
        Writer -> SetInput(Image8);
        Writer -> Write();

    } else {

        printf("Bit Depth Not Supported.\n");

    }

    return 0;
}

