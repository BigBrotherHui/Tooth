#include "PointSelectionWindowInterface.h"
#include "mainwindow.h"
#include <vtkOutputWindow.h>
#include <QmitkRegisterClasses.h>
#include <QApplication>
class MyApplication : public QApplication
{
public:
	MyApplication(int &argc,char **argv) : QApplication(argc,argv)
	{
		
	}
protected:
	bool MyApplication::notify(QObject* receiver, QEvent* event) override{
		try {
			return QApplication::notify(receiver, event);
		}
		catch (const std::exception& e) {
			std::cout << "Exception caught in notify:" << e.what();
			return false;
		}
	}
};

class PointSelectionWindowInterface::Impl
{
public:
	Impl();
	~Impl();
	MainWindow* m_mainWindow{nullptr};
	MyApplication* m_application{nullptr};
};

PointSelectionWindowInterface::Impl::Impl()
{
	QmitkRegisterClasses();
	vtkOutputWindow::GlobalWarningDisplayOff();
	int argc=0;
	m_application = new MyApplication(argc,{});
	m_mainWindow = new MainWindow;
}

PointSelectionWindowInterface::Impl::~Impl()
{
	if(m_mainWindow)
	{
		delete m_mainWindow;
		m_mainWindow = nullptr;
	}
	if(m_application)
	{
		delete m_application;
		m_application = nullptr;
	}
}

PointSelectionWindowInterface::PointSelectionWindowInterface() : m_impl(new Impl)
{
	
}

PointSelectionWindowInterface::~PointSelectionWindowInterface()
{
	if(m_impl)
	{
		delete m_impl;
		m_impl = nullptr;
	}
}

void PointSelectionWindowInterface::OpenWindow()
{
	m_impl->m_mainWindow->show();
	m_impl->m_application->exec();
}

void PointSelectionWindowInterface::addSTLFile(const std::string& filePath, const std::string objectName)
{
	m_impl->m_mainWindow->addSTLFile(filePath, objectName);
}

void PointSelectionWindowInterface::setMatrix(const std::string& objectName, double matrix[16])
{
	if (!matrix)
		return;
	vtkSmartPointer<vtkMatrix4x4> vmt = vtkSmartPointer<vtkMatrix4x4>::New();
	vmt->DeepCopy(matrix);
	m_impl->m_mainWindow->setMatrix(objectName, vmt);
}

bool PointSelectionWindowInterface::getMatrix(const std::string& objectName, double** matrix)
{
	if (!matrix)
		return false;
	vtkMatrix4x4* vmt = m_impl->m_mainWindow->getMatrix(objectName);
	if (!vmt)
		return false;
	*matrix = new double[16];
	memcpy(*matrix, vmt->GetData(), sizeof(double) * 16);
	return true;
}

std::vector<std::array<std::array<double, 3>, 2> > PointSelectionWindowInterface::getPointPairPosition()
{
	return m_impl->m_mainWindow->getPointPairPosition();
}
