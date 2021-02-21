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
    textEdit = ui->plainTextEdit;

    codePrefix = "/*\n"
" * Copyright (c) JellySnow Studio. All rights reserved.\n"
" *\n"
" */\n"
"#pragma warning disable CS0649\n"
"using JellyScript;\n"
"using UnityEngine;\n"
"public class DS_Joanna_";

    dialogueKeyPrefix = "dialogue_";

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
    } else {
        QRegularExpression re("[0-9]+");
        QRegularExpressionMatch match = re.match(filename);

        if(match.hasMatch()) {
            dialogueKeyPrefix += match.captured() + "_";
            codePrefix += match.captured() + " : DialogueScript\n{\n\tinternal override BaseDialogueEntry[] ConstructDialogue() => new BaseDialogueEntry[]\n\t{\n";
            textEdit->insertPlainText(codePrefix);
        }
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

    int row = 0;

    table->setColumnCount(3);

    while(!xin.atEnd()) {
        auto line = xin.readLine();

        if(line.contains(';')) {

            // It's a dialogue
            table->setRowCount(row + 1);
            setValueAt(row, 0, dialogueKeyPrefix + QString::number(row + 1));

            QString name = "";
            QString dialogue = "";
            auto value = line.split(';');

            const int colCount = value.size();

            for(int j=0; j < colCount; ++j) {
                if(value.at(j).contains("대화 이름=\"")){
                    QRegularExpression re("(?<=\").*?(?=\")");
                    QRegularExpressionMatch match = re.match(value.at(j));

                    if(match.hasMatch()) {
                        QString matched = match.captured();
                        name = matched;
                        setValueAt(row, 1, name);
                    }
                } else {
                    dialogue = value.at(j);
                    setValueAt(row, 2, dialogue);
                }
            }

            SetDialogue(name, dialogue);
            ++row;

        } else {
            // It's not a dialogue
            if(line.isEmpty()) {
                textEdit->insertPlainText("\n");
            } else {
                textEdit->insertPlainText("\t\t// " + line + "\n");
            }
        }
    }

    textEdit->insertPlainText("\t};\n}");

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

void MainWindow::SetDialogue(QString name, QString dialogue)
{
    QString temp = "";

    if(name.isEmpty()) {
        temp = "\t\tnew TextDialogueEntry(\"" + dialogue + "\",\n";
    } else {
        temp = "\t\tnew TextDialogueEntry(\"" + name + "\", \"" + dialogue + "\",\n";
    }

    textEdit->insertPlainText(temp);
}

void MainWindow::on_actionExport_all_triggered()
{

}

void MainWindow::on_actionExport_tsv_triggered()
{

}

void MainWindow::on_actionExport_Cs_triggered()
{

}
