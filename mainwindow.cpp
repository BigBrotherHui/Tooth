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
#include <QDebug>
#include "PanoramaView.h"
#include <usModuleRegistry.h>
#include <mitkAffineBaseDataInteractor3D.h>
#include "PointSetVtkMapper3D.h"
#include <itkImageSeriesReader.h>
#include <itkGDCMImageIO.h>
#include <itkGDCMSeriesFileNames.h>
#include <mitkITKImageImport.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkPiecewiseFunction.h>
#include <vtkColorTransferFunction.h>
#include <vtkVolumeProperty.h>
#include <vtkAutoInit.h>
#include <QDateTime>
#include <QTemporaryDir>
#include <vtkDiscreteFlyingEdges3D.h>
#include <vtkSTLWriter.h>
#include <vtkNIFTIImageWriter.h>
#include <vtkDecimatePro.h>
#include <vtkIterativeClosestPointTransform.h>
#include <vtkLandmarkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkVertexGlyphFilter.h>
VTK_MODULE_INIT(vtkRenderingOpenGL2);
VTK_MODULE_INIT(vtkRenderingVolumeOpenGL2);
VTK_MODULE_INIT(vtkInteractionStyle);
VTK_MODULE_INIT(vtkRenderingFreeType);
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

void MainWindow::addSTLFile(const std::string& filePath, const std::string objectName)
{
    if (filePath.length()==0 || objectName.length()==0)
        return;
    QTemporaryDir tempDir;
    if (!tempDir.isValid()) {
        qWarning() << "Failed to create temporary directory.";
        return;
    }
    QString newName = QDateTime::currentDateTime().toString("yyyy_MM_dd_hh_mm_ss");
    QString newFilePath = tempDir.path() + "/" + newName + ".stl";
    QFile::copy(QString::fromStdString(filePath), newFilePath);
    if (m_stlIndex < 2)
    {
        newName = QString::fromStdString(m_stlName[m_stlIndex]);
        ++m_stlIndex;
    }
    const std::string &filePath2 = newFilePath.toStdString();
    const std::string &objectName2 = newName.toStdString();
    mitk::IOUtil::Load(filePath2, *m_dataStorage3D);
    QFileInfo fileinfo(QString::fromStdString(filePath2));
    std::string filebaseName = fileinfo.baseName().toStdString();
    mitk::DataNode* node = m_dataStorage3D->GetNamedNode(filebaseName);
    if (!node)
        return;
    node->SetName(objectName2);
    m_dataStorage->Add(node);
    node->SetBoolProperty("pickable", true);
    node->SetFloatProperty("line width", 10);
    auto affineDataInteractor = mitk::AffineBaseDataInteractor3D::New();
    affineDataInteractor->LoadStateMachine("AffineInteraction3D.xml",
        us::ModuleRegistry::GetModule("MitkDataTypesExt"));
    affineDataInteractor->SetEventConfig("AffineMouseConfig.xml", us::ModuleRegistry::GetModule("MitkDataTypesExt"));
    affineDataInteractor->SetDataNode(node);
    mitk::RenderingManager::GetInstance()->InitializeViewByBoundingObjects(m_stdMultiWidget->getRenderWindow(1)->GetVtkRenderWindow(), m_dataStorage3D);
}

