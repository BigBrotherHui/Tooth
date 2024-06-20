#ifndef POINTSETDATANODEINTERACTOR_H
#define POINTSETDATANODEINTERACTOR_H
#include "mitkCommon.h"
#include "mitkDataInteractor.h"
#include <mitkBaseRenderer.h>
#include <mitkDataInteractor.h>
#include <mitkGeometry3D.h>
#include <mitkIOUtil.h>
#include <mitkInteractionConst.h>
#include <mitkInteractionPositionEvent.h>
#include <mitkPointSet.h>
#include <mitkRotationOperation.h>
#include <mitkSurface.h>
#include <vtkCamera.h>
#include <vtkInteractorObserver.h>
#include <vtkInteractorStyle.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkRenderWindowInteractor.h>

#include "QmitkRenderWindow.h"
#include "QmitkStdMultiWidget.h"
#include "mitkInteractionEvent.h"
#include "mitkPointSetDataInteractor.h"
#include "vtkPlaneSource.h"
#include "vtkRendererCollection.h"
#include "vtkSphereSource.h"

class PointSetDataNodeInteractor : public QObject, public mitk::PointSetDataInteractor {
    Q_OBJECT

public:
    mitkClassMacro(PointSetDataNodeInteractor, mitk::PointSetDataInteractor);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

protected:
    PointSetDataNodeInteractor();
    ~PointSetDataNodeInteractor() override;
    virtual void AddPoint(mitk::StateMachineAction *, mitk::InteractionEvent *event);
};

#endif  // POINTSETDATANODEINTERACTOR_H
