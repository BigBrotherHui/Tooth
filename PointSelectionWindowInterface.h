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
    //�����򳡾������stl�ļ��ڵ�
    //filePath:�ļ�·�� objectName:�ڵ�����
	void addSTLFile(const std::string& filePath, const std::string objectName);
    //����objectName��Ӧ�Ľڵ����̬
    void setMatrix(const std::string& objectName, double matrix[16]);
    //��ȡobjectName��Ӧ�Ľڵ����̬
    bool getMatrix(const std::string& objectName,double **matrix);
    //��ȡԲ����2��λ��
    std::vector<std::array<std::array<double, 3>, 2> > getPointPairPosition();
private:
    class Impl;
    Impl* m_impl{nullptr};
};
