#ifndef THREESLICEWIDGET_H
#define THREESLICEWIDGET_H

#include <mitkStandaloneDataStorage.h>

#include <QGridLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <mitkDisplayActionEventBroadcast.h>
#include <mitkDisplayActionEventHandler.h>

class vtkCamera;
namespace Ui {
class ThreeSliceWidget;
}
class QmitkRenderWindow;
class ThreeSliceWidget : public QWidget {
    Q_OBJECT
    enum DIRECTION { AXIAL = 0, SAGITTAL, FRONTAL };

public:
    explicit ThreeSliceWidget(QWidget *parent = nullptr, bool hLayout = true);
    ~ThreeSliceWidget();
    void setDataStorage(mitk::StandaloneDataStorage::Pointer ds);
    void setDataStorage3D(mitk::StandaloneDataStorage::Pointer ds);
    QmitkRenderWindow* getRenderWindow(int index);
protected:
    void initInteractor();
    mitk::StdFunctionCommand::ActionFunction SetCrosshairSynchronizedAction();
private:
    Ui::ThreeSliceWidget *ui;
    mitk::StandaloneDataStorage::Pointer m_dataStorage{nullptr};
    mitk::StandaloneDataStorage::Pointer m_dataStorage3D{ nullptr };
    mitk::DataNode::Pointer m_PlaneNode1;
    mitk::DataNode::Pointer m_PlaneNode2;
    mitk::DataNode::Pointer m_PlaneNode3;

    QmitkRenderWindow* m_renderwindows[5];

    mitk::DisplayActionEventBroadcast::Pointer m_DisplayActionEventBroadcast{nullptr};
    std::unique_ptr<mitk::DisplayActionEventHandler> m_DisplayActionEventHandler{nullptr};
};

#endif  // THREESLICEWIDGET_H
