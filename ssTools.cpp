#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vtkPointData.h>
#include <vtkDataArray.h>
#include <vtkImageData.h>
#include <vtkTIFFReader.h>
#include <vtkTIFFWriter.h>
#include <vtkSmartPointer.h>
#include <vtkStructuredPoints.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnsignedShortArray.h>
#include <vtkStructuredPointsWriter.h>
#include <vtkStructuredPointsReader.h>

void TIFF2VTK(const char _prefix[], int _nfiles) {

    char FileName[256];

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
        sprintf(FileName,"%s.tif",_prefix);
        TIFFReader -> SetFileName(FileName);
    }
    TIFFReader -> Update();

    sprintf(FileName,"%s.vtk",_prefix);

    vtkImageData *Image = TIFFReader -> GetOutput();
    
    // 8-Bit images
    if (Image -> GetScalarType() == VTK_UNSIGNED_CHAR) {

        vtkSmartPointer<vtkStructuredPointsWriter> Writer = vtkSmartPointer<vtkStructuredPointsWriter>::New();
        Writer -> SetFileName(FileName);
        Writer -> SetFileType(VTK_BINARY);
        Writer -> SetInputData(TIFFReader->GetOutput());
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
        Writer -> SetFileName(FileName);
        Writer -> SetFileType(VTK_BINARY);
        Writer -> SetInputData(Image8);
        Writer -> Write();

    } else {

        printf("Bit Depth Not Supported.\n");

    }

}

void VTK2TIFFSeq(const char _prefix[]) {
    char FileName[256];
    sprintf(FileName,"%s.vtk",_prefix);

    vtkSmartPointer<vtkStructuredPointsReader> VTKReader = vtkSmartPointer<vtkStructuredPointsReader>::New();
    VTKReader -> SetFileName(FileName);
    VTKReader -> Update();

    vtkSmartPointer<vtkTIFFWriter> TIFFWriter = vtkSmartPointer<vtkTIFFWriter>::New();
    TIFFWriter -> SetInputData(VTKReader -> GetOutput());
    TIFFWriter -> SetFilePattern("%s%04d.tif");
    TIFFWriter -> SetFilePrefix(_prefix);
    TIFFWriter -> Write();
}

int main(int argc, char *argv[]) {     
    
    int i, mode;
    int _nfiles = 1;
    char _prefix[256];
    sprintf(_prefix,"im");

    for (i = 0; i < argc; i++) {
        if (!strcmp(argv[i],"-vtk2tiffseq")) {
            mode = 1;
        }
        if (!strcmp(argv[i],"-tiff2vtk")) {
            mode = 2;
        }
        if (!strcmp(argv[i],"-prefix")) {
            sprintf(_prefix,"%s",argv[i+1]);
        }
        if (!strcmp(argv[i],"-n")) {
            _nfiles = atoi(argv[i+1]);
        }
    }

    if (mode==1)
        VTK2TIFFSeq(_prefix);

    if (mode==2)
        TIFF2VTK(_prefix,_nfiles);

    return 0;
}
