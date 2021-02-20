#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFileDialog>
#include <QRegularExpression>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    table = ui->tableWidget;
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_actionOpen_triggered()
{
    auto filename = QFileDialog::getOpenFileName(this, "Open File", QDir::rootPath(), "TSV File (*.tsv)");

    if(filename.isEmpty()) {
        return;
    }

    // Clear the table
    table->clear();
    table->setRowCount(0);
    table->setColumnCount(0);

    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
         return;
    }

    QTextStream xin(&file);
    int i = 0;

    while(!xin.atEnd()) {
        auto line = xin.readLine();
        table->setRowCount(i + 1);

        auto value = line.split('\t');

        const int colCount = value.size();
        table->setColumnCount(colCount);

        for(int j=0; j < colCount; ++j) {
            setValueAt(i, j, value.at(j));
        }

        ++i;
    }

    file.close();
    Flag_IsOpen = 1;
    Flag_IsNew = 0;
    Last_FileName = filename;
}

void MainWindow::on_actionNew_triggered()
{
    table->clear();
    const int rowCount = 2;
    const int colCount = 3;
    table->setRowCount(rowCount);
    table->setColumnCount(colCount);

    Flag_IsNew = 1;
    Flag_IsOpen = 1;
}

void MainWindow::on_actionSave_triggered()
{
    if(Flag_IsNew) {
        auto filename = QFileDialog::getSaveFileName(this, "Save", QDir::rootPath(), "TSV File (*.tsv)");

        if(filename.isEmpty()) {
            return;
        }

        QFile file(filename);

        if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            return;
        }

        QTextStream xout(&file);
        const int rowCount = table->rowCount();
        const int colCount = table->columnCount();

        for(int i=0; i<rowCount; ++i) {
            xout << getValueAt(i, 0);
            for(int j=1; j<colCount; ++j) {
                xout << "\t" << getValueAt(i, j);
            }
            xout << "\n";
        }
        file.flush();
        file.close();

        Flag_IsNew = 0;
        Last_FileName = filename;

    } else {
        QFile file(Last_FileName);
        if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QMessageBox::warning(this,tr("Warning"),tr("Fail to open the file"));
            return;
        }
        else
        {
            if(Last_FileName.isEmpty()) {
                return;
            }

            QFile file(Last_FileName);

            if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                return;
            }

            QTextStream xout(&file);
            const int rowCount = table->rowCount();
            const int colCount = table->columnCount();

            for(int i=0; i<rowCount; ++i) {
                xout << getValueAt(i, 0);
                for(int j=1; j<colCount; ++j) {
                    xout << "\t" << getValueAt(i, j);
                }
                xout << "\n";
            }
            file.flush();
            file.close();
        }
    }
}

void MainWindow::on_actionConvert_triggered()
{
    auto filename = QFileDialog::getOpenFileName(this, "Open File", QDir::rootPath(), "LNX File (*.lnx)");

    if(filename.isEmpty()) {
        return;
    }

    // Clear the table
    table->clear();
    table->setRowCount(0);
    table->setColumnCount(0);

    // Get the file and open it
    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
         return;
    }

    QTextStream xin(&file);
    int i = 0;

    while(!xin.atEnd()) {
        auto line = xin.readLine();
        table->setRowCount(i + 1);
        table->setColumnCount(3);

        if(line.contains(';')) {

            // It's a dialogue
            setValueAt(i, 0, "Dialogue");

            auto value = line.split(';');

            const int colCount = value.size();

            for(int j=0; j < colCount; ++j) {
                if(value.at(j).contains("대화 이름=\"")){
//                    setValueAt(i, 1, value.at(j));

                    QRegularExpression re("(?<=\").*?(?=\")");
                    QRegularExpressionMatch match = re.match(value.at(j));

                    if(match.hasMatch()) {
                        QString matched = match.captured();
                        setValueAt(i, 1, matched);
                    }
                } else {
                    setValueAt(i, 2, value.at(j));
                }
            }
        } else {
            // It's not a dialogue
            setValueAt(i, 0, line);
        }

        ++i;
    }

    file.close();
    Flag_IsNew = 1;
    Flag_IsOpen = 1;
}

void MainWindow::setValueAt(int i, int j, const QString &value)
{
    if(!table->item(i, j)) {
        table->setItem(i, j, new QTableWidgetItem(value));
    } else {
        table->item(i, j)->setText(value);
    }
}

QString MainWindow::getValueAt(int i, int j)
{
    if(!table->item(i, j)) {
        return "";
    }
    return table->item(i, j)->text();
}
