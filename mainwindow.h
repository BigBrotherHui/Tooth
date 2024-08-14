#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <mitkStandaloneDataStorage.h>
#include <mitkPointSet.h>
#include <vtkSmartPointer.h>
#include "PointSetDataNodeInteractor.h"
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE
class QmitkStdMultiWidget;
class QmitkRenderWindow;
class ThreeSliceWidget;
class PanoramaView;
class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();
    //�����򳡾������stl�ļ��ڵ�
    //filePath:�ļ�·�� objectName:�ڵ�����
    void addSTLFile(const std::string& filePath, const std::string objectName);
    //����objectName��Ӧ�Ľڵ����̬
    void setMatrix(const std::string& objectName, vtkMatrix4x4* vmt);
    //��ȡobjectName��Ӧ�Ľڵ����̬
    vtkMatrix4x4* getMatrix(const std::string& objectName);
    //��ȡԲ����2��λ��
    std::vector<std::array<std::array<double, 3>, 2> > getPointPairPosition();
private slots:
    void on_pushButton_loadCBCT_clicked();
    void on_pushButton_loadSTL_clicked();
    void on_pushButton_cpr_clicked();
    void on_pushButton_selectPoint_clicked();
    void on_pushButton_extractISO_clicked();
    void on_pushButton_modelMark_clicked();
    void on_pushButton_CTMark_clicked();
    void on_pushButton_roughRegister_clicked();
    void on_pushButton_reset_clicked();
protected:
    void SlotPointMarkFinished();
    void OnFollowedGeometryModified();
private:
    Ui::MainWindow* ui;
    ThreeSliceWidget* m_stdMultiWidget;
    mitk::StandaloneDataStorage::Pointer m_dataStorage;
    mitk::StandaloneDataStorage::Pointer m_dataStorage3D;
    QmitkRenderWindow* m_cprRenderWindow;
    PanoramaView* m_panoramaView{ nullptr };
    std::string m_stlName[2]{ "teethModel","tool" };
    int m_stlIndex{ 0 };
    int m_pointIndex{ 0 };
    PointSetDataNodeInteractor::Pointer m_interactor{nullptr};
    mitk::DataNode* m_activeDataNode{ nullptr };
    int m_maxPoints{ 0 };
    vtkSmartPointer<vtkMatrix4x4> m_tool2teethModel = vtkSmartPointer<vtkMatrix4x4>::New();
    QMap<std::string, std::string> m_objectNameMap;
};
#endif // MAINWINDOW_H
