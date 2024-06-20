#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QHBoxLayout>
#include <QmitkStdMultiWidget.h>
#include <QmitkRenderWindow.h>
#include <QmitkRenderWindowWidget.h>
#include <mitkDataNode.h>
#include <mitkPointSetDataInteractor.h>
#include <mitkNodePredicateDataType.h>
#include <mitkImage.h>
#include <QFileDialog>
#include <mitkSurface.h>
#include <mitkIOUtil.h>
#include  "threeslicewidget.h"
#include <mitkNodePredicateDataType.h>
#include <mitkPointSetDataInteractor.h>
#include "PointSetDataNodeInteractor.h"
#include <QDebug>
#include "PanoramaView.h"
#include <usModuleRegistry.h>
#include <mitkAffineBaseDataInteractor3D.h>
#include "PointSetVtkMapper3D.h"
MainWindow::MainWindow(QWidget* parent) : QWidget(parent)
    , ui(new Ui::MainWindow)
{
    
    ui->setupUi(this);
    m_dataStorage =mitk::StandaloneDataStorage::New();
    m_dataStorage3D = mitk::StandaloneDataStorage::New();
    m_stdMultiWidget =new ThreeSliceWidget(this);
    m_stdMultiWidget->setDataStorage(m_dataStorage);
    m_stdMultiWidget->setDataStorage3D(m_dataStorage3D);
    QHBoxLayout *l=new QHBoxLayout(ui->widget_Center);
    l->addWidget(m_stdMultiWidget);
}

MainWindow::~MainWindow()
{
    delete ui;
    if(m_panoramaView)
    {
        m_panoramaView->deleteLater();
        m_panoramaView = nullptr;
    }
}

void MainWindow::on_pushButton_loadCBCT_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this, "please select a cbct dir");
    if (path.isEmpty())
        return;    
    mitk::IOUtil::Load(path.toStdString(), *m_dataStorage);
    mitk::DataNode* cbctNode = m_dataStorage->GetNode(mitk::NodePredicateDataType::New(mitk::Image::GetStaticNameOfClass()));
    cbctNode->SetName("teeth");
    cbctNode->SetProperty("volumerendering", mitk::BoolProperty::New(true));
    m_dataStorage3D->Add(cbctNode);
    for(int i=0;i<5;++i)
		mitk::RenderingManager::GetInstance()->InitializeViewByBoundingObjects(m_stdMultiWidget->getRenderWindow(i)->GetVtkRenderWindow(), m_dataStorage);
}

void MainWindow::on_pushButton_loadSTL_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "please select a .stl file", ".", "*.stl");
    if (filePath.isEmpty())
       return;
    mitk::IOUtil::Load(filePath.toStdString(), *m_dataStorage3D);
    QFileInfo fileinfo(filePath);
    std::string filebaseName = fileinfo.baseName().toStdString();
    mitk::DataNode* node = m_dataStorage3D->GetNamedNode(filebaseName);
    if (!node)
        return;
    m_dataStorage->Add(node);
    node->SetBoolProperty("pickable", true);
    auto affineDataInteractor = mitk::AffineBaseDataInteractor3D::New();
    affineDataInteractor->LoadStateMachine("AffineInteraction3D.xml",
        us::ModuleRegistry::GetModule("MitkDataTypesExt"));
    affineDataInteractor->SetEventConfig("AffineMouseConfig.xml", us::ModuleRegistry::GetModule("MitkDataTypesExt"));
    affineDataInteractor->SetDataNode(node);
    mitk::RenderingManager::GetInstance()->InitializeViewByBoundingObjects(m_stdMultiWidget->getRenderWindow(1)->GetVtkRenderWindow(), m_dataStorage3D);
}

void MainWindow::on_pushButton_cpr_clicked()
{
    if(!m_panoramaView)
    {
        m_panoramaView = new PanoramaView;
        m_panoramaView->setMitkRenderWindow(m_stdMultiWidget->getRenderWindow(0));
    }
    m_panoramaView->show();
}

void MainWindow::on_pushButton_selectPoint_clicked()
{
    std::string name = "targetPoint";
    mitk::DataNode *node=m_dataStorage->GetNamedNode(name);
    if(node)
    {
        m_dataStorage->Remove(node);
        m_dataStorage3D->Remove(node);
    }
    mitk::DataNode::Pointer targetNode = mitk::DataNode::New();
    targetNode->SetName(name);
    mitk::PointSet::Pointer ps = mitk::PointSet::New();
    targetNode->SetData(ps);
    targetNode->SetMapper(mitk::BaseRenderer::Standard3D,PointSetVtkMapper3D::New());
    targetNode->SetProperty("show contour", mitk::BoolProperty::New(true));
    targetNode->SetProperty("show distances", mitk::BoolProperty::New(true));
    targetNode->SetProperty("contoursize", mitk::FloatProperty::New(5));
    PointSetDataNodeInteractor::Pointer interactor = PointSetDataNodeInteractor::New();
    interactor->SetMaxPoints(2);
    try {
        interactor->LoadStateMachine("PointSet.xml");
        interactor->SetEventConfig("PointSetConfig.xml");
    }
    catch (const mitk::Exception& e) {
        qDebug() << QString::fromStdString(e.GetDescription());
    }
    interactor->SetDataNode(targetNode);
    m_dataStorage->Add(targetNode);
    m_dataStorage3D->Add(targetNode);
}

