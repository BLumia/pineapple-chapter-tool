#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include "chaptertreemodel.h"

//#include <fileref.h>
#include <mpegfile.h>
#include <id3v2tag.h>
#include <chapterframe.h>
#include <tbytevectorlist.h> // since tableofcontentsframe is missing the declaration of ByteVectorList...
#include <tableofcontentsframe.h>
#include <textidentificationframe.h>
#include <urllinkframe.h>
#include <attachedpictureframe.h>

#include <QMimeData>
#include <QMouseEvent>

#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_filePath(R"(C:\Users\Gary\Downloads\auphonic_chapters_demo.mp3)")
{
    ui->setupUi(this);

    ui->actionSave->setIcon(qApp->style()->standardIcon(QStyle::SP_DialogSaveButton));

    loadFile();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    } else {
        event->ignore();
    }

    return QMainWindow::dragEnterEvent(event);
}

void MainWindow::dropEvent(QDropEvent *event)
{
    event->acceptProposedAction();

    const QMimeData * mimeData = event->mimeData();

    if (mimeData->hasUrls()) {
        const QList<QUrl> &urls = mimeData->urls();
        if (!urls.isEmpty()) {
            m_filePath = urls.first().toLocalFile();
            loadFile();
        }
    }
}

void MainWindow::loadFile()
{
    ChapterTreeModel * model = new ChapterTreeModel;
    model->loadFromFile(m_filePath);
    ui->treeView->setModel(model);
    ui->treeView->expandAll();
    ui->treeView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
}


