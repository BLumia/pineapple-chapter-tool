/*
    SPDX-FileCopyrightText: 2021 Gary Wang <wzc782970009@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only
*/

#pragma once

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class ChapterTreeModel;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void loadFile(QUrl url);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    void on_actionOpen_triggered();
    void on_actionApply_triggered();
    void on_actionExport_triggered();

    void on_treeView_viewSelectionChanged();

    void on_appendChapterBtn_clicked();
    void on_removeBtn_clicked();

    void on_importBtn_clicked();

private:
    void loadFile();
    void importFromLines(ChapterTreeModel *model, const QStringList &lines);

    void uiAdjustHeaderSelectionResizeMode();

private:
    Ui::MainWindow *ui;

    QString m_filePath;
};
