#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    void on_actionOpen_triggered();
    void on_actionSave_triggered();

    void on_treeView_viewSelectionChanged();

    void on_appendChapterBtn_clicked();
    void on_removeBtn_clicked();

    void on_importBtn_clicked();

private:
    void loadFile();
    void importFromLines(ChapterTreeModel *model, const QStringList &lines);

private:
    Ui::MainWindow *ui;

    QString m_filePath;
};
#endif // MAINWINDOW_H
