#pragma once

#include <QMainWindow>

#include <vector>

namespace Ui {
class CUiInspector;
}

class CUiInspector : public QMainWindow
{
public:
	explicit CUiInspector(QWidget *parent = nullptr);
	~CUiInspector();

private:
	void inspect();
	void visualize(const std::vector<struct WidgetHierarchy>& hierarchy);

	void inspectWidgetHierarchy(QWidget* widget, std::vector<struct WidgetHierarchy>& root) const;
	void inspectWidgetHierarchy(QLayout* layout, std::vector<struct WidgetHierarchy>& root) const;

private:
	Ui::CUiInspector *ui;

	QStringList _ignoredClasses;
};