void MainWindow::setMatrix(const std::string& objectName, vtkMatrix4x4* vmt)
{
    if (!vmt)
        return;
    mitk::DataNode* node = m_dataStorage->GetNamedNode(objectName);
    if (!node)
        return;
    if (!node->GetData() || !node->GetData()->GetGeometry())
        return;
    node->GetData()->GetGeometry()->SetIndexToWorldTransformByVtkMatrix(vmt);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

vtkMatrix4x4* MainWindow::getMatrix(const std::string& objectName)
{
    mitk::DataNode* node = m_dataStorage->GetNamedNode(objectName);
    if (!node)
        return nullptr;
    if (!node->GetData() || !node->GetData()->GetGeometry())
        return nullptr;
    return node->GetData()->GetGeometry()->GetVtkMatrix();
}

std::vector<std::array<std::array<double, 3>, 2> > MainWindow::getPointPairPosition()
{
    std::vector<std::array<std::array<double, 3>, 2> > result;
    std::array<std::array<double, 3>, 2> tmp;
    for(int i=0;i<m_pointIndex;++i)
    {
        mitk::PointSet* targetPoint = m_dataStorage->GetNamedObject<mitk::PointSet>(QString("targetPoint%1").arg(QString::number(i)).toStdString());
        if (!targetPoint || targetPoint->GetSize() != 2)
            continue;
        for (int i = 0; i < 2; ++i)
        {
            for (int j = 0; j < 3; ++j)
            {
                tmp[i][j] = targetPoint->GetPoint(i)[j];
            }
        }
        result.push_back(tmp);
    }
    return result;
}

void copyDirectory(const QString& sourceDir, const QString& tempDir)
{
    QDir source(sourceDir);
    QDir temp(tempDir);
    if (!temp.exists()) {
        if (!temp.mkpath(".")) {
            qWarning() << "Failed to create temporary directory:" << tempDir;
            return;
        }
    }
    QStringList files = source.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    foreach(const QString & file, files) {
        QString srcFilePath = source.filePath(file);
        QString destFilePath = temp.filePath(file);

        if (QFileInfo(srcFilePath).isDir()) {
            copyDirectory(srcFilePath, destFilePath);
        }
        else {
            if (!QFile::copy(srcFilePath, destFilePath)) {
                qWarning() << "Failed to copy file:" << srcFilePath;
            }
        }
    }
}

void MainWindow::on_pushButton_loadCBCT_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this, "please select a cbct dir");
    if (path.isEmpty())
        return;
    QTemporaryDir tempDir;
    if (!tempDir.isValid()) {
        qWarning() << "Failed to create temporary directory.";
        return;
    }
    copyDirectory(path, tempDir.path());
    using TPixel = signed short;
    const unsigned int DIM3 = 3;
    using TImage = itk::Image<TPixel, DIM3>;
    using TImagePtr = TImage::Pointer;
    using TPoint = TImage::IndexType;
    using ImageType = TImage;
    using ReaderType = itk::ImageSeriesReader<ImageType>;
    using ImageIOType = itk::GDCMImageIO;
    using SeriesFileNamesType = itk::GDCMSeriesFileNames;
    using FileNamesContainer = std::vector<std::string>;
    using SeriesIdContainer = std::vector<std::string>;
    using SeriesIdContainer = std::vector<std::string>;

    ImageIOType::Pointer m_gdcmImageIO;
    SeriesFileNamesType::Pointer m_gdcmSeriesFileNames = SeriesFileNamesType::New();
    ReaderType::Pointer m_gdcmReader;
    m_gdcmReader = ReaderType::New();
    m_gdcmImageIO = ImageIOType::New();

    m_gdcmSeriesFileNames->SetUseSeriesDetails(true);
    m_gdcmSeriesFileNames->SetDirectory(tempDir.path().toStdString().c_str());
    const SeriesIdContainer& seriesUID = m_gdcmSeriesFileNames->GetSeriesUIDs();
    FileNamesContainer fileNames = m_gdcmSeriesFileNames->GetFileNames(seriesUID[0]);
    m_gdcmReader->SetImageIO(m_gdcmImageIO);
    m_gdcmReader->SetFileNames(fileNames);
    m_gdcmReader->ForceOrthogonalDirectionOff();
    try {
        m_gdcmReader->Update();
        mitk::Image::Pointer mitkImage = mitk::Image::New();
        mitk::GrabItkImageMemory(m_gdcmReader->GetOutput(), mitkImage);
        mitk::DataNode::Pointer cbctNode = mitk::DataNode::New();
        cbctNode->SetData(mitkImage);
        cbctNode->SetName("teeth");
        m_dataStorage->Add(cbctNode);
        vtkSmartPointer<vtkSmartVolumeMapper> volumeMapper =
            vtkSmartPointer<vtkSmartVolumeMapper>::New();
        vtkSmartPointer<vtkImageData> image = vtkSmartPointer<vtkImageData>::New();
        image->DeepCopy(mitkImage->GetVtkImageData());
        auto origin = cbctNode->GetData()->GetGeometry()->GetOrigin();
        image->SetOrigin(origin[0],origin[1],origin[2]);
        volumeMapper->SetInputData(image);

        vtkSmartPointer<vtkVolumeProperty> volumeProperty =
            vtkSmartPointer<vtkVolumeProperty>::New();
        volumeProperty->SetInterpolationTypeToLinear();
        volumeProperty->ShadeOn(); 
        volumeProperty->SetAmbient(0.4);
        volumeProperty->SetDiffuse(0.6); 
        volumeProperty->SetSpecular(0.2);

        vtkSmartPointer<vtkPiecewiseFunction> compositeOpacity =
            vtkSmartPointer<vtkPiecewiseFunction>::New();
        compositeOpacity->AddPoint(70, 0.00);
        compositeOpacity->AddPoint(90, 0.40);
        compositeOpacity->AddPoint(180, 0.60);
        volumeProperty->SetScalarOpacity(compositeOpacity);

        vtkSmartPointer<vtkPiecewiseFunction> volumeGradientOpacity =
            vtkSmartPointer<vtkPiecewiseFunction>::New();
        volumeGradientOpacity->AddPoint(10, 0.0);
        volumeGradientOpacity->AddPoint(90, 0.5);
        volumeGradientOpacity->AddPoint(100, 1.0);
        volumeProperty->SetGradientOpacity(volumeGradientOpacity);

        vtkSmartPointer<vtkColorTransferFunction> color =
            vtkSmartPointer<vtkColorTransferFunction>::New();
        color->AddRGBPoint(150.000, 150.00/ 255.0, 0.00, 0.00);
        color->AddRGBPoint(240.0, 235.00/ 255.0, 0.00, 0.00);
        color->AddRGBPoint(300.0, 1.0, 203.00/ 255.0, 15.00/ 255.0);
        color->AddRGBPoint(500.0, 1.0, 230/255.0,170/255.0);
        color->AddRGBPoint(680.0, 1.0, 1.0, 54.0/255);
        color->AddRGBPoint(780.0, 1.0, 1.0, 1.0);
        volumeProperty->SetColor(color);
        vtkSmartPointer<vtkVolume> volume =
            vtkSmartPointer<vtkVolume>::New();
        volume->SetMapper(volumeMapper);
        volume->SetProperty(volumeProperty);

        vtkRenderer* ren = m_stdMultiWidget->getRenderWindow(1)->GetRenderer()->GetVtkRenderer();
        ren->AddVolume(volume);
        m_stdMultiWidget->getRenderWindow(1)->renderWindow()->Render();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return;
    }
    for (int i = 0; i < 5; ++i)
        mitk::RenderingManager::GetInstance()->InitializeViewByBoundingObjects(m_stdMultiWidget->getRenderWindow(i)->GetVtkRenderWindow(), m_dataStorage);
    tempDir.remove();
}

