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

    TagLib::MPEG::File file(m_audioFile.toLocal8Bit().data());
    TagLib::ID3v2::Tag * id3v2Tag = file.ID3v2Tag();
    if (id3v2Tag) {
        const TagLib::ID3v2::FrameList & framelist = id3v2Tag->frameList();
        for (const TagLib::ID3v2::Frame * frame : framelist) {
            if (frame->frameID() == "CHAP") {
                const TagLib::ID3v2::ChapterFrame * chapterFrame = dynamic_cast<const TagLib::ID3v2::ChapterFrame *>(frame);
                //std::cout << "[" << frame->frameID() << ":" << frame->toString() << "]" << std::endl;
                std::cout << chapterFrame->elementID() << " " << chapterFrame->startTime() << " " << chapterFrame->endTime() << std::endl;
                const TagLib::ID3v2::FrameList subFrames = chapterFrame->embeddedFrameList();

                for (const TagLib::ID3v2::Frame * subFrame : subFrames) {
                    if (subFrame->frameID() == "TIT2") {
                        const TagLib::ID3v2::TextIdentificationFrame * chapterTitle = dynamic_cast<const TagLib::ID3v2::TextIdentificationFrame *>(subFrame);
                        std::cout << "title: " << chapterTitle->toString() << std::endl;
                    } else if (subFrame->frameID() == "WXXX") {
                        const TagLib::ID3v2::UserUrlLinkFrame * wwwLink = dynamic_cast<const TagLib::ID3v2::UserUrlLinkFrame *>(subFrame);
                        std::cout << wwwLink->description() << " " << wwwLink->url() << std::endl;
                    } else if (subFrame->frameID() == "APIC") {
                        const TagLib::ID3v2::AttachedPictureFrame * pic = dynamic_cast<const TagLib::ID3v2::AttachedPictureFrame *>(subFrame);
                        std::cout << pic->description() << " " << pic->mimeType() << std::endl;
                    }
                }
                // TIT2: TextIdentificationFrame
                // APIC: AttachedPictureFrame
                // WXXX: UserUrlLinkFrame
            } else if (frame->frameID() == "CTOC") {
                const TagLib::ID3v2::TableOfContentsFrame * tocFrame = dynamic_cast<const TagLib::ID3v2::TableOfContentsFrame *>(frame);
                std::cout << "[" << frame->frameID() << ":" << frame->toString() << "]" << std::endl;
                std::cout << tocFrame->elementID() << " " << tocFrame->isTopLevel() << std::endl;
                for (const TagLib::ByteVector & bv : tocFrame->childElements()) {
                    std::cout << bv << std::endl;
                }
            }
        }
    }
//    TagLib::FileRef fileRef(m_audioFile.toLocal8Bit().data());
//    if (!fileRef.isNull() && fileRef.tag()) {
//        fileRef.tag();
//    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

