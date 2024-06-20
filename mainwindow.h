#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <mitkStandaloneDataStorage.h>
#include <mitkPointSet.h>
#include <vtkSmartPointer.h>

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
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_loadCBCT_clicked();
    void on_pushButton_loadSTL_clicked();
    void on_pushButton_cpr_clicked();
    void on_pushButton_selectPoint_clicked();
private:
    Ui::MainWindow *ui;
    ThreeSliceWidget*m_stdMultiWidget;
    mitk::StandaloneDataStorage::Pointer m_dataStorage;
    mitk::StandaloneDataStorage::Pointer m_dataStorage3D;
    QmitkRenderWindow *m_cprRenderWindow;
    PanoramaView* m_panoramaView{nullptr};
};
#endif // MAINWINDOW_H
