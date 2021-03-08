#include "chapterinfowidget.h"
#include "ui_chapterinfowidget.h"

#include "chaptertreemodel.h"

#include <QLineEdit>
#include <QTimeEdit>

ChapterInfoWidget::ChapterInfoWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChapterInfoWidget)
{
    ui->setupUi(this);
}

ChapterInfoWidget::~ChapterInfoWidget()
{
    delete ui;
}

void ChapterInfoWidget::setCurrentChapter(ChapterTreeModel *model, const QModelIndex &selectedIndex)
{
    m_model = model;
    m_currentIndex = selectedIndex;

    ui->chapterInfoTable->clear();

    ChapterItem * currentItem = static_cast<ChapterItem *>(model->itemFromIndex(m_currentIndex));
    if (currentItem->hasChildren()) {
        return;
    }

    ui->chapterInfoTable->setItem(0, 0, new QTableWidgetItem(tr("Title:")));
    auto titleEdit = new QLineEdit(currentItem->data(ChapterTitle).toString());
    connect(titleEdit, &QLineEdit::textChanged, [currentItem](const QString &text){
        currentItem->setItemProperty(ChapterTitle, text);
    });
    ui->chapterInfoTable->setCellWidget(0, 1, titleEdit);
    ui->chapterInfoTable->setItem(1, 0, new QTableWidgetItem(tr("Start Time:")));
    auto startTimeWidget = new QTimeEdit;
    startTimeWidget->setDisplayFormat("hh:mm:ss.zzz");
    startTimeWidget->setTime(QTime::fromMSecsSinceStartOfDay(currentItem->data(ChapterStartTimeMs).toInt()));
    connect(startTimeWidget, &QTimeEdit::timeChanged, [currentItem](const QTime &time){
        currentItem->setItemProperty(ChapterStartTimeMs, time.msecsSinceStartOfDay());
    });
    ui->chapterInfoTable->setCellWidget(1, 1, startTimeWidget);

    QVariant endTime(currentItem->data(ChapterEndTimeMs));
    if (endTime.isNull()) {
        ui->chapterInfoTable->setRowCount(2);
    } else {
        ui->chapterInfoTable->setRowCount(3);
        ui->chapterInfoTable->setItem(2, 0, new QTableWidgetItem(tr("End Time:")));
        auto endTimeWidget = new QTimeEdit;
        endTimeWidget->setDisplayFormat("hh:mm:ss.zzz");
        endTimeWidget->setTime(QTime::fromMSecsSinceStartOfDay(currentItem->data(ChapterEndTimeMs).toInt()));
        connect(endTimeWidget, &QTimeEdit::timeChanged, [endTimeWidget, currentItem](const QTime &time){
            if (currentItem->data(ChapterStartTimeMs).toInt() > time.msecsSinceStartOfDay()) {
                endTimeWidget->setTime(QTime::fromMSecsSinceStartOfDay(currentItem->data(ChapterEndTimeMs).toInt()));
                return;
            }
            currentItem->setItemProperty(ChapterEndTimeMs, time.msecsSinceStartOfDay());
        });
        ui->chapterInfoTable->setCellWidget(2, 1, endTimeWidget);
    }
}
