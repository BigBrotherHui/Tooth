#pragma once

#ifdef POINTSELECTIONWINDOWINTERFACE_EXPORTS
#define POINTSELECTIONWINDOWINTERFACE_API __declspec(dllexport)
#else 
#define POINTSELECTIONWINDOWINTERFACE_API __declspec(dllimport)
#endif
#include <string>
#include <vector>

class POINTSELECTIONWINDOWINTERFACE_API PointSelectionWindowInterface
{
public:
    PointSelectionWindowInterface();
    ~PointSelectionWindowInterface();
	void OpenWindow();
    //用于向场景中添加stl文件节点
    //filePath:文件路径 objectName:节点名字
	void addSTLFile(const std::string& filePath, const std::string objectName);
    //设置objectName对应的节点的姿态
    void setMatrix(const std::string& objectName, double matrix[16]);
    //获取objectName对应的节点的姿态
    bool getMatrix(const std::string& objectName,double **matrix);
    //获取圆柱的2点位置
    std::vector<std::array<std::array<double, 3>, 2> > getPointPairPosition();
private:
    class Impl;
    Impl* m_impl{nullptr};
};
