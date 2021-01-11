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

#include <QDebug>

// spec: https://id3.org/id3v2-chapters-1.0

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QString m_audioFile(QStringLiteral(R"(C:\Users\Gary\Downloads\auphonic_chapters_demo.mp3)"));

    ChapterTreeModel * model = new ChapterTreeModel;
    model->loadFromFile(m_audioFile);
    ui->treeView->setModel(model);
    ui->treeView->expandAll();
    ui->treeView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
}

MainWindow::~MainWindow()
{
    delete ui;
}