void MainWindow::on_pushButton_loadSTL_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "please select a .stl file", ".", "*.stl");
    addSTLFile(filePath.toStdString(), filePath.toStdString());
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
    QString name = "targetPoint"+QString::number(m_pointIndex);
    mitk::DataNode *node=m_dataStorage->GetNamedNode(name.toStdString());
    if(node)
    {
        mitk::PointSet* ps = static_cast<mitk::PointSet*>(node->GetData());
        if(ps->GetSize()<2)
        {
            m_interactor->SetDataNode(node);
            return;
        }
    }
    ++m_pointIndex;
    name = "targetPoint" + QString::number(m_pointIndex);
    mitk::DataNode::Pointer targetNode = mitk::DataNode::New();
    targetNode->SetName(name.toStdString());
    mitk::PointSet::Pointer ps = mitk::PointSet::New();
    targetNode->SetData(ps);
    targetNode->SetMapper(mitk::BaseRenderer::Standard3D,PointSetVtkMapper3D::New());
    targetNode->SetProperty("show contour", mitk::BoolProperty::New(true));
    targetNode->SetProperty("show distances", mitk::BoolProperty::New(true));
    targetNode->SetProperty("contoursize", mitk::FloatProperty::New(5));
    if(!m_interactor)
    {
        m_interactor = PointSetDataNodeInteractor::New();
        m_interactor->SetMaxPoints(2);
        try {
            m_interactor->LoadStateMachine("PointSet.xml");
            m_interactor->SetEventConfig("PointSetConfig.xml");
        }
        catch (const mitk::Exception& e) {
            qDebug() << QString::fromStdString(e.GetDescription());
        }
    }
    m_interactor->SetDataNode(targetNode);
    m_dataStorage->Add(targetNode);
    m_dataStorage3D->Add(targetNode);
}

