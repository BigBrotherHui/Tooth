/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef PanoramaView_h
#define PanoramaView_h

#include "CBCTPanorama.h"
#include "mitkGraphcutSegmentationToSurfaceFilter.h"
#include "qstackedwidget.h"
#include "ui_PanoramaViewControls.h"
#include <QString>
#include <QmitkIOUtil.h>
#include <QmitkRenderWindow.h>
#include <mitkDataNode.h>
#include <mitkITKImageImport.h>
#include <mitkImageToItk.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>
#include <mitkPlanarFigure.h>
#include <mitkPointSet.h>
#include <mitkPointSetDataInteractor.h>

class PanoramaView : public QWidget
{
  Q_OBJECT

public:
  PanoramaView(QWidget *parent=nullptr);
  ~PanoramaView();
  void setMitkRenderWindow(QmitkRenderWindow *renderWindow);
private:
  void CreateQtPartControl();
  void OnProcessMip();
  void OnProcessOtsu();
  void OnProcessMorph();
  void OnProcessSpline();
  void OnProcessArch();
  void OnProcessPanorama();
  void RemoveNode(std::string nodeName);
  void OnSetLowerSlice();
  void OnSetHigherSlice();

  //重新设置参数
  void OnResetMip();
  void OnResetMorph();
  void OnResetArch();
  void OnResetPanorama();
  void OnResetLooseROI();

  void ResetViewBasicFunc(std::vector<std::string> show, std::vector<std::string> hide, std::string locate);
  Panorama *panorama;
  Ui::PanoramaViewControls *m_Controls;
  mitk::DataNode::Pointer m_PointSetNode;
  mitk::DataInteractor::Pointer m_DataInteractor;
  mitk::DataStorage::Pointer m_dataStorage;
  QmitkRenderWindow* m_RenderWindow;
  mitk::PointSet::Pointer m_PointSet;
  std::set<mitk::SliceNavigationController *> m_Sncs;
  //保存的样条曲线拟合点
  std::vector<Panorama::Point> result;
  //拟合点---每个拟合点的插值点
  std::vector<std::vector<Panorama::Point>> interpoMap;

  //节点颜色
  const float RED[3] = {255.0, 0, 0};
  const float GREEN[3] = {0, 255.0, 0};
  const float BLUE[3] = {0, 0, 255.0};
  const float GRAY[3] = {127.0, 127.0, 127.0};
};
#endif
