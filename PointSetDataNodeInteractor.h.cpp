#include "PointSetDataNodeInteractor.h"

#include <mitkPointOperation.h>

#include <QDebug>

#include "mitkInteractionConst.h"  // TODO: refactor file
#include "mitkInternalEvent.h"
#include "mitkMapper.h"
#include "mitkMouseMoveEvent.h"
#include "mitkOperationEvent.h"
#include "mitkPlaneGeometryDataVtkMapper3D.h"
#include "mitkRenderingManager.h"
#include "mitkVtkMapper.h"
//
#include <mitkBaseRenderer.h>
#include <mitkDataInteractor.h>
#include <mitkGeometry3D.h>
#include <mitkInteractionConst.h>
#include <mitkInteractionPositionEvent.h>
#include <mitkRotationOperation.h>
#include <mitkSurface.h>
#include <vtkCamera.h>
#include <vtkInteractorObserver.h>
#include <vtkInteractorStyle.h>
#include <vtkMapper.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkRenderWindowInteractor.h>

#include "mitkBaseRenderer.h"
#include "mitkDispatcher.h"
#include "mitkInteractionEvent.h"
#include "mitkUndoController.h"
#include "mitkVtkPropRenderer.h"
#include "vtkAbstractPicker.h"
#include "vtkCellPicker.h"

void PointSetDataNodeInteractor::AddPoint(mitk::StateMachineAction *stateMachineAction,
                                         mitk::InteractionEvent *interactionEvent)
{
    unsigned int timeStep = interactionEvent->GetSender()->GetTimeStep(GetDataNode()->GetData());
    mitk::ScalarType timeInMs = interactionEvent->GetSender()->GetTime();

    if (m_PointSet->GetSize(timeStep) >= m_MaxNumberOfPoints) {
        m_PointSet->SetSelectInfo(m_PointSet->GetSize(timeStep) - 1, false);
        m_PointSet->RemovePointIfExists(m_PointSet->GetSize(timeStep) - 1);
    }

    auto *positionEvent = dynamic_cast<mitk::InteractionPositionEvent *>(interactionEvent);
    if (positionEvent != nullptr) {
        mitk::Point3D itkPoint = positionEvent->GetPositionInWorld();
        this->UnselectAll(timeStep, timeInMs);
        int lastPosition = 0;
        mitk::PointSet::PointsIterator it, end;
        it = m_PointSet->Begin(timeStep);
        end = m_PointSet->End(timeStep);
        while (it != end) {
            if (!m_PointSet->IndexExists(lastPosition, timeStep)) break;
            ++it;
            ++lastPosition;
        }
        if (m_PointSet->IsEmpty()) {
            lastPosition = 0;
        }
        auto *doOp = new mitk::PointOperation(mitk::OpINSERT, timeInMs, itkPoint, lastPosition);
        if (m_UndoEnabled) {
            auto *undoOp = new mitk::PointOperation(mitk::OpREMOVE, timeInMs, itkPoint, lastPosition);
            mitk::OperationEvent *operationEvent = new mitk::OperationEvent(m_PointSet, doOp, undoOp, "Add point");
            mitk::OperationEvent::IncCurrObjectEventId();
            m_UndoController->SetOperationEvent(operationEvent);
        }

        m_PointSet->ExecuteOperation(doOp);
        if (!m_UndoEnabled) delete doOp;
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();

        IsClosedContour(stateMachineAction, interactionEvent);

        if (m_MaxNumberOfPoints > 0 && m_PointSet->GetSize(timeStep) >= m_MaxNumberOfPoints) {
            this->NotifyResultReady();
            mitk::InternalEvent::Pointer event = mitk::InternalEvent::New(nullptr, this, "MaximalNumberOfPoints");
            positionEvent->GetSender()->GetDispatcher()->QueueEvent(event.GetPointer());
        }
    }
}

PointSetDataNodeInteractor::PointSetDataNodeInteractor()
{
}

PointSetDataNodeInteractor::~PointSetDataNodeInteractor()
{
}