
#include "PointSelectionWindowInterface.h"
#include <vtkMatrix4x4.h>
#include <vtkSmartPointer.h>
void a()
{
    PointSelectionWindowInterface w;
    w.addSTLFile("D:/Images/Teeth/__1/5.stl", "1");
    w.addSTLFile("D:/Images/Teeth/__1/tool.stl", "2");
    vtkSmartPointer<vtkMatrix4x4> mt = vtkSmartPointer<vtkMatrix4x4>::New();
    mt->SetElement(2, 3, 100);
    w.setMatrix("2", mt->GetData());
    w.OpenWindow();
}
int main(int argc, char *argv[])
{
    std::cout << "first call" << std::endl;
    a();
    std::cout << "second call" << std::endl;
    a();
    return 0;
}
