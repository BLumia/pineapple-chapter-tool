/*
    SPDX-FileCopyrightText: 2021 Gary Wang <wzc782970009@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only
*/

#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include "chaptertreemodel.h"
#include "filehandlermanager.h"
#include "filehandlerinterface.h"
#include "chapterinfowidget.h"

#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QMimeData>
#include <QMouseEvent>
#include <QRegularExpression>
#include <QTime>

#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Due to we have action that doesn't really match to a freedesktop icon theme
    // action type, using theme icon will cause style doesn't match. Before we figure
    // out some better solution, we use action icons inside the resource file...
//    ui->actionOpen->setIcon(qApp->style()->standardIcon(QStyle::SP_DialogOpenButton));
//    ui->actionSave->setIcon(qApp->style()->standardIcon(QStyle::SP_DialogSaveButton));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loadFile(QUrl url)
{
    QString filePath(url.toLocalFile());
    if (QFile::exists(filePath)) {
        m_filePath = filePath;
    }

    loadFile();
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
            loadFile(urls.first());
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

        ui->actionApply->setEnabled(true);
    } else {
        QMessageBox::information(
                    this, tr("Load failed"),
                    tr("Unsupported file type.\nThe supported file types are: %1.")
                    .arg("mp3, ogg, opus"));
    }
}

QTime timeFromString(const QString & timeStr)
{
    switch (timeStr.count(':')) {
    case 0:
        return QTime::fromString(timeStr, "s");
    case 1:
        return QTime::fromString(timeStr, "m:s");
    default:
        return QTime::fromString(timeStr, "h:m:s");
    }
}

// We use a very flexible rule similar to the one used on YouTube
// https://support.google.com/youtube/answer/9884579
// YouTube require the first chapter marker as 0:00, and a chapter line
// must start with a chapter marker, we don't require this.
void MainWindow::importFromLines(ChapterTreeModel * model, const QStringList &lines)
{
    Q_CHECK_PTR(model);

    model->clearChapterTreeButKeepTOC();
    // This regex see: https://stackoverflow.com/questions/8318236/
    QRegularExpression timeRegex(QStringLiteral(R"((?:([01]?\d|2[0-3]):)?([0-5]?\d):([0-5]?\d))"));
    for (QString line : lines) {
        QRegularExpressionMatch match(timeRegex.match(line));
        qDebug() << match.capturedTexts() << match.hasMatch();
        if (match.hasMatch()) {
            const QString timeStr(match.capturedTexts()[0]);
            line.remove(timeStr);
            line = line.trimmed();
            QTime startTime(timeFromString(timeStr));
            qDebug() << startTime << startTime.msecsSinceStartOfDay();
            model->appendChapter(startTime.msecsSinceStartOfDay(), line);
        }
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


void MainWindow::on_actionApply_triggered()
{
    ChapterTreeModel * chapterModel = ui->treeView->model();
    QFileInfo currentFile(m_filePath);

    if (!chapterModel) return;
    if (!currentFile.exists()) return;

    QString filePath = QFileDialog::getSaveFileName(this, tr("Save Audio File with Chapters"),
                                    currentFile.absolutePath(),
                                    tr("Audio Files (*.mp3 *.ogg *.opus, *.m4a, *.cue, *.info.json)"));

    chapterModel->saveToFile(filePath);
}

void MainWindow::on_actionExport_triggered()
{
    ChapterTreeModel * chapterModel = ui->treeView->model();
    QFileInfo currentFile(m_filePath);
    QString defaultSavePath;
    QStringList filters; // Copywritings
    QMap<QString, QString> filterExporterMap; // <Filter Copywriting, Suffix>

    QList<QPair<QString, QString> > exporters = FileHandlerManager::instance()->exporterList();
    for (const QPair<QString, QString> & exporter : qAsConst(exporters)) {
        filterExporterMap[exporter.second] = exporter.first;
        filters.append(exporter.second);
    }

    if (!chapterModel) return;
    defaultSavePath = currentFile.exists() ? currentFile.absolutePath() : QDir::homePath();

    const QStringList schemes = QStringList(QStringLiteral("file"));
    QFileDialog dlg(this, tr("Export Chapter Data to File"), defaultSavePath,
                    filters.join(QLatin1String(";;")));
    dlg.setSupportedSchemes(schemes);
    dlg.setAcceptMode(QFileDialog::AcceptSave);
    if (dlg.exec() == QDialog::Accepted) {
        QString selectedFilter = dlg.selectedNameFilter();
        if (filterExporterMap.contains(selectedFilter)) {
            QString filePath(dlg.selectedUrls().value(0).toLocalFile());
            chapterModel->exportToFile(filePath, filterExporterMap[selectedFilter]);
        }
    }
}

void MainWindow::on_treeView_viewSelectionChanged()
{
    bool canDelete = false;

    QModelIndexList selectedIndex(ui->treeView->selectionModel()->selectedIndexes());
    if (!selectedIndex.isEmpty()) {
        canDelete = true;
        ui->chapterInfoWidget->setCurrentChapter(ui->treeView->model(), selectedIndex.first());
    }

    ui->removeBtn->setEnabled(canDelete);
}

void MainWindow::on_appendChapterBtn_clicked()
{
    ChapterTreeModel * model = ui->treeView->model();
    if (model) {
        model->appendChapter(ui->treeView->selectionModel());
    }
}

void MainWindow::on_removeBtn_clicked()
{
    QItemSelectionModel * selectionModel = ui->treeView->selectionModel();
    if (!selectionModel) return;

    QModelIndexList selectedIndex(ui->treeView->selectionModel()->selectedIndexes());
    if (!selectedIndex.isEmpty()) {
        ui->treeView->model()->removeRow(selectedIndex[0].row(), selectedIndex[0].parent());
    }
}


void MainWindow::on_importBtn_clicked()
{
    ChapterTreeModel * model = ui->treeView->model();
    if (model) {
        QString text = QInputDialog::getMultiLineText(this, tr("Single line chapters"), tr("Sample:\n1:23   Chapter 1 title\n1:23:45 Chapter 2 title"));
        if (!text.isEmpty()) {
            importFromLines(model, text.split('\n'));
        }
    }
    //
}

