#include <QApplication>
#include <QTableView>
#include <QFile>
#include <QTextStream>
#include <QHeaderView>
#include <QDebug>
#include "csvmodel.h"


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QFile test_data_file("test_data.csv");
    if (!test_data_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "can't open test data file";
        return -1;
    }

    QTextStream test_data_stream(&test_data_file);

    QTableView *csv_view = new QTableView();
    csv_view->setAttribute(Qt::WA_DeleteOnClose);
    csv_view->setWindowTitle("CSV Model Example");

    CSVModel *csv_model = new CSVModel(csv_view);
    csv_model->parse(&test_data_stream, QChar(','), true);

    test_data_file.close();

    csv_view->horizontalHeader()->setVisible(csv_model->hasHeader());
    csv_view->verticalHeader()->setVisible(false);

    csv_view->setModel(csv_model);

    csv_view->show();

    return app.exec();
}