void MainWindow::on_pushButton_extractISO_clicked()
{
    mitk::Image*image=m_dataStorage->GetNamedObject<mitk::Image>("teeth");
    if (!image)
        return;
    mitk::Surface* teethModel = m_dataStorage->GetNamedObject<mitk::Surface>(m_stlName[0]);
    if (!teethModel)
        return;
    vtkSmartPointer<vtkImageData> img = vtkSmartPointer<vtkImageData>::New();
    img->DeepCopy(image->GetVtkImageData());
    img->SetOrigin(image->GetGeometry()->GetOrigin().data());
    vtkSmartPointer<vtkImageThreshold> threshold = vtkSmartPointer<vtkImageThreshold>::New();
    threshold->SetInputData(img);
    threshold->ThresholdByLower(3000);
    threshold->ThresholdByUpper(2400);
    threshold->ReplaceInOn();
    threshold->SetInValue(1);
    threshold->ReplaceOutOn();
    threshold->SetOutValue(0);
    threshold->Update();
    vtkSmartPointer<vtkDiscreteFlyingEdges3D> flying = vtkSmartPointer<vtkDiscreteFlyingEdges3D>::New();
    flying->SetInputData(threshold->GetOutput());
    flying->SetNumberOfContours(1);
    flying->SetValue(0,1);
    flying->SetComputeGradients(false);
    flying->SetComputeNormals(false);
    flying->SetComputeScalars(true);
    flying->Update();

    vtkSmartPointer<vtkTransformPolyDataFilter> filter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
    vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
    transform->SetMatrix(teethModel->GetGeometry()->GetVtkMatrix());
    filter->SetTransform(transform);
    filter->SetInputData(teethModel->GetVtkPolyData());
    filter->Update();
    vtkSmartPointer<vtkIterativeClosestPointTransform> icp =
        vtkSmartPointer<vtkIterativeClosestPointTransform>::New();
    icp->SetSource(filter->GetOutput());
    icp->SetTarget(flying->GetOutput());
    icp->GetLandmarkTransform()->SetModeToRigidBody();
    icp->SetMaximumNumberOfIterations(50);
    icp->StartByMatchingCentroidsOn();
    icp->Modified();
    icp->Update();
    vtkMatrix4x4 *vmt=icp->GetMatrix();
    vtkSmartPointer<vtkMatrix4x4> result = vtkSmartPointer<vtkMatrix4x4>::New();
    vtkMatrix4x4::Multiply4x4(vmt, teethModel->GetGeometry()->GetVtkMatrix(), result);
    teethModel->GetGeometry()->SetIndexToWorldTransformByVtkMatrix(result);
    mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(m_dataStorage);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void MainWindow::on_pushButton_modelMark_clicked()
{
    std::string name = "modelmark";
    if(m_dataStorage3D->GetNamedNode(name.c_str()))
    {
        m_dataStorage3D->Remove(m_dataStorage3D->GetNamedNode(name.c_str()));
    }
    mitk::DataNode::Pointer dt = mitk::DataNode::New();
    dt->SetName(name.c_str());
    mitk::PointSet::Pointer ps = mitk::PointSet::New();
    dt->SetData(ps);
    mitk::PointSetDataInteractor::Pointer interactor = mitk::PointSetDataInteractor::New();
    interactor->LoadStateMachine("PointSet.xml");
    interactor->SetEventConfig("PointSetConfig.xml");
    interactor->SetMaxPoints(5);
    interactor->SetDataNode(dt);
    m_dataStorage3D->Add(dt);
}

void MainWindow::on_pushButton_CTMark_clicked()
{
    std::string name = "ctmark";
    if (m_dataStorage->GetNamedNode(name.c_str()))
    {
        m_dataStorage->Remove(m_dataStorage->GetNamedNode(name.c_str()));
    }
    mitk::DataNode::Pointer dt = mitk::DataNode::New();
    dt->SetName(name.c_str());
    mitk::PointSet::Pointer ps = mitk::PointSet::New();
    dt->SetData(ps);
    mitk::PointSetDataInteractor::Pointer interactor = mitk::PointSetDataInteractor::New();
    interactor->LoadStateMachine("PointSet.xml");
    interactor->SetEventConfig("PointSetConfig.xml");
    interactor->SetMaxPoints(5);
    interactor->SetDataNode(dt);
    m_dataStorage->Add(dt);
}

void MainWindow::on_pushButton_roughRegister_clicked()
{
    vtkSmartPointer<vtkPoints> sourcePoints = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkPoints> targetPoints = vtkSmartPointer<vtkPoints>::New();
    auto sourceps = m_dataStorage3D->GetNamedObject<mitk::PointSet>("modelmark");
    auto targetps = m_dataStorage->GetNamedObject<mitk::PointSet>("ctmark");
    if (sourceps->GetSize() < 5 || targetps->GetSize() < 5)
        return;
    for(int i=0;i<5;++i)
    {
        sourcePoints->InsertNextPoint(sourceps->GetPoint(i)[0], sourceps->GetPoint(i)[1], sourceps->GetPoint(i)[2]);
        targetPoints->InsertNextPoint(targetps->GetPoint(i)[0], targetps->GetPoint(i)[1], targetps->GetPoint(i)[2]);
    }
    vtkNew<vtkLandmarkTransform> landmarkTransform;
    landmarkTransform->SetSourceLandmarks(sourcePoints);
    landmarkTransform->SetTargetLandmarks(targetPoints);
    landmarkTransform->SetModeToRigidBody();
    landmarkTransform->Update();
    vtkMatrix4x4* vmt = landmarkTransform->GetMatrix();
    mitk::Surface* teethModel = m_dataStorage->GetNamedObject<mitk::Surface>(m_stlName[0]);
    if (!teethModel)
        return;
    teethModel->GetGeometry()->SetIndexToWorldTransformByVtkMatrix(vmt);
    mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(m_dataStorage);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

