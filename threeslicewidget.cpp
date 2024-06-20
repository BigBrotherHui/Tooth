#include "threeslicewidget.h"

#include <mitkBaseRenderer.h>
#include <mitkDataStorage.h>
#include <mitkDataNode.h>

#include <QDebug>

#include "mitkSliceNavigationController.h"
#include <mitkInteractionConst.h>
#include "ui_threeslicewidget.h"
#include <mitkDisplayActionEventHandlerStd.h>
#include <mitkPlaneGeometryDataMapper2D.h>
#include <mitkNodePredicateDataType.h>
#include <mitkImage.h>
#include <mitkDisplayActionEventFunctions.h>

mitk::Color GetDecorationColor(unsigned int widgetNumber)
{
    switch (widgetNumber) {
        case 0: {
            float green[3] = {0.0f, 0.69f, 0.0f};
            return mitk::Color(green);
        }
        case 1: {
            float blue[3] = {0.0, 0.502f, 1.0f};
            return mitk::Color(blue);
        }
        case 2: {
            float red[3] = {0.753f, 0.0f, 0.0f};
            return mitk::Color(red);
        }
        default:
            return mitk::Color();
    }
}

ThreeSliceWidget::ThreeSliceWidget(QWidget *parent,bool hlayout) : QWidget(parent), ui(new Ui::ThreeSliceWidget)
{
    ui->setupUi(this);
    ui->widget_panorama->GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard2D);
    ui->widget_3d->GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard3D);

    ui->widget_sagittal->GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard2D);
    ui->widget_coronal->GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard2D);
    ui->widget_axial->GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard2D);

    ui->widget_panorama->GetSliceNavigationController()->SetDefaultViewDirection(mitk::AnatomicalPlane::Axial);
    ui->widget_sagittal->GetSliceNavigationController()->SetDefaultViewDirection(mitk::AnatomicalPlane::Sagittal);
    ui->widget_coronal->GetSliceNavigationController()->SetDefaultViewDirection(mitk::AnatomicalPlane::Coronal);
    ui->widget_axial->GetSliceNavigationController()->SetDefaultViewDirection(mitk::AnatomicalPlane::Axial);

    m_PlaneNode1 =
        mitk::BaseRenderer::GetInstance(ui->widget_sagittal->renderWindow())->GetCurrentWorldPlaneGeometryNode();
    m_PlaneNode1->SetColor(GetDecorationColor(0));
    m_PlaneNode2 =
        mitk::BaseRenderer::GetInstance(ui->widget_coronal->renderWindow())->GetCurrentWorldPlaneGeometryNode();
    m_PlaneNode2->SetColor(GetDecorationColor(1));
    m_PlaneNode3 =
        mitk::BaseRenderer::GetInstance(ui->widget_axial->renderWindow())->GetCurrentWorldPlaneGeometryNode();
    m_PlaneNode3->SetColor(GetDecorationColor(2));

    m_PlaneNode1->SetIntProperty("Crosshair.Gap Size", 0);
    m_PlaneNode2->SetIntProperty("Crosshair.Gap Size", 0);
    m_PlaneNode3->SetIntProperty("Crosshair.Gap Size", 0);

    mitk::PlaneGeometryDataMapper2D::Pointer mapper1, mapper2, mapper3;
    std::set<mitk::PlaneGeometryDataMapper2D::Pointer> mappers;
    mitk::BaseRenderer *renderer1 = mitk::BaseRenderer::GetInstance(ui->widget_sagittal->renderWindow());
    m_PlaneNode1 = renderer1->GetCurrentWorldPlaneGeometryNode();
    m_PlaneNode1->SetProperty("visible", mitk::BoolProperty::New(true));
    m_PlaneNode1->SetProperty("name", mitk::StringProperty::New(std::string("sagittal.plane")));
    m_PlaneNode1->SetProperty("includeInBoundingBox", mitk::BoolProperty::New(false));
    m_PlaneNode1->SetProperty("helper object", mitk::BoolProperty::New(true));
    mapper1 = mitk::PlaneGeometryDataMapper2D::New();
    m_PlaneNode1->SetMapper(mitk::BaseRenderer::Standard2D, mapper1);
    mappers.emplace(mapper1);

    mitk::BaseRenderer *renderer2 = mitk::BaseRenderer::GetInstance(ui->widget_coronal->renderWindow());
    m_PlaneNode2 = renderer2->GetCurrentWorldPlaneGeometryNode();
    m_PlaneNode2->SetProperty("visible", mitk::BoolProperty::New(true));
    m_PlaneNode2->SetProperty("name", mitk::StringProperty::New(std::string("coronal.plane")));
    m_PlaneNode2->SetProperty("includeInBoundingBox", mitk::BoolProperty::New(false));
    m_PlaneNode2->SetProperty("helper object", mitk::BoolProperty::New(true));
    mapper2 = mitk::PlaneGeometryDataMapper2D::New();
    m_PlaneNode2->SetMapper(mitk::BaseRenderer::Standard2D, mapper2);
    mappers.emplace(mapper2);

    mitk::BaseRenderer *renderer3 = mitk::BaseRenderer::GetInstance(ui->widget_axial->renderWindow());
    m_PlaneNode3 = renderer3->GetCurrentWorldPlaneGeometryNode();
    m_PlaneNode3->SetProperty("visible", mitk::BoolProperty::New(true));
    m_PlaneNode3->SetProperty("name", mitk::StringProperty::New(std::string("axial.plane")));
    m_PlaneNode3->SetProperty("includeInBoundingBox", mitk::BoolProperty::New(false));
    m_PlaneNode3->SetProperty("helper object", mitk::BoolProperty::New(true));
    mapper3 = mitk::PlaneGeometryDataMapper2D::New();
    m_PlaneNode3->SetMapper(mitk::BaseRenderer::Standard2D, mapper3);
    mappers.emplace(mapper3);

    mapper1->setReletivePlaneMappers(mappers);
    mapper2->setReletivePlaneMappers(mappers);
    mapper3->setReletivePlaneMappers(mappers);

    m_renderwindows[0] = ui->widget_panorama;
    m_renderwindows[1] = ui->widget_3d;
    m_renderwindows[2] = ui->widget_sagittal;
    m_renderwindows[3] = ui->widget_coronal;
    m_renderwindows[4] = ui->widget_axial;

    initInteractor();
}

