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

#include <QFileDialog>
#include <QMessageBox>
#include <QMimeData>
#include <QMouseEvent>

#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_filePath(R"(C:\Users\Gary\Downloads\auphonic_chapters_demo.mp3)")
{
    ui->setupUi(this);

    ui->actionOpen->setIcon(qApp->style()->standardIcon(QStyle::SP_DialogOpenButton));
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
    QFileInfo fileInfo(m_filePath);
    if (!fileInfo.exists()) return;

    ChapterTreeModel * model = new ChapterTreeModel;
    bool succ = model->loadFromFile(m_filePath);
    if (succ) {
        QAbstractItemModel * oldModel = ui->treeView->model();

        ui->treeView->setModel(model);
        ui->treeView->expandAll();
        ui->treeView->header()->setSectionResizeMode(0, QHeaderView::Stretch);

        if (oldModel) {
            delete oldModel;
        }

        QFileInfo fileInfo(m_filePath);
        setWindowTitle(fileInfo.fileName());

        ui->actionSave->setEnabled(true);
    } else {
        QMessageBox::information(
                    this, tr("Load failed"),
                    tr("Unsupported file type.\nThe supported file types are: %1.")
                    .arg("mp3, ogg, opus"));
    }
}

void MainWindow::on_actionOpen_triggered()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Open Audio File"),
                                                    QDir::homePath(),
                                                    tr("Audio Files (*.mp3 *.ogg *.opus)"));

    if (!filePath.isEmpty()) {
        m_filePath = filePath;
        loadFile();
    }
}


void MainWindow::on_actionSave_triggered()
{
    QAbstractItemModel * currentModel = ui->treeView->model();
    ChapterTreeModel * chapterModel = qobject_cast<ChapterTreeModel *>(currentModel);
    QFileInfo currentFile(m_filePath);

    if (!chapterModel) return;
    if (!currentFile.exists()) return;

    QString filePath = QFileDialog::getSaveFileName(this, tr("Save Audio File with Chapters"),
                                                    currentFile.absolutePath(),
                                                    tr("Audio Files (*.mp3 *.ogg *.opus)"));

    chapterModel->saveToFile(filePath);
}

void MainWindow::on_treeView_viewSelectionChanged()
{
    bool canDelete = false;

    QModelIndexList selectedIndex(ui->treeView->selectionModel()->selectedIndexes());
    if (!selectedIndex.isEmpty()) {
        canDelete = true;
    }

    ui->removeBtn->setEnabled(canDelete);
}

void MainWindow::on_appendChapterBtn_clicked()
{
    QModelIndexList selectedIndex(ui->treeView->selectionModel()->selectedIndexes());

    ChapterTreeModel * model = qobject_cast<ChapterTreeModel *>(ui->treeView->model());
    if (model) {
        model->appendChapter(selectedIndex);
    }
}

void MainWindow::on_removeBtn_clicked()
{
    QModelIndexList selectedIndex(ui->treeView->selectionModel()->selectedIndexes());
    if (!selectedIndex.isEmpty()) {
        ui->treeView->model()->removeRow(selectedIndex[0].row(), selectedIndex[0].parent());
    }
}

