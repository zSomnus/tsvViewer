#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QTableWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionOpen_triggered();

    void on_actionNew_triggered();

    void on_actionSave_triggered();

    void on_actionConvert_triggered();

    void on_actionExport_all_triggered();

    void on_actionExport_tsv_triggered();

    void on_actionExport_Cs_triggered();

private:

    void setValueAt(int i, int j, const QString &value);
    QString getValueAt(int i, int j);

    Ui::MainWindow *ui;

    int Flag_IsOpen = 0;       // Flag: Is the file opend or not
    int Flag_IsNew = 0;        // Flag: if it's new file, set the value to 1, initial number is 0
    QString Last_FileName;     // File name during last save
    QString Last_FileContent;  // File content during last save

    QTableWidget *table;
};
#endif // MAINWINDOW_H
