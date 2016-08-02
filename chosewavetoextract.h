#ifndef CHOSEWAVETOEXTRACT_H
#define CHOSEWAVETOEXTRACT_H

#include <QDialog>

#include <__common.h>
#include <QTableWidget>

namespace Ui {
class choseWaveToExtract;
}

class choseWaveToExtract : public QDialog
{
    Q_OBJECT

    lstDoubleAxisCalibration daCalib;

public:
    explicit choseWaveToExtract(QWidget *parent = 0);
    ~choseWaveToExtract();    

private slots:
    void on_pbAdd_clicked();

    void switchSelected( QTableWidget *tableOrig, QTableWidget *tableDest);

    void fromTablesToFiles();

    void on_pbRemoveAll_clicked();

    void on_pbAddAll_clicked();

    void on_pbRemove_clicked();

private:
    Ui::choseWaveToExtract *ui;

    void fillOptions();

    void insertRow(QString wave , QTableWidget *table);

    void refreshOptChoi();

    void iniOptsAndChois(bool allOptions);

};

#endif // CHOSEWAVETOEXTRACT_H