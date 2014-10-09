#ifndef CSVMODEL_H
#define CSVMODEL_H

#include <QAbstractTableModel>

class QTextStream;
class CSVModelPrivate;

class CSVModel : public QAbstractTableModel {
    Q_OBJECT
    Q_DECLARE_PRIVATE(CSVModel)

    CSVModelPrivate * const d_ptr;
public:
    explicit CSVModel(QObject *parent = 0);
    ~CSVModel();

    QChar separator() const;
    bool hasHeader() const;

    bool stripQuotes() const;
    void setStripQuotes(bool on);

    void parse(QTextStream *source_stream, const QChar &separator, bool has_header);
    void clear();

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
};

#endif // CSVMODEL_H