ThreeSliceWidget::~ThreeSliceWidget()
{
    delete ui;
}

void ThreeSliceWidget::setDataStorage(mitk::StandaloneDataStorage::Pointer ds)
{
    if (!ds || m_dataStorage ==ds) return;
    m_dataStorage = ds;
    for(int i=0;i<5;++i)
    {
        if(i==1)
            continue;
        m_renderwindows[i]->GetRenderer()->SetDataStorage(ds);
    }
    if (m_PlaneNode1.IsNotNull() && m_PlaneNode2.IsNotNull() && m_PlaneNode3.IsNotNull()) {
        if (!m_dataStorage->Exists(m_PlaneNode1))
            m_dataStorage->Add(m_PlaneNode1);
        if (!m_dataStorage->Exists(m_PlaneNode2))
            m_dataStorage->Add(m_PlaneNode2);
        if (!m_dataStorage->Exists(m_PlaneNode3))
            m_dataStorage->Add(m_PlaneNode3);
    }
}

void ThreeSliceWidget::setDataStorage3D(mitk::StandaloneDataStorage::Pointer ds)
{
    if (!ds || m_dataStorage3D == ds) return;
    m_dataStorage3D = ds;
    m_renderwindows[1]->GetRenderer()->SetDataStorage(ds);
}

QmitkRenderWindow* ThreeSliceWidget::getRenderWindow(int index)
{
    if (index > 4)
        return nullptr;
    return m_renderwindows[index];
}

mitk::StdFunctionCommand::ActionFunction ThreeSliceWidget::SetCrosshairSynchronizedAction() {
    auto actionFunction = [&](const itk::EventObject& displayInteractorEvent)
    {
        if (mitk::DisplaySetCrosshairEvent().CheckEvent(&displayInteractorEvent))
        {
            const mitk::DisplaySetCrosshairEvent* displayActionEvent =
                dynamic_cast<const mitk::DisplaySetCrosshairEvent*>(&displayInteractorEvent);
            const mitk::BaseRenderer::Pointer sendingRenderer = displayActionEvent->GetSender();
            if (nullptr == sendingRenderer || sendingRenderer->GetMapperID() == mitk::BaseRenderer::Standard3D)
            {
                return;
            }
            for (int i = 0; i < 5; ++i)
            {
                if(i==1)
                    continue;
                auto renderWindow = getRenderWindow(i)->GetVtkRenderWindow();
                mitk::BaseRenderer::GetInstance(renderWindow)
                    ->GetSliceNavigationController()
                    ->SelectSliceByPoint(displayActionEvent->GetPosition());
            }
        }
    };

    return actionFunction;
}

void ThreeSliceWidget::initInteractor()
{
    if(!m_DisplayActionEventBroadcast)
    {
        m_DisplayActionEventBroadcast = mitk::DisplayActionEventBroadcast::New();
        m_DisplayActionEventBroadcast->LoadStateMachine("DisplayInteraction.xml");
        m_DisplayActionEventBroadcast->SetEventConfig("DisplayConfigMITKBase.xml");
        m_DisplayActionEventBroadcast->AddEventConfig("DisplayConfigCrosshair.xml");
    }
    if (!m_DisplayActionEventHandler)
    {
        m_DisplayActionEventHandler = std::make_unique<mitk::DisplayActionEventHandlerStd>();
        m_DisplayActionEventHandler->SetObservableBroadcast(m_DisplayActionEventBroadcast);
        mitk::StdFunctionCommand::ActionFunction actionFunction = ThreeSliceWidget::SetCrosshairSynchronizedAction();
        m_DisplayActionEventHandler->ConnectDisplayActionEvent(mitk::DisplaySetCrosshairEvent(nullptr, mitk::Point3D()),
                                                              actionFunction);

	    actionFunction = mitk::DisplayActionEventFunctions::MoveSenderCameraAction();
	    m_DisplayActionEventHandler->ConnectDisplayActionEvent(mitk::DisplayMoveEvent(nullptr, mitk::Vector2D()),
	                                                              actionFunction);

	    actionFunction = mitk::DisplayActionEventFunctions::ZoomSenderCameraAction();
	    m_DisplayActionEventHandler->ConnectDisplayActionEvent(mitk::DisplayZoomEvent(nullptr, 0.0, mitk::Point2D()),
	                                                              actionFunction);

	    actionFunction = mitk::DisplayActionEventFunctions::ScrollSliceStepperAction();
	    m_DisplayActionEventHandler->ConnectDisplayActionEvent(mitk::DisplayScrollEvent(nullptr, 0, true),
	                                                              actionFunction);

	    actionFunction = mitk::DisplayActionEventFunctions::SetLevelWindowAction();
	    m_DisplayActionEventHandler->ConnectDisplayActionEvent(
		mitk::DisplaySetLevelWindowEvent(nullptr, mitk::ScalarType(), mitk::ScalarType()), actionFunction);
    }
}


