#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFileDialog>
#include <QRegularExpression>
#include <QMessageBox>
#include <QInputDialog>

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
"public class ";

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionConvert_triggered()
{
    auto filename = QFileDialog::getOpenFileName(this, "Open File", QDir::rootPath(), "LNX File (*.lnx)");

    if(filename.isEmpty()) {
        return;
    } else {
        // Clear the table
        table->clear();
        table->setRowCount(0);
        table->setColumnCount(0);

        // Clear the text edit
        textEdit->clear();
        textEdit->setPlainText("");

        QRegularExpression re("[0-9]+");
        QRegularExpressionMatch match = re.match(filename);

        if(match.hasMatch()) {
            dialogueKeyPrefix = "dialogue_";
            dialogueKeyPrefix += match.captured() + "_";
            setFileName(match.captured());
            textEdit->insertPlainText(codePrefix + saveFilename + " : DialogueScript\n{\n"
"\tinternal override BaseDialogueEntry[] ConstructDialogue() => new BaseDialogueEntry[]\n"
"\t{\n");
        }
    }

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

void MainWindow::exportCsFile(QString filename)
{
    if(filename.isEmpty()) {
        return;
    }

    QFile file(filename);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::information(this, "Error Message", "Invalid file name!");
        return;
    }

    QTextStream xout(&file);
    xout << textEdit->toPlainText();

    file.flush();
    file.close();

    Flag_IsNew = 0;
    Last_FileName = filename;
}

void MainWindow::exportTsvFile(QString filename)
{
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
}

void MainWindow::setFileName(QString indexString)
{
    bool ok;
    QString text = QInputDialog::getText(this, "Set file name", "File name", QLineEdit::Normal, "DS_Joanna_" + indexString, &ok);
    if (ok && !text.isEmpty())
    {
        saveFilename = text;
    }
}

void MainWindow::SetDialogue(QString name, QString dialogue)
{
    QString temp = "";

    if(name.isEmpty()) {
        temp = "\t\tnew TextDialogueEntry(\"" + dialogue + "\"),\n";
    } else {
        temp = "\t\tnew TextDialogueEntry(\"" + name + "\", \"" + dialogue + "\"),\n";
    }

    textEdit->insertPlainText(temp);
}

void MainWindow::on_actionExport_all_triggered()
{
    auto filename = QFileDialog::getExistingDirectory(this, "Export", QDir::rootPath());

    exportTsvFile(filename + "/" + saveFilename + ".tsv");
    exportCsFile(filename + "/" + saveFilename + ".cs");
}

void MainWindow::on_actionExport_tsv_triggered()
{
    exportTsvFile(QFileDialog::getExistingDirectory(this, "Export", QDir::rootPath()) + "/" + saveFilename + ".tsv");
}

void MainWindow::on_actionExport_Cs_triggered()
{
    exportCsFile(QFileDialog::getExistingDirectory(this, "Export", QDir::rootPath()) + "/" + saveFilename + ".cs");
}
