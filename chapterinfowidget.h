#pragma once

#include <QModelIndex>
#include <QWidget>

namespace Ui {
class ChapterInfoWidget;
}

class ChapterTreeModel;
class ChapterInfoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ChapterInfoWidget(QWidget *parent = nullptr);
    ~ChapterInfoWidget();

    void setCurrentChapter(ChapterTreeModel * model, const QModelIndex & selectedIndex);

private:
    Ui::ChapterInfoWidget *ui;

    ChapterTreeModel * m_model = nullptr; // this class doesn't manage this pointer's lifecycle.
    QModelIndex m_currentIndex;
};

